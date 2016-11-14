
#include <fcntl.h>
#include <sys/types.h>

#include <assert.h>
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "log.h"

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include "ringbuffer.h"
#include "OpenSlApi.h"
#include "AudioPlayer.h"
#include "FFmpegApi.h"

static SL SLObj;
static pSL pSLObj = NULL;

static CallbackCntxt RecordCntxt;
static int8_t RecordData[NB_BUFFERS_IN_QUEUE * BUFFER_SIZE_IN_BYTES];  


static int SL_CreateEngine(void)
{
    SLresult result;

    // create engine
    result = slCreateEngine(&pSLObj->engineObject, 0, NULL, 0, NULL, NULL);

    // realize the engine
    result = (*pSLObj->engineObject)->Realize(pSLObj->engineObject, SL_BOOLEAN_FALSE);

    // get the engine interface, which is needed in order to create other objects
    result = (*pSLObj->engineObject)->GetInterface(pSLObj->engineObject, SL_IID_ENGINE, &pSLObj->engineEngine);

    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    //result = (*pSLObj->engineEngine)->CreateOutputMix(pSLObj->engineEngine, &pSLObj->outputMixObject, 1, ids, req);
    result = (*pSLObj->engineEngine)->CreateOutputMix(pSLObj->engineEngine, &pSLObj->outputMixObject, 0, 0, 0);
    // realize the output mix
    result = (*pSLObj->outputMixObject)->Realize(pSLObj->outputMixObject, SL_BOOLEAN_FALSE);

    // get the environmental reverb interface
    // this could fail if the environmental reverb effect is not available,
    // either because the feature is not present, excessive CPU load, or
    // the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
    result = (*pSLObj->outputMixObject)->GetInterface(pSLObj->outputMixObject, SL_IID_ENVIRONMENTALREVERB,
              &pSLObj->outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        //aux effect on the output mix, used by the buffer queue player
        static const SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
        result = (*pSLObj->outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(pSLObj->outputMixEnvironmentalReverb, &reverbSettings);
    }

    return 0;
}

// this callback handler is called every time a buffer finishes playing
static void SL_PlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    void *buffer = NULL;
    int bufferSize;

    bufferSize = SL_GetPcmData(&buffer);

    // for streaming playback, replace this test by logic to find and fill the next buffer
    if (bufferSize > 0) 
    {
        SLresult result;
        // enqueue another buffer
        result = (*pSLObj->bqPlayerBufferQueue)->Enqueue(pSLObj->bqPlayerBufferQueue, buffer, bufferSize);
        // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
        // which for this code example would indicate a programming error
        //assert(SL_RESULT_SUCCESS == result);
        (void)result;
    }
}


// create buffer queue audio player
static int SL_CreateBufferQueueAudioPlayer(pPCM_PARAM pPCM)
{
    SLresult result;

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};

    SLDataFormat_PCM format_pcm;
    format_pcm.formatType = SL_DATAFORMAT_PCM;
    format_pcm.numChannels = pPCM->channels;
    format_pcm.samplesPerSec = pPCM->rate * 1000;
    format_pcm.bitsPerSample = pPCM->bitsPerSample;
    if(format_pcm.bitsPerSample == SL_PCMSAMPLEFORMAT_FIXED_8){
        format_pcm.containerSize = SL_PCMSAMPLEFORMAT_FIXED_8;
    }
    else{
        format_pcm.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
    }
    
    if (format_pcm.numChannels == 2){
        format_pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    }
    else{
        format_pcm.channelMask = SL_SPEAKER_FRONT_CENTER;
    }
    format_pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;
    
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, pSLObj->outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // create audio player
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND,/*SL_IID_MUTESOLO,*/ SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE,/*SL_BOOLEAN_TRUE,*/ SL_BOOLEAN_TRUE};
    result = (*pSLObj->engineEngine)->CreateAudioPlayer(pSLObj->engineEngine, &pSLObj->bqPlayerObject, &audioSrc, &audioSnk,3, ids, req);
    //assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // realize the player
    result = (*pSLObj->bqPlayerObject)->Realize(pSLObj->bqPlayerObject, SL_BOOLEAN_FALSE);
    //assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // get the play interface
    result = (*pSLObj->bqPlayerObject)->GetInterface(pSLObj->bqPlayerObject, SL_IID_PLAY, &pSLObj->bqPlayerPlay);
    //assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // get the buffer queue interface
    result = (*pSLObj->bqPlayerObject)->GetInterface(pSLObj->bqPlayerObject, SL_IID_BUFFERQUEUE,&pSLObj->bqPlayerBufferQueue);
    //assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // register callback on the buffer queue
    result = (*pSLObj->bqPlayerBufferQueue)->RegisterCallback(pSLObj->bqPlayerBufferQueue, SL_PlayerCallback, NULL);
    //assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // get the effect send interface
    result = (*pSLObj->bqPlayerObject)->GetInterface(pSLObj->bqPlayerObject, SL_IID_EFFECTSEND, &pSLObj->bqPlayerEffectSend);
    //assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // get the volume interface
    result = (*pSLObj->bqPlayerObject)->GetInterface(pSLObj->bqPlayerObject, SL_IID_VOLUME, &pSLObj->bqPlayerVolume);
    //assert(SL_RESULT_SUCCESS == result);
    (void)result;

    return 0;
}


int SL_Play(pPCM_PARAM pPCM)
{
    if(!pSLObj){
        return -1;
    }

    if(!pPCM->rate || !pPCM->channels || !pPCM->bitsPerSample){
        LOGW("param err");
        return -1;
    }

    //播放前如果当前正在播放，需先停止
    SL_PlayStop();

    _lock(&pSLObj->Mutex);
    if(!pSLObj->engineObject){
        if(0 != SL_CreateEngine()){
            _unlock(&pSLObj->Mutex);
            return -1;
        }
    }
    if(0 != SL_CreateBufferQueueAudioPlayer(pPCM)){
        _unlock(&pSLObj->Mutex);
        return -1;
    }
        
    _unlock(&pSLObj->Mutex);

    (*pSLObj->bqPlayerPlay)->SetPlayState(pSLObj->bqPlayerPlay, SL_PLAYSTATE_PLAYING);

    SL_PlayerCallback(NULL, NULL);
    
    LOGD("SL_Play");

    return 0;
}

int SL_PlayStop(void)
{    
    if(!pSLObj){
        return -1;
    }
    _lock(&pSLObj->Mutex);
    // destroy buffer queue audio player object, and invalidate all associated interfaces
    if (pSLObj->bqPlayerObject != NULL) {
        (*pSLObj->bqPlayerPlay)->SetPlayState(pSLObj->bqPlayerPlay, SL_PLAYSTATE_STOPPED);
        (*pSLObj->bqPlayerObject)->Destroy(pSLObj->bqPlayerObject);
        pSLObj->bqPlayerObject = NULL;
        pSLObj->bqPlayerPlay = NULL;
        pSLObj->bqPlayerBufferQueue = NULL;
        pSLObj->bqPlayerEffectSend = NULL;
        pSLObj->bqPlayerMuteSolo = NULL;
        pSLObj->bqPlayerVolume = NULL;
        LOGD("SL_StopPlay");
    }
    _unlock(&pSLObj->Mutex);
}


int SL_SetVolume(int volume)
{
    int result;

    if(!pSLObj || !pSLObj->bqPlayerVolume){
        return -1;
    }

    LOGV("SL_SetVolume volume = %d", volume);

    result = (*pSLObj->bqPlayerVolume)->SetVolumeLevel(pSLObj->bqPlayerVolume, volume);

    return result;
}

int SL_SetMute(int state)
{
    int result;

    if(!pSLObj || !pSLObj->bqPlayerVolume){
        return -1;
    }

    LOGV("SL_SetMute state = %d", state);

    result = (*pSLObj->bqPlayerVolume)->SetMute(pSLObj->bqPlayerVolume, state);

    return result;
}




static int SL_CreateRecorderEngine(void)
{
    SLresult result;

    // create engine
    result = slCreateEngine(&pSLObj->recorderEngineObject, 0, NULL, 0, NULL, NULL);

    // realize the engine
    result = (*pSLObj->recorderEngineObject)->Realize(pSLObj->recorderEngineObject, SL_BOOLEAN_FALSE);

    // get the engine interface, which is needed in order to create other objects
    result = (*pSLObj->recorderEngineObject)->GetInterface(pSLObj->recorderEngineObject, SL_IID_ENGINE, &pSLObj->recorderEngineEngine);

    return 0;
}

//-----------------------------------------------------------------  
/* Structure for passing information to callback function */  

/* Callback for recording buffer queue events */  
static void SL_RecorderCallback(SLRecordItf caller, void *pContext, SLuint32 event) {  
    if (SL_RECORDEVENT_HEADATNEWPOS & event) {  
        SLmillisecond pMsec = 0;  
        (*caller)->GetPosition(caller, &pMsec);  
        //LOGD("SL_RECORDEVENT_HEADATNEWPOS current position=%ums\n", pMsec);  
    }  
  
    if (SL_RECORDEVENT_HEADATMARKER & event) {  
        SLmillisecond pMsec = 0;  
        (*caller)->GetPosition(caller, &pMsec);  
        //LOGD("SL_RECORDEVENT_HEADATMARKER current position=%ums\n", pMsec);  
    }  
}  
  
/* Callback for recording buffer queue events */  
static void SL_RecorderBufferQueueCallback(SLAndroidSimpleBufferQueueItf queueItf, void *pContext)
{  
    CallbackCntxt *pCntxt = (CallbackCntxt*) pContext;  
    
    SL_WriteRecordData(pCntxt->pDataBase, BUFFER_SIZE_IN_BYTES);

    /* Increase data pointer by buffer size */  
    pCntxt->pData += BUFFER_SIZE_IN_BYTES;  
  
    if (pCntxt->pData >= pCntxt->pDataBase + (NB_BUFFERS_IN_QUEUE * BUFFER_SIZE_IN_BYTES)) {  
        pCntxt->pData = pCntxt->pDataBase;  
    }  
  
    (*queueItf)->Enqueue(queueItf, pCntxt->pDataBase, BUFFER_SIZE_IN_BYTES);
    //(*queueItf)->Enqueue(queueItf, pCntxt->pData, BUFFER_SIZE_IN_BYTES); 
  
    SLAndroidSimpleBufferQueueState recQueueState;  
    (*queueItf)->GetState(queueItf, &recQueueState);  
  
}  


/* 
 * Class:     com_example_testopensl_AudioRecordActivity 
 * Method:    createAudioRecorder 
 * Signature: (Ljava/lang/String;)V 
 */  
static int SL_CreateAudioRecorder(pPCM_PARAM pPCM)
{    
    SLresult result;  
  
    /* setup the data source*/  
    SLDataLocator_IODevice ioDevice = {  
            SL_DATALOCATOR_IODEVICE,  
            SL_IODEVICE_AUDIOINPUT,  
            SL_DEFAULTDEVICEID_AUDIOINPUT,  
            NULL  
    };  
  
    SLDataSource recSource = {&ioDevice, NULL};  
  
    SLDataLocator_AndroidSimpleBufferQueue recBufferQueue = {  
            SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,  
            NB_BUFFERS_IN_QUEUE  
    };  
  
    SLDataFormat_PCM format_pcm = {  
            SL_DATAFORMAT_PCM,  
            2,  
            SL_SAMPLINGRATE_44_1,  
            SL_PCMSAMPLEFORMAT_FIXED_16,  
            16,  
            SL_SPEAKER_FRONT_LEFT| SL_SPEAKER_FRONT_RIGHT,  
            SL_BYTEORDER_LITTLEENDIAN  
    };
    
    format_pcm.formatType = SL_DATAFORMAT_PCM;
    format_pcm.numChannels = pPCM->channels;
    format_pcm.samplesPerSec = pPCM->rate * 1000;
    format_pcm.bitsPerSample = pPCM->bitsPerSample;
    if(format_pcm.bitsPerSample == SL_PCMSAMPLEFORMAT_FIXED_8){
        format_pcm.containerSize = SL_PCMSAMPLEFORMAT_FIXED_8;
    }
    else{
        format_pcm.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
    }
    
    if (format_pcm.numChannels == 2){
        format_pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    }
    else{
        format_pcm.channelMask = SL_SPEAKER_FRONT_CENTER;
    }
    format_pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;


    SLDataSink dataSink = { &recBufferQueue, &format_pcm };  
    SLInterfaceID iids[NUM_EXPLICIT_INTERFACES_FOR_RECORDER] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_ANDROIDCONFIGURATION};  
    SLboolean required[NUM_EXPLICIT_INTERFACES_FOR_RECORDER] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};  
  
    /* Create the audio recorder */  
    result = (*pSLObj->recorderEngineEngine)->CreateAudioRecorder(pSLObj->recorderEngineEngine, &pSLObj->recorderObject , &recSource, &dataSink,  
            NUM_EXPLICIT_INTERFACES_FOR_RECORDER, iids, required);  
    //assert(SL_RESULT_SUCCESS == result);  
  
  
    /* get the android configuration interface*/  
    result = (*pSLObj->recorderObject)->GetInterface(pSLObj->recorderObject, SL_IID_ANDROIDCONFIGURATION, &pSLObj->configItf);  
    //assert(SL_RESULT_SUCCESS == result);  
  
    /* Realize the recorder in synchronous mode. */  
    result = (*pSLObj->recorderObject)->Realize(pSLObj->recorderObject, SL_BOOLEAN_FALSE);  
    //assert(SL_RESULT_SUCCESS == result);  
  
    /* Get the buffer queue interface which was explicitly requested */  
    result = (*pSLObj->recorderObject)->GetInterface(pSLObj->recorderObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, (void*)&pSLObj->recBuffQueueItf);  
    //assert(SL_RESULT_SUCCESS == result);  
  
  
    /* get the record interface */  
    result = (*pSLObj->recorderObject)->GetInterface(pSLObj->recorderObject, SL_IID_RECORD, &pSLObj->recordItf);  
    //assert(SL_RESULT_SUCCESS == result);  
  
  
    /* Set up the recorder callback to get events during the recording */  
    result = (*pSLObj->recordItf)->SetMarkerPosition(pSLObj->recordItf, 2000);  
    //assert(SL_RESULT_SUCCESS == result);  
  
    result = (*pSLObj->recordItf)->SetPositionUpdatePeriod(pSLObj->recordItf, 500);  
    //assert(SL_RESULT_SUCCESS == result);  
  
    result = (*pSLObj->recordItf)->SetCallbackEventsMask(pSLObj->recordItf,  
                SL_RECORDEVENT_HEADATMARKER | SL_RECORDEVENT_HEADATNEWPOS);  
    //assert(SL_RESULT_SUCCESS == result);  
  
    result = (*pSLObj->recordItf)->RegisterCallback(pSLObj->recordItf, SL_RecorderCallback, NULL);  
    //assert(SL_RESULT_SUCCESS == result);  
  
    /* Initialize the callback and its context for the recording buffer queue */  
  
    RecordCntxt.pDataBase = (int8_t*)&RecordData;  
    RecordCntxt.pData = RecordCntxt.pDataBase;  
    RecordCntxt.size = sizeof(RecordData);  
    result = (*pSLObj->recBuffQueueItf)->RegisterCallback(pSLObj->recBuffQueueItf, SL_RecorderBufferQueueCallback, &RecordCntxt);  
    //assert(SL_RESULT_SUCCESS == result);  
  
    /* Enqueue buffers to map the region of memory allocated to store the recorded data */  
    int i;
    for ( i=0; i < NB_BUFFERS_IN_QUEUE; i++) {  
        result = (*pSLObj->recBuffQueueItf)->Enqueue(pSLObj->recBuffQueueItf, RecordCntxt.pData, BUFFER_SIZE_IN_BYTES);  
        //assert(SL_RESULT_SUCCESS == result);  
        RecordCntxt.pData += BUFFER_SIZE_IN_BYTES;  
    }  
    RecordCntxt.pData = RecordCntxt.pDataBase;

    return 0;
}  

int SL_Record(pPCM_PARAM pPCM)
{
    if(!pSLObj){
        return -1;
    }
    //播放前如果当前正在录音，需先停止
    SL_RecordStop();

    _lock(&pSLObj->Mutex);
    if(!pSLObj->recorderEngineObject){
        if(0 != SL_CreateRecorderEngine()){
            _unlock(&pSLObj->Mutex);
            return -1;
        }
    }
    if(0 != SL_CreateAudioRecorder(pPCM)){
        _unlock(&pSLObj->Mutex);
        return -1;
    }
    _unlock(&pSLObj->Mutex);

    (*pSLObj->recordItf)->SetRecordState(pSLObj->recordItf, SL_RECORDSTATE_RECORDING);  
    
    LOGD("SL_Record");

    return 0;

}

int SL_RecordStop(void)
{
    if(!pSLObj){
        return -1;
    }
    
    _lock(&pSLObj->Mutex);
    if (pSLObj->recorderObject != NULL) {
        (*pSLObj->recordItf)->SetRecordState(pSLObj->recordItf, SL_RECORDSTATE_STOPPED); 
        (*pSLObj->recorderObject)->Destroy(pSLObj->recorderObject);
        pSLObj->recorderObject = NULL;
        pSLObj->recordItf = NULL;  
        pSLObj->recBuffQueueItf = NULL;  
        pSLObj->configItf = NULL;  
        LOGD("SL_RecordStop");
    }
    _unlock(&pSLObj->Mutex);
    
    return 0;
}

int SL_SetRecordState(int state)
{
    if(!pSLObj){
        return -1;
    }
    
    _lock(&pSLObj->Mutex);
    if(pSLObj->recordItf) {
        (*pSLObj->recordItf)->SetRecordState(pSLObj->recordItf, state);  
    }
    _unlock(&pSLObj->Mutex);
    
    LOGD("SL_SetRecordState");
    return 0;    
}

int SL_PlayShutdown(void)
{
    if(!pSLObj){
        return -1;
    }
    
    _lock(&pSLObj->Mutex);
    // destroy buffer queue audio player object, and invalidate all associated interfaces
    if (pSLObj->bqPlayerObject != NULL) {
        (*pSLObj->bqPlayerObject)->Destroy(pSLObj->bqPlayerObject);
        pSLObj->bqPlayerObject = NULL;
        pSLObj->bqPlayerPlay = NULL;
        pSLObj->bqPlayerBufferQueue = NULL;
        pSLObj->bqPlayerEffectSend = NULL;
        pSLObj->bqPlayerMuteSolo = NULL;
        pSLObj->bqPlayerVolume = NULL;
    }

    // destroy output mix object, and invalidate all associated interfaces
    if (pSLObj->outputMixObject != NULL) {
        (*pSLObj->outputMixObject)->Destroy(pSLObj->outputMixObject);
        pSLObj->outputMixObject = NULL;
        pSLObj->outputMixEnvironmentalReverb = NULL;
    }
    
    // destroy engine object, and invalidate all associated interfaces
    if (pSLObj->engineObject != NULL) {
        (*pSLObj->engineObject)->Destroy(pSLObj->engineObject);
        pSLObj->engineObject = NULL;
        pSLObj->engineEngine = NULL;
        
        LOGD("SL_PlayShutdown");
    }
    _unlock(&pSLObj->Mutex);

    return 0;
}

int SL_RecordShutdown(void)
{
    if(!pSLObj){
        return -1;
    }
    
    _lock(&pSLObj->Mutex);
    //destroy recorder object , and invlidate all associated interfaces  
    if (pSLObj->recorderObject != NULL) {  
        (*pSLObj->recordItf)->SetRecordState(pSLObj->recordItf, SL_RECORDSTATE_STOPPED);
        (*pSLObj->recorderObject)->Destroy(pSLObj->recorderObject);  
        pSLObj->recorderObject = NULL;  
        pSLObj->recordItf = NULL;  
        pSLObj->recBuffQueueItf = NULL;  
        pSLObj->configItf = NULL;  
    }

    // destroy engine object, and invalidate all associated interfaces
    if (pSLObj->recorderEngineObject != NULL) {
        (*pSLObj->recorderEngineObject)->Destroy(pSLObj->recorderEngineObject);
        pSLObj->recorderEngineObject = NULL;
        pSLObj->recorderEngineEngine = NULL;

        LOGD("SL_RecordShutdown");
    }
    _unlock(&pSLObj->Mutex);

    return 0;
}

int SL_Shutdown(void)
{
    if(!pSLObj){
        return -1;
    }
    
    SL_PlayShutdown();
    SL_RecordShutdown();

    LOGD("SL_Shutdown");

    return 0;
}


int InitOpenSL(void)
{
    int nRet;
    
    pSLObj = &SLObj;

    bzero(pSLObj, sizeof(SL));
	nRet = _lock_init(&pSLObj->Mutex, NULL);
	if(0 != nRet)
	{
		return -1;
	}

    return 0;
}

int DelInitOpenSL(void)
{
    if(pSLObj){
        SL_Shutdown();
        _lock_destroy(&pSLObj->Mutex);
        pSLObj = NULL;
    }
    return 0;
}



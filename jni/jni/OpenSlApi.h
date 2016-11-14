

#ifndef OPENSL_API_H
#define OPENSL_API_H

#include <pthread.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include "wavfile.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0
#define SL_PCMSAMPLEFORMAT_FIXED_8     ((SLuint16) 0x0008)
#define SL_PCMSAMPLEFORMAT_FIXED_16    ((SLuint16) 0x0010)
#define SL_PCMSAMPLEFORMAT_FIXED_20    ((SLuint16) 0x0014)
#define SL_PCMSAMPLEFORMAT_FIXED_24    ((SLuint16) 0x0018)
#define SL_PCMSAMPLEFORMAT_FIXED_28    ((SLuint16) 0x001C)
#define SL_PCMSAMPLEFORMAT_FIXED_32    ((SLuint16) 0x0020)
#define SL_PCMSAMPLEFORMAT_FIXED_64    ((SLuint16) 0x0040)
#endif

/* Explicitly requesting SL_IID_ANDROIDSIMPLEBUFFERQUEUE and SL_IID_ANDROIDCONFIGURATION 
 * on the AudioRecorder object */  
#define NUM_EXPLICIT_INTERFACES_FOR_RECORDER    2  
  
/* Size of the recording buffer queue */  
#define NB_BUFFERS_IN_QUEUE                     3  
/* Size of each buffer in the queue */  
#define BUFFER_SIZE_IN_SAMPLES                  1024  
#define BUFFER_SIZE_IN_BYTES                    (2 * BUFFER_SIZE_IN_SAMPLES) 


typedef struct
{
    // engine interfaces
    SLObjectItf engineObject;
    SLEngineItf engineEngine;
    
    // output mix interfaces
    SLObjectItf outputMixObject;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb;
    
    // buffer queue player interfaces
    SLObjectItf bqPlayerObject;
    SLPlayItf bqPlayerPlay;
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
    SLEffectSendItf bqPlayerEffectSend;
    SLMuteSoloItf bqPlayerMuteSolo;
    SLVolumeItf bqPlayerVolume;



    // engine interfaces
    SLObjectItf recorderEngineObject;
    SLEngineItf recorderEngineEngine;

    SLObjectItf recorderObject;  
    SLRecordItf recordItf;  
    SLAndroidSimpleBufferQueueItf recBuffQueueItf;  
    SLAndroidConfigurationItf configItf; 

    _lock_t Mutex; //Ïß³Ì»¥³âËø
}SL, *pSL;

typedef struct CallbackCntxt_ {  
    SLuint32  size;  
    SLint8*   pDataBase;    // Base address of local audio data storage  
    SLint8*   pData;        // Current address of local audio data storage  
}CallbackCntxt, *pCallbackCntxt;  



int SL_Play(pPCM_PARAM pPCM);
int SL_PlayStop(void);
int SL_Record(pPCM_PARAM pPCM);
int SL_RecordStop(void);
int SL_SetRecordState(int state);
int SL_SetVolume(int volume);
int SL_SetMute(int state);
int SL_PlayShutdown(void);
int SL_RecordShutdown(void);
int SL_Shutdown(void);


int InitOpenSL(void);
int DelInitOpenSL(void);

#ifdef __cplusplus
}
#endif

#endif //AUDIOPLAYER_AUDIODEVICE_H

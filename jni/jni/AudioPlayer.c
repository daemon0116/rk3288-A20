

#include <fcntl.h>
#include <pthread.h>

#include "log.h"

#include "FFmpegApi.h"
#include "OpenSlApi.h"
#include "AudioPlayer.h"
#include "g711.h"
#include "ringbuffer.h"

#include "UdpApp.h"
#include "NetWork.h"
#include "main.h"


LOCAL_PLAY LocalPlay;
pRingBuffer gs_PlayAudioData = NULL;


PLAY_TASK_INFO PlayTask[PLAY_TASK_MAX];
_lock_t PlayTaskMutex;


//1byte��λΪ1�ĸ���
const char Bits1InByte[] = 
{
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,///0
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,///1
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,///2
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,///3
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,///4
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,///5
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,///6
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,///7
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,///8
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,///9
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,///10
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,///11
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,///12
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,///13
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,///14
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8 ///15
}
;


int GetStreamData(void *opaque, uint8_t *buf, int buf_size)
{
    int len = 0;

    while(LocalPlay.format && (len = GetAudioData(gs_PlayAudioData, buf, buf_size))<= 0)
    {
        WaitAudioData(gs_PlayAudioData);
    }
    return len;
}

int GetPcmData(void **pcm)
{
    int len = 0;
    uint8 static audio_buf[1024];

    if(LocalPlay.format == FORMAT_MP3){
        len = FFmpegAudioDecode(LocalPlay.pffm, pcm);
    }
    else if(LocalPlay.format == FORMAT_WAV){  
        len = GetStreamData(NULL, audio_buf, sizeof(audio_buf));
        *pcm = audio_buf;
    }
    else if(LocalPlay.format == FORMAT_G711U){
        uint8 buf[sizeof(audio_buf)/2];
        len = GetStreamData(NULL, buf, sizeof(buf));
        if(len > 0){
            len = g711u_de(buf, audio_buf, len);
        }
        *pcm = audio_buf;
    }
    
    return len;
}

int SL_GetPcmData(void **pcm)
{
    int len = 0;
    static int DiscardCnt = 0;

    if(RingBuffAvaiData(gs_PlayAudioData) > 1280){
        LOGV("Discard data");
        GetPcmData(pcm);      //��Ƶ����������ƫ�࣬����һЩ����
        if(++DiscardCnt > 20){//��Ƶ��������������ƫ�࣬˵������SL�ص�ȡ���ݳ����쳣
            DiscardCnt = 0;
            LOGE("SL get data too slow!!!");
            PlayLocalHaveNew(CH_RESET_PLAYER);
            return 0;
        }
    }
    else{
        DiscardCnt = 0;
    }

    len = GetPcmData(pcm);
    
    return len;
}

void SL_WriteRecordData(char *data, int dataLen)
{
    int i;
    int len;
    int AvaDataSize;
    pPLAY_TASK_INFO pTask;
    
    #if(USE_G711)
    char audio_buf[BUFFER_SIZE_IN_BYTES/2];

    if(dataLen > BUFFER_SIZE_IN_BYTES){
        dataLen = BUFFER_SIZE_IN_BYTES;
    }
    dataLen = g711u_en(data, audio_buf, dataLen);
    data = audio_buf;
    #endif

    for(i=0; i<PLAY_TASK_MAX && pSysInfo->pRecordTask[i]; i++){
        pTask = pSysInfo->pRecordTask[i];
        if(pTask->isPlay/* && pTask->pRecordBuf*/){
            len = WriteAudioData(pTask->pRecordBuf, data, dataLen, &AvaDataSize);
            if(AvaDataSize<=dataLen || len!=dataLen){
                PostAudioData(pTask->pRecordBuf);
                if(len != dataLen){
                    LOGD("WriteRecordData size=%d, write=%d", dataLen, len);
                }
            }
        }
    }
}
/*******************************************************************************
���⣺DelayUs
���ܣ���startΪ�����ʱdelay΢��
���룺start: ��ʼʱ��
      delay: ��ʱ΢����
�����start: ��¼��ǰʱ��
����ֵ:
*******************************************************************************/

void DelayUs1(struct timeval *start, int delay)
{ 
    int time;
    struct timeval tm;
    
    gettimeofday(&tm, 0);
    time = delay - ((tm.tv_sec-start->tv_sec)*1000000+(tm.tv_usec-start->tv_usec));
    if(time > 0){
        tm.tv_sec=time/1000000;
        tm.tv_usec=time%1000000;
        select(0,0,0,0,&tm);
    }
    gettimeofday(start, 0);
}
void DelayUs(struct timeval *start, int delay)
{ 
    int time;
    struct timeval cur_tm;
    struct timeval delay_tm;
    
    gettimeofday(&cur_tm, 0);
    if(cur_tm.tv_sec == start->tv_sec){
        time = delay - (cur_tm.tv_usec-start->tv_usec);
    }
    else{
        time = delay - ((cur_tm.tv_sec-start->tv_sec)*1000000+(cur_tm.tv_usec-start->tv_usec));
    }
    if(time > 0){
        if(time <= 1000000){
            delay_tm.tv_sec = 0;
            delay_tm.tv_usec = time;
        }
        else{
            delay_tm.tv_sec = time/1000000;
            delay_tm.tv_usec = time%1000000;
        }
        
        start->tv_usec = cur_tm.tv_usec + time;
        if(start->tv_usec <= 1000000){
            start->tv_sec = cur_tm.tv_sec;
        }
        else{
            start->tv_sec = cur_tm.tv_sec + start->tv_usec/1000000;
            start->tv_usec = start->tv_usec%1000000;
        }

        select(0,0,0,0,&delay_tm);
    }
    else{
        start->tv_sec = cur_tm.tv_sec;
        start->tv_usec = cur_tm.tv_usec;
    }
}

/*******************************************************************************
���⣺max_valid_bit
���ܣ����������λΪ1�����λ��
���룺
�����
����ֵ:
*******************************************************************************/
int max_valid_bit(const char *buf, int size)
{
    int i;

    if(!buf){
        return 0;
    }

    for(i=size*8; i>0; --i){
        if(buf[i/8]&(1<<(i%8))){
            break;
        }
    }

    if(i || buf[0]){
        return i+1;
    }

    return 0;
}

/*******************************************************************************
���⣺max_valid_bytes
���ܣ��ҳ�����������治Ϊ0����Ŀ
���룺
�����
����ֵ:
*******************************************************************************/
int max_valid_bytes(const char *buf, int size)
{
    int i;
    int ret = 0;

    if(!buf){
        return 0;
    }

    for(i=0; i<size; i++){
        if(buf[i]){
            ret = i+1;
        }
    }

    return ret;
}

/*******************************************************************************
���⣺bits8_bits7
���ܣ����������ֽ�8��Чλת���ֽ�7��Чλ
���룺pTask: ����ָ��
�����
����ֵ:
*******************************************************************************/
int bits8_bits7(const char *in, int inSize, char *out, int outSize)
{
    int i;
    int bits;

    if(!in || !out){
        return -1;
    }
    
    bits = inSize * 8;

    memset(out, 0x80, outSize);

    for(i=0; i<bits; i++){
        if(in[i/8]&(1<<(i%8))){
            out[i/7] |= 1<<(i%7);
        }
    }

    if(0)
    {
    //test
    int i;
    char buff[1024];
    for(i=0; i<inSize&&i<sizeof(buff)/3; i++){
        sprintf(buff+i*3, "%02X ", in[i]);
    }
    LOGV("play dev in = %s", buff);
    }

    if(0)
    {
    //test
    int i;
    char buff[1024];
    for(i=0; i<outSize&&i<sizeof(buff)/3; i++){
        sprintf(buff+i*3, "%02X ", out[i]);
    }
    LOGV("play dev out = %s", buff);
    }

    return 0;
}


/*******************************************************************************
���⣺PriorityCmp
���ܣ��������ȼ��Ƚϣ��ȽϹ���Ϊ���������ȼ�����ʱ���ȣ���ʱ���
���룺
�����
����ֵ:���ȼ�j>i����TRUE, ���򷵻�FALSE
*******************************************************************************/
BOOL PriorityCmp(pPLAY_TASK_INFO pi, pPLAY_TASK_INFO pj)
{
    enPRIO_TYPE PrioType = pSysInfo->PrioType;
    
    if(PrioType == PRIO_PRIO_BH || PrioType == PRIO_PRIO_BF){
        if(pi->priority < pj->priority){
            return TRUE;
        }
        else if(pi->priority > pj->priority){
            return FALSE;
        }
    }
    
    if(PrioType==PRIO_BEHIND || PrioType ==PRIO_PRIO_BH){
        return pi->StartTime>pj->StartTime ? FALSE:TRUE;
    }
    else if(PrioType==PRIO_BEFORE || PrioType==PRIO_PRIO_BF){
        return pi->StartTime<pj->StartTime ? FALSE:TRUE;
    }

    return FALSE;
}

BOOL isNeedBroadcast(pPLAY_TASK_INFO pTask)
{
    int i;
    int cnt = 0;

    if(!pTask){
        return FALSE;
    }

    for(i=0; i<pTask->DevListBytes; i++){
        cnt += Bits1InByte[pTask->DevList[i]&0x7f];
        if(cnt >= 2){
            return TRUE;    //���������ϵ��豸����һ��������Ҫ�㲥���豸
        }
    }

    if(cnt==1 && !pTask->isLocalPlay){//ֻ��һ���豸�ֲ��Ǳ��ز��ţ��Ǿ�����Ҫ�㲥����
        return TRUE;
    }
    return FALSE;
}

/*******************************************************************************
���⣺PlayTaskSched
���ܣ������л����ȣ����������(��ʼ���Ż���ͣ�󲥷�)���˳�(��ͣ�����)���������
      �ú����������豸���䣬���ſ���Ȩ���䡣
���룺pThisTask: ״̬�仯������ָ��
�����
����ֵ:
*******************************************************************************/
int PlayTaskSched(pPLAY_TASK_INFO pThisTask)
{
    int i,j;
    int PlayNum = 0;
    pPLAY_TASK_INFO pPlayTask[PLAY_TASK_MAX];


    if(!pThisTask){
        return -1;
    }
    
    //�����ŵ������ҳ�
    for(i=PLAY_TASK_START_INDEX; i<PLAY_TASK_MAX; i++){
        if(PlayTask[i].isUse && PlayTask[i].PlayCtrl==PLAY){
            pPlayTask[PlayNum++] = &PlayTask[i];
        }
    }

    //�����ڲ��ŵ�����������ȼ�����(ð���㷨����)
    for(i=0; i<PlayNum-1; i++){
        for(j=i+1; j<PlayNum; j++){
            if(PriorityCmp(pPlayTask[i], pPlayTask[j])){
                pPLAY_TASK_INFO tmp  = pPlayTask[i];
                pPlayTask[i] = pPlayTask[j];
                pPlayTask[j] = tmp;
            }
        }
    }

    //test
    for(i=0; i<PlayNum; i++){
        LOGV("-->TaskIndex order = %d", pPlayTask[i]->TaskIndex);
    }

    char *this1,*this2,*other1,*other2;
    if(pThisTask->PlayCtrl == PLAY)
    {
        //���Ž��룬�豸����
        //�ȱ��������ȼ��ߣ�������������������豸
        this1 = pThisTask->DevList;
        this2 = pThisTask->DevList2; 
        for(i=0; i<pThisTask->DevListBytes; i++){//
            *this1 = *this2;
            for(j=0; j<PlayNum && pPlayTask[j]!=pThisTask; j++){
                if(i < pPlayTask[j]->DevListBytes){
                    other2 = &pPlayTask[j]->DevList2[i];
                    *this1 = ((*this2^*other2)&*this1)|0x80;
                }
            }
            this1++;
            this2++;
        }

        int DivIndex = j+1;//+1��������������

        //�ȱ��������ȼ��ͣ���������ռ�������豸
        this1 = pThisTask->DevList;
        this2 = pThisTask->DevList2;
        for(i=0; i<pThisTask->DevListBytes; i++){//
            for(j=DivIndex; j<PlayNum; j++){
                if(i < pPlayTask[j]->DevListBytes){
                    other1 = &pPlayTask[j]->DevList[i];
                    *this1 |= (*other1)&(*this2);
                    *other1 = ((*other1^*this2)&*other1)|0x80;
                }
            }
            this1++;
            this2++;
        }
    }
    else
    {
        pThisTask->isPlay = FALSE;
        //�����˳����豸����        
        this1 = pThisTask->DevList;
        for(i=0; i<pThisTask->DevListBytes; i++)
        {
            for(j=0; j<PlayNum; j++){
                if(i < pPlayTask[j]->DevListBytes){
                    other1 = &pPlayTask[j]->DevList[i];
                    other2 = &pPlayTask[j]->DevList2[i];
                    char tmp = *other1;
                    *other1 = ((*this1|tmp)&*other2)|0x80;
                    *this1 = ((~(*other1^tmp))&*this1)|0x80;
                }
            }
            this1++;
        }
        
        if(pThisTask->pRecordBuf){
            PostAudioData(pThisTask->pRecordBuf);
        }
    }

    
    LOGV("========cur local TaskIndex=%d", LocalPlay.ptask->TaskIndex);

    //�ҳ��ĸ�������Ҫ���豸���ţ������Ƿ���Ҫ�㲥
    enPLAY_TYPE PlayType = 0;
    pPLAY_TASK_INFO pPlayTaskTmp;
    pPLAY_TASK_INFO pLocalPlayTask = NULL;
    for(i=0,j=0; i<PlayNum; i++){
        pPlayTaskTmp = pPlayTask[i];
        if(!pLocalPlayTask && pPlayTaskTmp->DevListBytes>pSysInfo->DevListIndex && isHaveSelf(pPlayTaskTmp->DevList)){
            pLocalPlayTask = pPlayTaskTmp;
            pPlayTaskTmp->isLocalPlay = TRUE;
        }
        else{
            pPlayTaskTmp->isLocalPlay = FALSE;
        }
        pPlayTaskTmp->isBroadcast = isNeedBroadcast(pPlayTaskTmp);
        pPlayTaskTmp->isPlay = (pPlayTaskTmp->PlayCtrl==PLAY && (pPlayTaskTmp->isLocalPlay || pPlayTaskTmp->isBroadcast));

        #if 1
        //¼�����⴦������Ҫ���ŵ�¼������ŵ�pRecordTask������
        if(pPlayTaskTmp->isPlay && (pPlayTaskTmp->PlayType==PLAY_RECORD_MIC || pPlayTaskTmp->PlayType==PLAY_RECORD_LINE)){
            if(!PlayType || pPlayTaskTmp->PlayType == PlayType){
                PlayType = pPlayTaskTmp->PlayType;
                pSysInfo->pRecordTask[j++] = pPlayTaskTmp;
            }
            else{
                pPlayTaskTmp->isPlay = FALSE;
            }
        }
        #endif
    }
    pSysInfo->pRecordTask[j] = NULL;

    if(pLocalPlayTask){
        LOGD("========new local TaskIndex=%d", pLocalPlayTask->TaskIndex);
        if(LocalPlay.ptask != pLocalPlayTask){
            LocalPlay.ptask = pLocalPlayTask;
            if(pLocalPlayTask->format){
                PlayLocalHaveNew(CH_FILE);
            }
        }
    }
    else if(pThisTask->PlayCtrl==STOP){
        LocalPlay.ptask = &PlayTask[0];//PlayTask[0]����ʹ�ã������ز�����ʱָ��ýڵ�
        PlayLocalHaveNew(CH_FILE);
    }

    return 0;
}

/*******************************************************************************
���⣺HandPlayFile
���ܣ��ļ������������ļ��б�Ĵ����ϲ㴫�������б��е���Ŀ֮����'|'�ָ�����ģ��
      ��'|'�滻��NULL�����б����������NULL��ʾ������������һ�ס���һ�׵Ĵ���
���룺pTask: ����ָ��
      ctrl: ��ת���Ʊ�ʶ
      file: �����Ʋ��ž�����һ��ʱ����Ŀ��
�����
����ֵ:
*******************************************************************************/
int HandPlayFile(pPLAY_TASK_INFO pTask, enPLAY_CTRL ctrl, const char *file)
{
    char *p;
    
    if(!pTask || !pTask->FileList){
        return -1;
    }
    if(pTask->PlayType != PLAY_FILE){
        return 1;
    }

    if(ctrl == PREV){
        int cnt = 0;
        p = pTask->CurFile;
        while(1){
            if(p == pTask->FileList){
                break;
            }
            else if(*p == 0){
                if(++cnt >= 2){
                    p++;
                    break;
                }
            }
            p--;
        }
        pTask->CurFile = p;
        return 1;
    }
    else if(ctrl == NEXT){
        pTask->CurFile += strlen(pTask->CurFile)+1;
        if(*pTask->CurFile == 0){//����
            pTask->CurFile = pTask->FileList;
            return 0;
        }
        return 1;
    }
    else if(ctrl == CURRENT){
        p = pTask->FileList;
        while(*p){
            if(!strcmp(p, file)){
                pTask->CurFile = p;
                return 1;
            }
            p += strlen(p)+1;
        }
    }
    else if(ctrl == 0){
        //�б��е���Ŀ֮����'|'�ָ��������滻��NULL���б��������2��NULL
        p = pTask->FileList;
        pTask->CurFile = p;
        while(*p){
            if(*p == '|'){
                *p = 0;
            }
            p++;
        }
        *(++p) = 0;
        return 1;
    }
    return -1;   
}

/*******************************************************************************
���⣺SetRecordState
���ܣ�¼��״̬����������¼����ͣ��ֹͣʱ�����ж���������������¼�����еĻ�����
      ִ����ͣ��ֹͣ��
���룺state: ����״̬
�����
����ֵ:
*******************************************************************************/
int SetRecordState(enPLAY_CTRL state)
{
    int TaskIndex;
    
    _lock(&PlayTaskMutex);
    if(state == PLAY){
        SL_SetRecordState(SL_RECORDSTATE_RECORDING);
    }
    else{
        for(TaskIndex=PLAY_TASK_START_INDEX; TaskIndex<PLAY_TASK_MAX; TaskIndex++){
            if(PlayTask[TaskIndex].isUse && PlayTask[TaskIndex].isPlay && \
              (PlayTask[TaskIndex].PlayType==PLAY_RECORD_MIC || PlayTask[TaskIndex].PlayType==PLAY_RECORD_LINE)){
                goto end;
            }
        }
        if(state == PAUSE){
            SL_SetRecordState(SL_RECORDSTATE_STOPPED);
        }
        else{
            SL_RecordStop();
        }
    }

    end:
    _unlock(&PlayTaskMutex);

    return 0;
}

/*******************************************************************************
���⣺ThreadHandlePlayTask
���ܣ������������̣߳�������״̬���ַ����ݣ����Ʋ����豸
���룺p: ����ָ��
�����
����ֵ:
*******************************************************************************/
void *ThreadHandlePlayTask(void *p)
{
    LOGV("Thread run");
    
    int i;
    int FileFd = -1;
    int DataLen;
    char *pSendBuf  = NULL;
    pFFMPEG pFFinfo = NULL;
    pPLAY_TASK_INFO pTaskInfo;
    pRTP_PACKET pRtpPacket;
    struct timeval start;
    unsigned int delay; //����ÿ֡��Ƶ����ʱ��������λ΢��
    int BaseTimeCnt;    //����ʱ������
    int SendCtrlTimeCnt;//���Ͳ��ſ��Ƽ��ʱ�����
    int volume;

    if(!p){
        return NULL;
    }
    pTaskInfo = (pPLAY_TASK_INFO)p;
    
    //¼����Ҫ����¼�����ݽ��ջ�����
    if(pTaskInfo->PlayType == PLAY_RECORD_MIC || pTaskInfo->PlayType == PLAY_RECORD_LINE){
        pTaskInfo->pRecordBuf = RingBuffCreate(NULL, RECROD_BUF_SIZE, DYNAMIC_REQ_MEM);
        if(!pTaskInfo->pRecordBuf){
            goto AllEnd;
        }
    }
    //�������ļ��б�
    else{
        if(HandPlayFile(pTaskInfo, 0, NULL) < 0){
            goto AllEnd;
        }
    }
    pTaskInfo->SrcDevType = pSysInfo->DevType;
    pTaskInfo->SrcDevNum = pSysInfo->DevNum;
    pTaskInfo->SrcPlayID = pTaskInfo->TaskIndex;
    pTaskInfo->SrcPlayNum = 0;
    snprintf(pTaskInfo->GroupIP.ip, sizeof(pTaskInfo->GroupIP.ip), "225.%d.%d.%d", (pSysInfo->DevNum>>8)&0xff, pSysInfo->DevNum&0xff, pTaskInfo->TaskIndex);   
    pTaskInfo->GroupIP.addr = inet_addr(pTaskInfo->GroupIP.ip);

    pSendBuf = (char *)_malloc(AUDIO_SEND_BUF_SIZE+sizeof(RTP_HEADER));
    if(!pSendBuf){
        goto AllEnd;
    }
    bzero(pSendBuf, sizeof(RTP_HEADER));
    pRtpPacket = (pRTP_PACKET)pSendBuf;
    pRtpPacket->header.ssrc = pTaskInfo->StartTime;

    volume = pTaskInfo->volume;

    //��Ŀ�б�ѭ������pTaskInfo->PlayTimes��
    while((pTaskInfo->PlayTimes--)>0 && pTaskInfo->PlayCtrl!=STOP)
    {
        pTaskInfo->CurFile = pTaskInfo->FileList;
        //��Ŀ�б���1��
        while(pTaskInfo->PlayCtrl != STOP)
        {
            LOGD("------>TaskIndex=%d, %s", pTaskInfo->TaskIndex, pTaskInfo->CurFile);
            
            pRtpPacket->header.sn = 0;

            //�ļ��㲥
            if(pTaskInfo->PlayType == PLAY_FILE)
            {                
                FileFd = open(pTaskInfo->CurFile, O_RDONLY);
                if(FileFd < 0){
                    LOGW("Task %d open file fail-->%s", pTaskInfo->TaskIndex, pTaskInfo->CurFile);
                    goto OneEnd;
                }

                if(0 == WaveReadHeader(FileFd, &pTaskInfo->pcm, NULL)){
                    //WAV�ļ�
                    pTaskInfo->format = FORMAT_WAV;
                    delay = (AUDIO_SEND_SIZE*1000000)/(pTaskInfo->pcm.rate*pTaskInfo->pcm.channels*pTaskInfo->pcm.bitsPerSample/8);
                }
                else{
                    //MP3�ļ�
                    if(!(pFFinfo = FFmpegCreateAudioPlay(NULL, pTaskInfo->CurFile, &pTaskInfo->pcm))){
                        goto OneEnd;
                    }
                    //delay = 25900;
                    delay = (MP3_FRAME_SIZE*1000000)/pTaskInfo->pcm.rate;
                    pTaskInfo->format = FORMAT_MP3;

                    LOGV("FFmpegCreateAudioPlay ok");
                }
                
                PlayCallJava(pTaskInfo->TaskIndex, CURRENT, pTaskInfo->CurFile);
            }
            //¼���㲥
            else if(pTaskInfo->PlayType == PLAY_RECORD_MIC || pTaskInfo->PlayType == PLAY_RECORD_LINE)
            {
                pTaskInfo->pcm.rate = 16000;
                pTaskInfo->pcm.channels = 2;
                pTaskInfo->pcm.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
                delay = (AUDIO_SEND_SIZE*1000000)/(pTaskInfo->pcm.rate*pTaskInfo->pcm.channels*pTaskInfo->pcm.bitsPerSample/8);
                #if(USE_G711)
                delay <<= 1;//delay *= 2; G711ѹ�����������ԭʼ���ݵ�2����delayҲҪ����2
                pTaskInfo->format = FORMAT_G711U;
                #else
                pTaskInfo->format = FORMAT_WAV;
                #endif
                
                LOGV("task=%d SL_Record", pTaskInfo->TaskIndex);
                SL_Record(&pTaskInfo->pcm);
                //PlayCallJava();//�����ϲ��л�ͨ��
            }
            else{
                goto AllEnd;
            }

            pTaskInfo->SrcPlayNum++;
            if(pTaskInfo->isLocalPlay){
                PlayLocalHaveNew(CH_FILE);
            }
            if(pTaskInfo->isBroadcast){
                SendAudioCtrl(pTaskInfo);
            }

            delay = (delay*9997)/10000; //���ݷ����ٶ�΢����һ��ʹdelay�Լ�Сһ�㣬ʹ�������ݳ��㣬��΢���Ӱ�첻��
            BaseTimeCnt = (TIME_BASE*1000)/delay;
            SendCtrlTimeCnt = (SEND_PLAY_CTRL_INTERVAL/TIME_BASE);
            LOGV("delay=%d, BaseTimeCnt=%d, SendCtrlTimeCnt=%d", delay, BaseTimeCnt, SendCtrlTimeCnt);

            gettimeofday(&start,0);
            msleep(150);
           
            //������Ŀ�б��е�1��
            while(pTaskInfo->PlayCtrl != STOP)
            {
                if(pTaskInfo->isPlay)
                {
                    if(pTaskInfo->format == FORMAT_MP3)
                    {   //ȡMP3����
                        DataLen = FFmpegReadFrame(pFFinfo, &pRtpPacket->data, AUDIO_SEND_BUF_SIZE);
                        DelayUs(&start, delay);
                    }
                    else if(pTaskInfo->format == FORMAT_WAV)
                    {   //ȡWAV����
                        DataLen = read(FileFd, &pRtpPacket->data, AUDIO_SEND_SIZE);
                        DelayUs(&start, delay);
                    }
                    else if(pTaskInfo->PlayType==PLAY_RECORD_MIC || pTaskInfo->PlayType==PLAY_RECORD_LINE)
                    {   //ȡ¼������
                        if((DataLen=GetAudioData(pTaskInfo->pRecordBuf, (char *)&pRtpPacket->data, AUDIO_SEND_SIZE))<=0){
                            WaitAudioData(pTaskInfo->pRecordBuf);
                            continue;
                        }
                    }
                    else{
                        goto OneEnd;
                    }
                    
                    if(DataLen <= 0)
                    {   //������1��
                        LOGV("OneEnd DataLen=%d", DataLen);
                        break;
                    }
                    
                    //��Ҫ�㲥����ʱ���Ͳ��ſ���ָ�������Ƶ����
                    if(pTaskInfo->isBroadcast)
                    {
                        pRtpPacket->header.sn++;
                        SendAudioData(pTaskInfo, pRtpPacket, DataLen+sizeof(RTP_HEADER));
                    }
                    
                    //��Ҫ�������ţ�����Ƶ�������͵���Ƶ���Ż�����
                    if(pTaskInfo->isLocalPlay)
                    {
                        int AvaDataSize;
                        int len = WriteAudioData(gs_PlayAudioData, (char *)&pRtpPacket->data, DataLen, &AvaDataSize);
                        if(AvaDataSize<=DataLen || len!=DataLen){
                            PostAudioData(gs_PlayAudioData);
                            if(len != DataLen){
                                LOGV("task=%d,size=%d, write=%d", pTaskInfo->TaskIndex, DataLen, len);
                            }
                        }
                    }

                    if(--BaseTimeCnt <= 0)
                    {
                        BaseTimeCnt = (TIME_BASE*1000)/delay; //��������������Ҫ��ȷ

                        if(pTaskInfo->isHaveNew){
                            pTaskInfo->isHaveNew = FALSE;
                            break;
                        }

                        if(volume != pTaskInfo->volume){
                            volume = pTaskInfo->volume;
                            if(pTaskInfo->isLocalPlay){
                                PlayLocalHaveNew(CH_VOLUME);
                            }
                            if(pTaskInfo->isBroadcast){
                                SendAudioCtrl(pTaskInfo);
                            }
                        }
                        
                        if(pTaskInfo->isBroadcast && --SendCtrlTimeCnt <= 0){
                            SendCtrlTimeCnt = (SEND_PLAY_CTRL_INTERVAL/TIME_BASE);
                            SendAudioCtrl(pTaskInfo);
                        }
                    }
                }
                else
                {
                    SendAudioCtrl(pTaskInfo);
                    if(pTaskInfo->PlayType==PLAY_RECORD_MIC || pTaskInfo->PlayType==PLAY_RECORD_LINE){
                        SetRecordState(PAUSE);
                    }
                    while(pTaskInfo->PlayCtrl!=STOP && !pTaskInfo->isPlay){
                        msleep(200);
                        if(pTaskInfo->isHaveNew){
                            pTaskInfo->isHaveNew = FALSE;
                            goto OneEnd;
                        }
                    }
                    SendAudioCtrl(pTaskInfo);
                    msleep(150);
                    if(pTaskInfo->PlayType==PLAY_RECORD_MIC || pTaskInfo->PlayType==PLAY_RECORD_LINE){
                        SetRecordState(PLAY);
                        //PlayCallJava();//�����ϲ��л�ͨ��
                    }
                }
            }

            OneEnd:
            LOGV("play task %d one end", pTaskInfo->TaskIndex);
            
            if(FileFd > 0){
                close(FileFd);
                FileFd = -1;
            }
            if(pFFinfo){
                FFmpegReleaseAudioPlay(pFFinfo);
                pFFinfo = NULL;
            }

            if(DataLen<=0 && HandPlayFile(pTaskInfo, NEXT, NULL)==0){
                break;
            }
        }
    }
    
    AllEnd:

    LOGV("play task %d end", pTaskInfo->TaskIndex);

    if(pTaskInfo->PlayCtrl != STOP || !pSysInfo->bRun){
        HandPlayTaskStatus(pTaskInfo->TaskIndex, STOP);
        PlayCallJava(pTaskInfo->TaskIndex, STOP, " ");
    }
    SendAudioCtrl(pTaskInfo);

    if(pTaskInfo->PlayType==PLAY_RECORD_MIC || pTaskInfo->PlayType==PLAY_RECORD_LINE){
        SetRecordState(STOP);
    }

    if(pFFinfo){
        FFmpegReleaseAudioPlay(pFFinfo);
    }

    if(FileFd > 0){
        close(FileFd);
    }
    if(pSendBuf){
        _free(pSendBuf);
    }
    
    msleep(100);
    FreePlayTask(pTaskInfo);

    return 0;
}


/*******************************************************************************
���⣺AllocPlayTask
���ܣ���������ʱ����һ������ڵ㣬��һЩ��س�ʼ��
���룺PlayType: ��������
      fileList: ��Ŀ�б�����������
      devList:  ��Ҫִ�е��豸����
      DevListBytes: devList�ĳ���
      times: ����ѭ������
      defVol: Ĭ������
�����
����ֵ:
*******************************************************************************/
int AllocPlayTask(enPLAY_TYPE PlayType, const char *fileList, const char *devList, 
                     int DevListBytes, int times, int defVol)
{
    int TaskIndex;
    pPLAY_TASK_INFO pTask = NULL;

    _lock(&PlayTaskMutex);
    if(PlayType == PLAY_RECORD_MIC /*|| PlayType == PLAY_RECORD_LINE*/){//¼��������ͬʱ���ڶ���
        for(TaskIndex=PLAY_TASK_START_INDEX; TaskIndex<PLAY_TASK_MAX; TaskIndex++){
            if(PlayTask[TaskIndex].isUse && PlayTask[TaskIndex].PlayType==PlayType){
                goto unlock;
            }
        }
    }
    for(TaskIndex=PLAY_TASK_START_INDEX; TaskIndex<PLAY_TASK_MAX; TaskIndex++){
        if(!PlayTask[TaskIndex].isUse){
            pTask = &PlayTask[TaskIndex];
            bzero(pTask, sizeof(PLAY_TASK_INFO));
            pTask->TaskIndex = TaskIndex;
            pTask->isUse = TRUE;
            break;
        }
    }
    unlock:
    _unlock(&PlayTaskMutex);
    
    if(!pTask){
        return -1;
    }

    LOGD("TaskIndex=%d PlayType=%d", pTask->TaskIndex, PlayType);
    
    pTask->PlayType = PlayType;
    pTask->volume = defVol;
    pTask->priority = 1;

    if(PlayType==PLAY_FILE || PlayType==PLAY_RECORD_MIC || PlayType==PLAY_RECORD_LINE)
    {
        pTask->FileList = (char *)_malloc(strlen(fileList)+2);
        if(!pTask->FileList){
            goto err;
        }
        strcpy(pTask->FileList, fileList);

        DevListBytes = max_valid_bytes(devList, DevListBytes);
        pTask->DevListBytes = (DevListBytes*8)/7 + ((DevListBytes*8)%7 ? 1:0);//ui�㰴ÿ�ֽ�8λ��Чλ�·����˴���7λ��Чλ�棬�ֽ���������
        pTask->DevList = _malloc(pTask->DevListBytes*2+2);
        if(!pTask->DevList){
            goto err;
        }
        bits8_bits7(devList, DevListBytes, pTask->DevList, pTask->DevListBytes);
        pTask->DevList[pTask->DevListBytes] = 0;
        if((PlayType==PLAY_RECORD_MIC || PlayType==PLAY_RECORD_LINE) && pTask->DevListBytes>pSysInfo->DevListIndex){
            //DecSelf(pTask->DevList);  //MIC¼��Ҫ��֤�����ط�
        }
        pTask->DevList2 = &pTask->DevList[pTask->DevListBytes+1];
        memcpy(pTask->DevList2, pTask->DevList, pTask->DevListBytes+1);
        
        pTask->PlayTimes = times;//test
        pTask->StartTime = time(NULL);

        HandPlayTaskStatus(pTask->TaskIndex, PLAY);
        if(0 != ThrdPlAlloc(ThreadHandlePlayTask, pTask, sizeof(PLAY_TASK_INFO))){
            goto err;
        }
        msleep(5);
    }
    else if(PlayType == PLAY_NET)
    {
        pTask->FileList = (char *)_malloc(NET_PLAY_NAME_SIZE);
        if(!pTask->FileList){
            goto err;
        }
        
        pTask->DevListBytes = pSysInfo->DevListIndex+1;
        pTask->DevList = _malloc(pTask->DevListBytes*2+2);
        if(!pTask->DevList){
            goto err;
        }
        memset(pTask->DevList, 0x80, pTask->DevListBytes);
        pTask->DevList[pTask->DevListBytes] = 0;
        AddSelf(pTask->DevList);
        pTask->DevList2 = &pTask->DevList[pTask->DevListBytes+1];
        memcpy(pTask->DevList2, pTask->DevList, pTask->DevListBytes+1);
        pTask->LastTime = time(NULL);
    }
    else{
        goto err;
    }

    return pTask->TaskIndex;

    err:
    FreePlayTask(pTask);

    return -1;
}

/*******************************************************************************
���⣺FreePlayTask
���ܣ������˳�ʱ�ͷ������Դ
���룺ptask: ����ָ��
�����
����ֵ:
*******************************************************************************/
int FreePlayTask(pPLAY_TASK_INFO ptask)
{
    if(!ptask){
        return -1;
    }

    LOGV("FreePlayTask %d", ptask->TaskIndex);

    _lock(&PlayTaskMutex);

    if(ptask->DevList){
        _free(ptask->DevList);
        ptask->DevList = NULL;
    }
    if(ptask->FileList){
        _free(ptask->FileList);
        ptask->FileList = NULL;
    }
    if(ptask->pRecordBuf){
        RingBuffDestroy(ptask->pRecordBuf);
    }

    memset(ptask, 0, sizeof(PLAY_TASK_INFO));
    
    _unlock(&PlayTaskMutex);

    return 0;
}




/*******************************************************************************
���⣺HandPlayTaskStatus
���ܣ�ui��ı䲥��״̬�Ĵ���
���룺id: һ����������ľ��(��Ӧ�������е�PlayTask�����±�)
      status: ���Ƹı��״̬
�����
����ֵ: 0: �ɹ�������: ʧ��
*******************************************************************************/
int HandPlayTaskStatus(int id, enPLAY_CTRL status)
{
    int i;
    int ret = 0;
    
    _lock(&PlayTaskMutex);
    
    if(!isVALID_TASK_ID(id) || !PlayTask[id].isUse){
        ret = -1;
        goto out;
    }

    if(status==PREV || status==NEXT || status==CURRENT){
        if(HandPlayFile(&PlayTask[id], status, NULL) < 0){
            goto out;
        }
        PlayTask[id].isHaveNew = TRUE;
        if(PlayTask[id].PlayCtrl == PLAY){
            goto out;
        }
        status = PLAY;
    }

    LOGD("id = %d, status=%d", id, status);

    PlayTask[id].PlayCtrl = status;
    PlayTaskSched(&PlayTask[id]);

    uint8 tmp = 0;
    for(i=PLAY_TASK_START_INDEX; i<PLAY_TASK_MAX; i++){
        if(PlayTask[i].isUse && (PlayTask[i].PlayCtrl==PLAY || PlayTask[i].PlayCtrl==PAUSE)){
            if(PlayTask[i].PlayType == PLAY_FILE){
                tmp |= 1;
            }
            else if(PlayTask[i].PlayType == PLAY_RECORD_MIC){
                tmp |= 1<<2;
            }
            else if(PlayTask[i].PlayType == PLAY_RECORD_LINE){
                tmp |= 1<<4;
            }
            else if(PlayTask[i].PlayType == PLAY_NET){
                tmp |= 1<<6;
            }
        }
    }
    pSysInfo->PlayStatus = tmp;

    out:
    _unlock(&PlayTaskMutex);

    return ret;
}

int HandSetVol(int id, int volume)
{
    int ret = -1;
    
    _lock(&PlayTaskMutex);
    if(isVALID_TASK_ID(id) && PlayTask[id].isUse && isVALID_VOL(volume)){
        PlayTask[id].volume = volume;
        ret = 0;
    }
    _unlock(&PlayTaskMutex);

    return ret;
}

int HandSetMute(BOOL isMute)
{
    _lock(&PlayTaskMutex);
    LOGD("isMute = %d", isMute);
    LocalPlay.isMute = isMute;
    PlayLocalHaveNew(CH_MUTE);
    _unlock(&PlayTaskMutex);

    return 0;
}

void PlayLocalHaveNew(enPLAY_CH type)
{
    LOGV("PlayLocalHaveNew = 0x%02x", type);
    LocalPlay.NewFlag |= type;
    if(type == CH_FILE){
        PostAudioData(gs_PlayAudioData);
    }
    _sem_post(&LocalPlay.sem);
}

/*******************************************************************************
���⣺ThreadPlayLocal
���ܣ������������̣߳���Ҫ��ffmpeg�⡢opensl��ĵ��á�
���룺
�����
����ֵ:
*******************************************************************************/
void *ThreadPlayLocal(void *p)
{
    LOGV("Thread start");
    int i;
    pLOCAL_PLAY pLP = &LocalPlay;
    
    while(pSysInfo->bRun)
    {
        WaitAudioData(gs_PlayAudioData);

        #if 1
        if(pLP->isMute)
        {
            CleanAudioData(gs_PlayAudioData);
            continue;
        }
        #endif
        
        _lock(&PlayTaskMutex);
        if(pLP->ptask->format){
            pLP->format = pLP->ptask->format;
            pLP->NewFlag = 0;
            memcpy(&pLP->pcm, &pLP->ptask->pcm, sizeof(PCM_PARAM));
        }
        _unlock(&PlayTaskMutex);
        
        if(pLP->format == 0){
            continue;
        }
        if(pLP->format == FORMAT_MP3){
            if(NULL == FFmpegCreateAudioPlay(&pLP->pffm, NULL, &pLP->pcm)){
                goto release;
            }
            #if 1
            //SL_Play��һС��ʱ��û�лص�ȡ���ݣ�������Ƶ�������������ݣ�
            //���Ż����"֨"һ����OPENSL��ʱû�ص�ԭ��δ�ҵ���Ϊ��ܲ�
            //��"֨"һ�������ｫffmpeg���յ�����������
            for(i=0; i<6 && !isBitSet(pLP->NewFlag, CH_FILE); i++){
                FFmpegReadFrame(pLP->pffm, NULL, 0);
            }
            #endif
        }
        
        LOGD("rate=%d, channel=%d, bitsPerSample=%d", pLP->pcm.rate, pLP->pcm.channels, pLP->pcm.bitsPerSample);
        
        if(0 != SL_Play(&pLP->pcm)){
            goto release; 
        }
        pLP->volume = VOL_MAX;
        if(pLP->isMute){
            SL_SetMute(pLP->isMute);
        }
        
        while(pLP->ptask->PlayCtrl != STOP)
        {
            if(isBitSet(pLP->NewFlag, CH_FILE)){
                break;
            }
            
            if(isBitSet(pLP->NewFlag, CH_RESET_PLAYER)){
                break;
            }
            
            if(pLP->isMute){
                //SetBit(pLP->NewFlag, CH_RESET_PLAYER);
                break; //������ʱʵ��Ϊֹͣ���ز���
            }
            
            if(isBitSet(pLP->NewFlag, CH_MUTE)){
                SL_SetMute(pLP->isMute);
            }

            if(pLP->volume != pLP->ptask->volume){
                pLP->volume = pLP->ptask->volume;
                SL_SetVolume(vol_conversion(pLP->volume));
            }
            pLP->NewFlag = 0;
            
            _sem_wait(&pLP->sem);
        }
    
        release:
        LOGV("release");
        pLP->format = 0;
        PostAudioData(gs_PlayAudioData);
        if(isBitSet(pLP->NewFlag, CH_RESET_PLAYER))SL_PlayShutdown();
        else SL_PlayStop();
        FFmpegReleaseAudioPlay(pLP->pffm);
        pLP->pffm = NULL;
        msleep(50);
        CleanAudioData(gs_PlayAudioData);
    }

    LOGV("Thread end");
}



/*******************************************************************************
���⣺InitPlayTask
���ܣ�������������صĳ�ʼ��
���룺��
�������
����ֵ:
*******************************************************************************/
int InitPlayTask(void)
{
    _lock_init(&PlayTaskMutex, NULL);
    bzero(PlayTask, sizeof(PlayTask));
    bzero(&LocalPlay, sizeof(LocalPlay));
    _sem_init(&LocalPlay.sem, 0, 0); 
    LocalPlay.ptask = &PlayTask[0];
    if(!gs_PlayAudioData){
        gs_PlayAudioData = RingBuffCreate(NULL, AUDIO_BUF_SIZE, DYNAMIC_REQ_MEM);
    }
    ThreadCreate(ThreadPlayLocal, NULL);

    return 0;
}

int DelInitPlayTask(void)
{
    int i;
    for(i=PLAY_TASK_START_INDEX; i<PLAY_TASK_MAX; i++){
        PlayTask[i].PlayCtrl = STOP;
        if(PlayTask[i].pRecordBuf){
            PostAudioData(PlayTask[i].pRecordBuf);
        }
    }
    _sem_post(&LocalPlay.sem);
    PostAudioData(gs_PlayAudioData);
    msleep(500);
    _sem_destroy(&LocalPlay.sem); 
    _lock_destroy(&PlayTaskMutex);
    RingBuffDestroy(gs_PlayAudioData);
    gs_PlayAudioData = NULL;

    return 0;
}





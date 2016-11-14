/*****************************************************************

******************************************************************/

#ifndef __AUDIO_PLAYER_H__
#define __AUDIO_PLAYER_H__

#include "common.h"
#include "TypepDef.h"
#include "ringbuffer.h"
#include "OpenSlApi.h"
#include "FFmpegApi.h"
#include "wavfile.h"

#ifdef __cplusplus
extern "C" {
#endif

#define USE_G711                1 //¼�������Ƿ�����G711ѹ����0�����ã�1����

#define PLAY_TASK_MAX           30
#define PLAY_TASK_START_INDEX   1
#define isVALID_TASK_ID(id)     (id>=PLAY_TASK_START_INDEX && id < PLAY_TASK_MAX)

#define NET_PLAY_NAME_SIZE      128
#define AUDIO_SEND_BUF_SIZE     1200
#define AUDIO_BUF_SIZE          (4*1024)
#define RECROD_BUF_SIZE         (4*1024)
#define AUDIO_SEND_SIZE         (1024)

#define AUDIO_BUF_RING_NUM       4

#define TIME_BASE               (300) //��λ:����
#define SEND_PLAY_CTRL_INTERVAL (3500)//��λ:����

#define MP3_FRAME_SIZE          (1152)

#define DEF_DEV_LIST_SIZE       (1024)

#define isSelf(num)             (num == pSysInfo->DevNum)
#define isHaveSelf(buf)         (buf[pSysInfo->DevListIndex] &  pSysInfo->DevMask)
#define AddSelf(buf)            (buf[pSysInfo->DevListIndex] |= pSysInfo->DevMask)
#define DecSelf(buf)            (buf[pSysInfo->DevListIndex] &= ~(pSysInfo->DevMask))

#define isBitSet(x, y)          ((x) & (y))
#define ClearBit(x, y)          ((x) &= (~y))
#define SetBit(x, y)            ((x) |= y)


#define VOL_MIN                 (0)
#define VOL_MAX                 (100)
#define isVALID_VOL(vol)        (vol>=VOL_MIN && vol<=VOL_MAX)
#define vol_conversion(vol)     ((vol-VOL_MAX)*50)


typedef enum
{
    DEV_HOST=1,         //����
    DEV_SUB             //�ֻ�
}enDEV_TYPE;


typedef enum
{
    FORMAT_NULL,
    FORMAT_WAV,
    FORMAT_MP3,
    FORMAT_G711U,
    FORMAT_G711A,
}enAUDIO_FORMAT;

typedef enum
{
    STOP=0,       	//0��ֹ��ֵ���ܸ�
    PLAY,       	//1����
    PAUSE,    	    //2��ͣ
    CURRENT,        //3��ǰ����
    PREV,           //4ǰһ��
    NEXT,           //5��һ��
    VOLUME,         //��������
    MUTE,           //��������

    NET_PLAY = 100  //���粥��
}enPLAY_CTRL;

typedef enum
{
    PLAY_FILE=1,        //1�����ļ�
    PLAY_RECORD_MIC,    //2MIC�ɲ�
    PLAY_RECORD_LINE,   //3��������ɲ�
    PLAY_NET            //4��������
}enPLAY_TYPE;

typedef enum
{
    CH_FILE         =0x01, //�ı䲥���ļ�
    CH_VOLUME       =0x02, //�ı�����
    CH_MUTE         =0x04, //�ı侲��
    CH_RESET_PLAYER =0x08, //����������
}enPLAY_CH;

typedef enum
{
    SEND_MODE_MC=1, //�鲥��ʽ
    SEND_MODE_P2P   //��Ե㷽ʽ
}enSEND_MODE;

typedef enum
{
    PRIO_BEFORE=1,  //1������ʱ��������
    PRIO_BEHIND,    //2������ʱ�������
    PRIO_PRIO_BF,   //3���������ȼ��������ȼ���ͬ������ʱ��������
    PRIO_PRIO_BH    //4���������ȼ��������ȼ���ͬ������ʱ�������
}enPRIO_TYPE;


typedef struct
{
    int TaskIndex;
    int priority;
    ULONG StartTime;
}PRIO_CMP,*pPRIO_CMP;

typedef struct 
{
    char ip[16];
    unsigned long addr;
    UINT32 isMC;
}IP_ADDR, *pIP_ADDR;

typedef struct 
{
    int num;
    char ip[16];
    unsigned long addr;
    UINT32 isMC;
}DEV_IF, *pDEV_IF;


typedef struct{
    void *next;
    BOOL isUse;         //����ڵ��Ƿ���ʹ��
    int TaskIndex;      //����ڵ��
    volatile enPLAY_CTRL PlayCtrl;//���񲥷ſ���
    enPLAY_TYPE PlayType;//��������
    int PlayTimes;      //���񲥷�ѭ������
    int priority;       //�������ȼ�
    ULONG StartTime;    //��������ʱ��
    char DevListBytes;  //�豸�б���(�ֽ�)
    char *DevList;      //������ʵ���豸�б��豸�ᱻ���ȼ��ߵĲ���������ռ
    char *DevList2;     //ui�·���ԭʼ�豸�б�ָ��DevList�ĺ�벿
    char *FileList;     //���񲥷��ļ��б���������
    char *CurFile;
    enSEND_MODE SendMode;
    pRingBuffer pRecordBuf;
    volatile BOOL isPlay;       //�����Ƿ���Ҫ���ţ���PlayCtrl��isLocalPlay��isBroadcast���ۺϽ��
    volatile BOOL isLocalPlay;  //�Ƿ���Ҫ���ز���
    volatile BOOL isBroadcast;  //�Ƿ���Ҫ�㲥
    volatile BOOL isHaveNew;    //�и��ʶ
    BOOL isMute;            //�Ƿ���
    int volume;             //����
    enAUDIO_FORMAT format;  //��Ƶ��ʽ
    PCM_PARAM pcm;          //PCM��������
    ULONG LastTime;         //�ϴν��յ����粥��ָ���ʱ���
    //����ֻ���粥���õ��Ĳ���
    int SrcDevType;         //Դ�豸����
    int SrcDevNum;          //Դ�豸���
    int SrcPlayID;          //Դ�豸��������id
    int SrcPlayNum;         //Դ�豸������Ŀ��
    int GroupFlag;          //�鲥��仯��ʶ
    IP_ADDR GroupIP;        //�鲥��ip
}PLAY_TASK_INFO, *pPLAY_TASK_INFO;


typedef struct
{
    unsigned char cc:4;  
    unsigned char x:1;  
    unsigned char p:1;  
    unsigned char v:2;  
      
    unsigned char pt:7;  
    unsigned char m:1;  
  
    unsigned short sn;  
    unsigned int timestamp;  
    unsigned int ssrc; 
}RTP_HEADER, *pRTP_HEADER;

typedef struct
{
    RTP_HEADER header;
    char *data;
}RTP_PACKET, *pRTP_PACKET;


typedef struct
{
    int NewFlag;
    BOOL isMute;    ///1������0ȡ������
    int volume;
    enAUDIO_FORMAT format;
    PCM_PARAM pcm;
    pPLAY_TASK_INFO volatile ptask;
    pSL pSLObj;
    pFFMPEG pffm;
    _sem_t sem;
}LOCAL_PLAY, *pLOCAL_PLAY;

extern LOCAL_PLAY LocalPlay;
extern pRingBuffer gs_PlayAudioData;

extern PLAY_TASK_INFO PlayTask[PLAY_TASK_MAX];
extern _lock_t PlayTaskMutex;

int SL_GetPcmData(void **pcm);
void SL_WriteRecordData(char *data, int len);
int GetStreamData(void *opaque, uint8_t *buf, int buf_size);

int AllocPlayTask(enPLAY_TYPE PlayType, const char *fileList, const char *devList, int len, int times, int defVol);
int HandPlayTaskStatus(int id, enPLAY_CTRL status);
void PlayLocalHaveNew(enPLAY_CH type);

int InitPlayTask(void);
int DelInitPlayTask(void);

#ifdef __cplusplus
}
#endif

#endif


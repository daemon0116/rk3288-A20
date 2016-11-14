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

#define USE_G711                1 //录音数据是否启用G711压缩，0不启用，1启用

#define PLAY_TASK_MAX           30
#define PLAY_TASK_START_INDEX   1
#define isVALID_TASK_ID(id)     (id>=PLAY_TASK_START_INDEX && id < PLAY_TASK_MAX)

#define NET_PLAY_NAME_SIZE      128
#define AUDIO_SEND_BUF_SIZE     1200
#define AUDIO_BUF_SIZE          (4*1024)
#define RECROD_BUF_SIZE         (4*1024)
#define AUDIO_SEND_SIZE         (1024)

#define AUDIO_BUF_RING_NUM       4

#define TIME_BASE               (300) //单位:毫秒
#define SEND_PLAY_CTRL_INTERVAL (3500)//单位:毫秒

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
    DEV_HOST=1,         //主机
    DEV_SUB             //分机
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
    STOP=0,       	//0静止，值不能改
    PLAY,       	//1播放
    PAUSE,    	    //2暂停
    CURRENT,        //3当前播放
    PREV,           //4前一曲
    NEXT,           //5下一曲
    VOLUME,         //音量控制
    MUTE,           //静音控制

    NET_PLAY = 100  //网络播放
}enPLAY_CTRL;

typedef enum
{
    PLAY_FILE=1,        //1播放文件
    PLAY_RECORD_MIC,    //2MIC采播
    PLAY_RECORD_LINE,   //3线性输入采播
    PLAY_NET            //4播放网络
}enPLAY_TYPE;

typedef enum
{
    CH_FILE         =0x01, //改变播放文件
    CH_VOLUME       =0x02, //改变音量
    CH_MUTE         =0x04, //改变静音
    CH_RESET_PLAYER =0x08, //重启播放器
}enPLAY_CH;

typedef enum
{
    SEND_MODE_MC=1, //组播方式
    SEND_MODE_P2P   //点对点方式
}enSEND_MODE;

typedef enum
{
    PRIO_BEFORE=1,  //1按播放时间先优先
    PRIO_BEHIND,    //2按播放时间后优先
    PRIO_PRIO_BF,   //3按任务优先级，如优先级相同按播放时间先优先
    PRIO_PRIO_BH    //4按任务优先级，如优先级相同按播放时间后优先
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
    BOOL isUse;         //任务节点是否在使用
    int TaskIndex;      //任务节点号
    volatile enPLAY_CTRL PlayCtrl;//任务播放控制
    enPLAY_TYPE PlayType;//播放类型
    int PlayTimes;      //任务播放循环次数
    int priority;       //任务优先级
    ULONG StartTime;    //任务启动时间
    char DevListBytes;  //设备列表长度(字节)
    char *DevList;      //运行中实际设备列表，设备会被优先级高的播放任务抢占
    char *DevList2;     //ui下发的原始设备列表，指向DevList的后半部
    char *FileList;     //任务播放文件列表，或任务名
    char *CurFile;
    enSEND_MODE SendMode;
    pRingBuffer pRecordBuf;
    volatile BOOL isPlay;       //任务是否需要播放，集PlayCtrl、isLocalPlay、isBroadcast的综合结果
    volatile BOOL isLocalPlay;  //是否需要本地播放
    volatile BOOL isBroadcast;  //是否需要广播
    volatile BOOL isHaveNew;    //切歌标识
    BOOL isMute;            //是否静音
    int volume;             //音量
    enAUDIO_FORMAT format;  //音频格式
    PCM_PARAM pcm;          //PCM几个参数
    ULONG LastTime;         //上次接收到网络播放指令的时间戳
    //以下只网络播放用到的参数
    int SrcDevType;         //源设备类型
    int SrcDevNum;          //源设备编号
    int SrcPlayID;          //源设备播放任务id
    int SrcPlayNum;         //源设备播放曲目号
    int GroupFlag;          //组播组变化标识
    IP_ADDR GroupIP;        //组播组ip
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
    BOOL isMute;    ///1静音，0取消静音
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


/*****************************************************************

******************************************************************/

#ifndef __NET_WORK_H__
#define __NET_WORK_H__

#include "TypepDef.h"
#include "linkbuffer.h"
#include "AudioPlayer.h"
#include "common.h"


#ifdef __cplusplus
extern "C" {
#endif

#define RECV_PUBLIC_GROUD_BUF_SIZE  (1600)

#define RECV_AUDIO_BUF_SIZE         (1600)
#define RECV_SEARCH_BUF_SIZE        (1600)
#define RECV_PLAY_CTRL_TIMEOUT      (5)//单位秒,要与SEND_PLAY_CTRL_INTERVAL配合

#define SVR_PORT                    (5090)
#define PUBLIC_GROUP_PORT           (6001)
#define AUDIO_CTRL_PORT             (6002)
#define AUDIO_DATA_PORT             (6003)
#define AUDIO_SEARCH_PORT           (5555)

#define SEND_AUDIO_SOCK_NUM         30

#define DEF_DEVS_IP_NUM             16


#define USE_MULTICAST_AUDIO_CTRL    0


#ifndef USE_MULTICAST_AUDIO_DATA    ///1:组播，0:P2P (发送音频数据)
#define USE_MULTICAST_AUDIO_DATA    1///0///
#endif

#ifndef USE_MULTICAST_AUDIO_CTRL    ///1:组播，0:P2P (发送播放控制指令)
#define USE_MULTICAST_AUDIO_CTRL    0
#endif

#if USE_MULTICAST_AUDIO_CTRL
#define USE_PUBLIC_GROUP            0
#endif

#ifndef USE_PUBLIC_GROUP            ///1:使用公共组，0:不使用公共组
#define USE_PUBLIC_GROUP            0
#endif

#define THREAD_HANDLE_CMD


typedef struct 
{
    uint16 port;
    char ip[16];
    int  dataLen;
    char *pdata;
}NET_RECV, *pNET_RECV;

typedef struct 
{
    int  CmdId;
    const char *CmdStr;
    int (*func)(pNET_RECV pNetRecv, const char *pdata);
}FUNC_ROUTER, *pFUNC_ROUTER;

typedef struct 
{
    BOOL flag;
    int sn;
    char cmd[128];
    _sem_t sem;
    _lock_t lock;
}SEND_WAIT, *pSEND_WAIT;


typedef enum
{
    CMD_REBOOT,             //
    CMD_REBOOT_TIME,        //
    CMD_AUDIO_PALY,         //
    CMD_REPLY,              //
    CMD_CHECK_MC,           //
}enNET_CMD;


typedef enum
{
    SEND_NO_WAIT=0,         //
    SEND_AND_WAIT,          //
}enSEND_WAIT_FLAG;

typedef int (*pSendFun)(int sock, const char *ip, uint16 port, const char *data);

extern int PublicGroudSockFd;
extern LINK SendLink;

int InitNetWork(void);
int DelInitNetWork(void);

void SendAudioData(pPLAY_TASK_INFO pTask, pRTP_PACKET pRtpPacket, int size);
void SendAudioDataMC(pPLAY_TASK_INFO pTask, char *data, int size);
void SendAudioDataP2P(pPLAY_TASK_INFO pTask, char *data, int size);
int SendAudioCtrl(pPLAY_TASK_INFO pTask);
int SendMsg(int sock, const char *ip, uint16 port, const char *data);
int SendMsgWait(int sock, const char *ip, uint16 port, const char *data);
int SendToSvr(const char *data, enSEND_WAIT_FLAG WaitFlag);

#ifdef __cplusplus
}
#endif

#endif


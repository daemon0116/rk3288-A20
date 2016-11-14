/*****************************************************************

******************************************************************/

#ifndef __MAIN_H__
#define __MAIN_H__

#include "TypepDef.h"
#include "NetWork.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
    BOOL bRun;
    enPRIO_TYPE PrioType;
    int DevType;
    int SubType;
    int SN;
    int DevNum;
    int DevListIndex;
    char DevMask;
    char IP[16];
    char SvrIP[16];
    char PublicGroudIP[16];
    BOOL AddGroupFlag;
    UINT32 addr;
    UINT32 CheckMulticastCnt;
    int  DevNums;
    pIP_ADDR pDevIP;
    int LogFlag;
    char LogServerIP[16];
    unsigned short LogServerPort;
    uint16 RebootTime;
    uint8 PlayStatus;
    uint8 VoipStatus;
    char SipNum[24];

    pPLAY_TASK_INFO pRecordTask[PLAY_TASK_MAX];

    enSEND_MODE SendMode;
    #if (SEND_AUDIO_SOCK_NUM > 0)
    int SockFd[SEND_AUDIO_SOCK_NUM];
    #endif

    _lock_t lock;
}SYS_INFO, *pSYS_INFO;


extern pSYS_INFO pSysInfo;


#ifdef __cplusplus
}
#endif


#endif




#include "threadpool.h"
#include "OpenSlApi.h"
#include "log.h"

#include "NetWork.h"
#include "main.h"


pSYS_INFO pSysInfo = NULL;

int InitSysInfo(int DevNum)
{
    int i;
    
    if(!pSysInfo){
        pSysInfo = (pSYS_INFO)_malloc(sizeof(SYS_INFO));
        memset(pSysInfo, 0, sizeof(SYS_INFO));
    }
    _lock_init(&pSysInfo->lock, NULL);

    //test
    pSysInfo->CheckMulticastCnt = 0;
    pSysInfo->DevNums = DEF_DEVS_IP_NUM;
    pSysInfo->pDevIP = (pIP_ADDR)_malloc(pSysInfo->DevNums*sizeof(IP_ADDR));
    for(i=0; i<pSysInfo->DevNums; i++){
        snprintf(pSysInfo->pDevIP[i].ip, sizeof(pSysInfo->pDevIP[0].ip), "192.168.80.%d", i+100);
        pSysInfo->pDevIP[i].addr = inet_addr(pSysInfo->pDevIP[i].ip);
        pSysInfo->pDevIP[i].isMC = 0;
    }

    pSysInfo->DevType = DEV_HOST;
    pSysInfo->SubType = 1;
    pSysInfo->SN = 123;
    pSysInfo->DevNum = DevNum;
    pSysInfo->DevListIndex = pSysInfo->DevNum/7;
    pSysInfo->DevMask = (1<<(pSysInfo->DevNum%7));
    pSysInfo->PrioType = PRIO_BEHIND;
    memcpy(pSysInfo->SvrIP, "192.168.80.247", sizeof(pSysInfo->SvrIP));
    memcpy(pSysInfo->PublicGroudIP, "225.0.0.123", sizeof(pSysInfo->PublicGroudIP));
    memcpy(pSysInfo->IP, pSysInfo->pDevIP[pSysInfo->DevNum].ip, sizeof(pSysInfo->IP));
    pSysInfo->addr = inet_addr(pSysInfo->IP);

    pSysInfo->SendMode = SEND_MODE_P2P;

    pSysInfo->LogFlag = 0;
    #if 0
    memcpy(pSysInfo->LogServerIP, "192.168.80.28", sizeof(pSysInfo->LogServerIP));
    pSysInfo->LogServerPort = 514;
    pSysInfo->LogFlag = 1;
    #endif

    return 0;
}

int DelInitSysInfo(void)
{
    if(pSysInfo){
        if(pSysInfo->pDevIP){
            _free(pSysInfo->pDevIP);
        }
        _lock_destroy(&pSysInfo->lock);
        _free(pSysInfo);
        pSysInfo = NULL;
    }
}

int InitApp(int DevNum)
{
    
    InitSysInfo(DevNum);
    pSysInfo->bRun = TRUE;
    InitLog(pSysInfo->LogServerIP, pSysInfo->LogServerPort, pSysInfo->LogFlag);
    InitThrdPl();
    InitOpenSL();
    InitPlayTask();
    InitSip();
    InitNetWork();
    
    return 0;
}

int DelInitApp(int mask)
{
    pSysInfo->bRun = FALSE;

    DelInitNetWork();
    DelInitPlayTask();
    DelInitSip();
    DelInitOpenSL(); 
    DelInitThrdPl();
    DelInitLog();
    DelInitSysInfo();

    #ifdef MOMMON_TEST
    common_test_printf();
    #endif
    
    return 0;
}



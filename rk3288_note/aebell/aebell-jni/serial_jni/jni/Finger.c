#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#include "Command.h"
#include "Finger.h"


//--------------------------------------------------
//Add 4 test
void GenChk(char cmd[])
{
    int i;
    unsigned int chk = 0;
    for (i = 0; i < 22; i++)
    {
        chk += cmd[i];
    }
    cmd[22] = chk & 0xFF;
    cmd[23] = (chk >> 8) & 0xFF;
}

/******************************
 提供以下接口:
 0.打开
 1.写入特征码[一个大概1秒钟]
 2.验证[时间比较长]
 3.删除一个特征码
 4.清空所有特征码
 5.关闭
 ********************************/

CFinger g_newFinger = {0, 0};


BOOL FingerOpen(char *filename, int BaudRate, enDataBits eDataBits, enStopBits eStopBits, enParityCheck eParityCheck)//打开指纹仪
{
    int fd;
    
    fd = SerialOpen(filename, BaudRate, eDataBits, eStopBits, eParityCheck);
    if(fd > 0){
        g_newFinger.m_nHandle = fd;
        return 1;
    }
    return 0;
}

BOOL FingerClose(void)//关闭指纹仪
{
    BOOL bRet = FALSE;
    if(g_newFinger.m_nHandle)
    {
        bRet = close(g_newFinger.m_nHandle);
        g_newFinger.m_nHandle = 0;
    }
    return bRet;
}


BOOL FingerWrite(int nTempleId, BYTE* pFingerData, int nFingerDataSize)//写入特征码
{
    BOOL bRet = FALSE;

    if (g_newFinger.m_nHandle)
    {
        //发送命令并接收命令ack
        BOOL w_blRet = FALSE;
        //. Assemble command packet
        InitPacket(CMD_WRITE_TEMPLATE_CODE, TRUE);
        COMMAND_DATALEN = 2;
        COMMAND_DATA1 = GD_RECORD_SIZE;
        AddCheckSum(TRUE);
        //. Send command packet to target
        w_blRet = SendCommand(g_newFinger.m_nHandle, CMD_WRITE_TEMPLATE_CODE);
        if (w_blRet == FALSE)
        {
            return bRet;
        }
        if (RESPONSE_RET != ERR_SUCCESS)
        {
            return bRet;
        }
        //发送指纹数据并接收返回值
        //. Assemble data packet
        InitPacket(CMD_WRITE_TEMPLATE_CODE, FALSE);
        COMMAND_DATALEN = GD_RECORD_SIZE + 2;
        *(WORD*) &g_pPacketBuffer->tbCMDData = nTempleId;
        memcpy(g_pPacketBuffer->tbCMDData + 2, pFingerData, GD_RECORD_SIZE);
        AddCheckSum(FALSE);
        //. Send data packet to target
        w_blRet = SendDataPacket(g_newFinger.m_nHandle, CMD_WRITE_TEMPLATE_CODE);
        if (w_blRet == FALSE)
        {
            return bRet;
        }
        if (RESPONSE_RET != ERR_SUCCESS)
        {
            return bRet;
        }
        bRet = TRUE;
    }
    return bRet;
}

int FingerAdd(int nId, BYTE* pData)
{
    int bRet;

    if(!g_newFinger.m_nHandle){
        return 0;
    }

    printf("@@@@@@@@@@@@@@@@@@@@CALL FUNC finger_add()@@@@@@@@@@@@@@@\n");
    bRet = FingerWrite(nId, pData, 0);
    return bRet;
}


BOOL FingerDelete(int nTempleId)//删除模板
{
    BOOL bRet = FALSE;

    if (g_newFinger.m_nHandle)
    {
        InitPacket(CMD_CLEAR_TEMPLATE_CODE, TRUE);
        COMMAND_DATALEN = 2;
        COMMAND_DATA1 = nTempleId;
        AddCheckSum(TRUE);
        WORD m_dwCode = CMD_CLEAR_TEMPLATE_CODE;
        BOOL w_blRet = FALSE;
        //  LONG        w_nLastError = ERROR_SUCCESS;
        if (g_pPacketBuffer->wPrefix == CMD_PREFIX_CODE)
        {
            w_blRet = SendCommand(g_newFinger.m_nHandle, m_dwCode);
        }
        else if (g_pPacketBuffer->wPrefix == CMD_DATA_PREFIX_CODE)
        {
            w_blRet = SendDataPacket(g_newFinger.m_nHandle, m_dwCode);
        }
        else
        {
            w_blRet = ReceiveAck(g_newFinger.m_nHandle, m_dwCode, TRUE);
        }
        if (w_blRet)
        {
            if (RESPONSE_RET == ERR_SUCCESS)
            {
                bRet = TRUE;
            }
            else
            {
                if (RESPONSE_DATA1 == ERR_TMPL_EMPTY)
                {
                    bRet = TRUE;
                }
            }
        }
    }
    return bRet;
}


BOOL FingerClean(void)//清空所有模板
{
    if (g_newFinger.m_nHandle)
    {
        BOOL w_blRet = FALSE;
        InitPacket(CMD_CLEAR_ALLTEMPLATE_CODE, TRUE);
        COMMAND_DATALEN = 0;
        AddCheckSum(TRUE);
        w_blRet = SendCommand(g_newFinger.m_nHandle, CMD_CLEAR_ALLTEMPLATE_CODE);
        if (w_blRet == FALSE)
        {
            return FALSE;
        }
        if (RESPONSE_RET == ERR_SUCCESS)
        {
            return TRUE;
        }
    }
    return FALSE;
}


//-----------------------------------------------------

BOOL FingerCancel(void)//取消操作
{
    BOOL bReat = FALSE;
    
    if (g_newFinger.m_nHandle)
    {
        InitPacket(CMD_FP_CANCEL_CODE, TRUE);
        COMMAND_DATALEN = 0;
        AddCheckSum(TRUE);
        bReat = SendCommand(g_newFinger.m_nHandle, CMD_FP_CANCEL_CODE);
    }
    return bReat;
}

BOOL FingerCheck(int* nTempleId)//验证
{
    BOOL bRet = FALSE;

    if (g_newFinger.m_nHandle)
    {
        WORD m_dwCode;
        InitPacket(CMD_IDENTIFY_CODE, TRUE);
        AddCheckSum(TRUE);
        BOOL w_blRet = FALSE;
        //  LONG        w_nLastError = ERROR_SUCCESS;
        m_dwCode = CMD_IDENTIFY_CODE;
        //need replace by new api
        //Utile_SetTip(TIP_PUT_FINGER, TIP_SHOW_TYPE_ALWAYS, NULL);
        w_blRet = SendCommand(g_newFinger.m_nHandle, CMD_IDENTIFY_CODE);
        if (w_blRet == FALSE)
        {
            return bRet;
        }
        if (RESPONSE_RET != ERR_SUCCESS)
        {
            return bRet;
        }
        if (ReceiveAck(g_newFinger.m_nHandle, CMD_IDENTIFY_CODE, TRUE))
        {
            if (RESPONSE_RET == ERR_SUCCESS)
            {
                *nTempleId = RESPONSE_DATA1;
                bRet = TRUE;
            }
        }
    }
    return bRet;
}

BOOL FingerSetTimeout(int timeoutVal)
{
    BOOL bReat = FALSE;

    BOOL w_blRet = FALSE;
    if (g_newFinger.m_nHandle)
    {
        InitPacket(CMD_SET_FP_TIMEOUT_CODE, TRUE);
        COMMAND_DATALEN = 2;
        COMMAND_DATA1 = timeoutVal;
        AddCheckSum(TRUE);
        w_blRet = SendCommand(g_newFinger.m_nHandle, CMD_CLEAR_ALLTEMPLATE_CODE);
        if (w_blRet == FALSE)
        {
            return FALSE;
        }
        if (RESPONSE_RET == ERR_SUCCESS)
        {
            return TRUE;
        }
    }
    return bReat;
}



//---------------------------------------------

int FingerDo(FINGERTASK* pTask)
{
    int bRet = 0;

    if(!g_newFinger.m_nHandle){
        return bRet;
    }
    
    switch (pTask->type)
    {
    case emFingerAdd:
    {
        printf("@@@@@@@@@@@@@@@@@@@@emFingerAdd@@@@@@@@@@@@@@@\n");
        bRet = FingerAdd(pTask->nId, pTask->pData);
    }
    break;
    case emFingerClear:
    {
        printf("####################emFingerClear@@@@@@@@@@@@@@@\n");
        bRet = FingerClean();
    }
    break;
    case emFingerDel:
    {
        printf("????????????????????emFingerDel@@@@@@@@@@@@@@@\n");
        bRet = FingerDelete(pTask->nId);
    }
    break;
    case emFingerCheck:
    {
        printf("%%%%%%%%%%%%%%%%%%%%emFingerCheck@@@@@@@@@@@@@@@\n");
        bRet = FingerCheck(&pTask->nId);
    }
    break;
    case emFingerSetTimeout:
    {
        printf("%%%%%%%%%%%%%%%%%%%%emFingerSetTimeout@@@@@@@@@@@@@@@\n");
        bRet = FingerSetTimeout((int)(*pTask->pData));
    }
    break;
    }
    
    return bRet;
}

#if 0
int main(int argc, char **argv)
{
    char buf[128];
    FINGERTASK *pinfo = (FINGERTASK *)buf;
        
    if(!FingerOpen("/dev/ttyAMA2", BR_9600, DBIT_8, SBIT_1, PCHK_NONE))
    {
        while(1)
        {
            printf("%%%%%%%%%%%%%%%%%%%%打开串口失败@@@@@@@@@@@@@@@\n");
        }
    }

    int timeout = 5;
    pinfo->type = emFingerSetTimeout;
    pinfo->pData = (BYTE *)&timeout;
    FingerDo(pinfo);
    
    while(1)
    {
        pinfo->type = emFingerCheck;
        pinfo->nId = 1;
        FingerDo(pinfo);

        sleep(2);
    }

    FingerClose();
}
#endif

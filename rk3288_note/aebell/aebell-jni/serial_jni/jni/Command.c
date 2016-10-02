/********************************************************************
 purpose:
 *********************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
//#include "hi_init.h"
//#include "sentry.h"
//#include "tran.h"
//#include "net_protocol.h"
//#include "hi_type.h"
#include "Command.h"
//CSerial           g_Serial;
BYTE g_Packet[MAX_DATA_LEN + 10];
PST_COM_PACKET g_pPacketBuffer = (PST_COM_PACKET) g_Packet;
ST_COMMAND g_Commands[] =
{
    { "Verify", CMD_VERIFY_CODE },
    { "Identify", CMD_IDENTIFY_CODE },
    { "Enroll", CMD_ENROLL_CODE },
    { "Enroll One Time", CMD_ENROLL_ONETIME_CODE },
    { "Clear Template", CMD_CLEAR_TEMPLATE_CODE },
    { "Clear All Template", CMD_CLEAR_ALLTEMPLATE_CODE },
    { "Get Empty ID", CMD_GET_EMPTY_ID_CODE },
    { "Get Template Status", CMD_GET_TEMPLATE_STATUS_CODE },
    { "Get Broken Template", CMD_GET_BROKEN_TEMPLATE_CODE },
    { "Read Template", CMD_READ_TEMPLATE_CODE },
    { "Write Template", CMD_WRITE_TEMPLATE_CODE },
    { "Set Security Level", CMD_SET_SECURITYLEVEL_CODE },
    { "Get Security Level", CMD_GET_SECURITYLEVEL_CODE },
    { "Set Finger Time Out", CMD_SET_FP_TIMEOUT_CODE },
    { "Get Finger Time Out", CMD_GET_FP_TIMEOUT_CODE },
    { "Set Device ID", CMD_SET_DEV_ID_CODE },
    { "Get Device ID", CMD_GET_DEV_ID_CODE },
    { "Get F/W Version", CMD_GET_FW_VERSION_CODE },
    { "Finger Detect", CMD_FINGER_DETECT_CODE },
    { "Set BaudRate", CMD_SET_BAUDRATE_CODE },
    { "Set Duplication Check", CMD_SET_DUP_CHECK_CODE },
    { "Get Duplication Check", CMD_GET_DUP_CHECK_CODE },
    { "Entering Standby Mode", CMD_ENTER_STANDBY_MODE_CODE },
    { "Enroll And Store in RAM", CMD_ENROLL_AND_STORE_CODE },
    { "Get Enroll Data", CMD_GET_ENROLL_DATA_CODE },
    { "Get Feature Data of Captured FP", CMD_ENROLL_AND_READ_CODE },
    { "Verify Downloaded Feature with Captured FP", CMD_VERIFY_WITH_TMPL_CODE },
    { "Identify Downloaded Feature with Captured FP", CMD_IDENTIFY_WITH_TMPL_CODE },
    { "Get Device Name", CMD_GET_DEV_NAME_CODE },
//  {CMD_FP_CANCEL,                 "Fp Cancel",                        CMD_FP_CANCEL_CODE},
    { 0, 0 }
};
/***************************************************************************/
WORD GetCheckSum(int bCmdData)
{
    WORD w_Ret = 0;
    WORD i;
    for (i = 0; i < (bCmdData ? CMD_PACKET_LEN : DATA_PACKET_LEN); i++)
    {
        w_Ret += (WORD) g_Packet[i];
        //sprintf(sz,"%02x",(WORD)g_Packet[i]);
        //str+=sz;
    }
    //AfxMessageBox(str);
    return w_Ret;
}
int ClearAllFingerComData(int fd)
{
    char tmp[256];
    int maxfd = fd;
    int s32ret = -1, nRes;
    struct timeval TimeoutVal;
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(fd,&read_fds);
    while (1)
    {
        TimeoutVal.tv_sec = 0;
        TimeoutVal.tv_usec = 10;
        s32ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32ret <= 0)
        {
            return 0;
        }
        if (FD_ISSET(fd, &read_fds))
        {
            nRes = read(fd, tmp, sizeof(tmp));
            if (nRes > 0)
            {
                continue;
            }
            else
            {
                return 0;
            }
        }
    }
    return -1;
}
BOOL RecvFinderData(int fd, char* recvBuf, int readLen)
{
    int count, nRes;
    count = 0;
    nRes = 0;
    int maxfd = fd;
    int s32ret = -1;
    struct timeval TimeoutVal;
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(fd,&read_fds);
    while (1)
    {
        if (count >= readLen)
        {
            break;
        }
        TimeoutVal.tv_sec = FINGER_UART_READ_TIMEOUT;
        TimeoutVal.tv_usec = 0;
        s32ret = select(maxfd + 1, &read_fds, NULL, NULL, &TimeoutVal);
        if (s32ret < 0)
        {
            return FALSE;
        }
        else if (s32ret == 0)
        {
            return FALSE;
        }
        if (FD_ISSET(fd, &read_fds))
        {
            nRes = read(fd, recvBuf + count, readLen - count);
            //printf("--- RecvFinderData, read, nRes = %d \n", nRes);
            if (nRes > 0)
            {
                count += nRes;
            }
            else if (nRes < 0)
            {
                return FALSE;
            }
            else
            {
                return TRUE;
            }
        }
    }
    return TRUE;
}
/***************************************************************************/
WORD AddCheckSum(int bCmdData)
{
    WORD w_Ret = 0;
    WORD w_Len = bCmdData ? CMD_PACKET_LEN : DATA_PACKET_LEN;
    int i;
    for (i = 0; i < w_Len; i++)
    {
        w_Ret += (WORD) g_Packet[i];
    }
    *(WORD*) &g_Packet[w_Len] = w_Ret;
    return w_Ret;
}
/***************************************************************************/
BOOL CheckReceive(WORD p_Prefix, WORD p_CMD)
{
    WORD w_wCheckSum, w_Len;
    if (p_Prefix != g_pPacketBuffer->wPrefix || p_CMD != g_pPacketBuffer->wCMD_RCM)
    {
        return FALSE;
    }
    w_Len = (p_Prefix == RCM_PREFIX_CODE) ? CMD_PACKET_LEN : DATA_PACKET_LEN;
    w_wCheckSum = *(WORD*) &g_Packet[w_Len];
    if (w_wCheckSum != GetCheckSum(p_Prefix == RCM_PREFIX_CODE))
    {
        return FALSE;
    }
    return TRUE;
}
/***************************************************************************/
void InitPacket(WORD p_wCMD, BOOL p_bCmdData)
{
    memset(g_Packet, 0, sizeof(g_Packet));
    g_pPacketBuffer->wPrefix = p_bCmdData ? CMD_PREFIX_CODE : CMD_DATA_PREFIX_CODE;
    g_pPacketBuffer->wCMD_RCM = p_wCMD;
    return;
}
/***************************************************************************/
BOOL SendCommand(int nFile, WORD p_wCMD)
{
    printf("CALL FUNC SendCommand() \n");
    if (write(nFile, g_Packet, CMD_PACKET_LEN + 2) == CMD_PACKET_LEN + 2)
    {
        printf("CALL FUNC SendCommand() OK\n");
        return ReceiveAck(nFile, p_wCMD, TRUE);
    }
    return FALSE;
}
/***************************************************************************/
BOOL ReceiveAck(int nFile, WORD p_wCMD, BOOL p_bCmdData)
{
    int readLen = p_bCmdData ? (CMD_PACKET_LEN + 2) : 12;
    if (!RecvFinderData(nFile, g_Packet, readLen))
    {
        printf("ReceiveAck --> RecvFinderData Read ERR\n");
        return FALSE;
    }
    return CheckReceive(p_bCmdData ? RCM_PREFIX_CODE : RCM_DATA_PREFIX_CODE, p_wCMD);
}
/***************************************************************************/
BOOL SendData(int nFile, WORD p_wCMD, int p_nDataLen, PBYTE p_pData)
{
    int w_nSendedCnt;
    int w_wPacketDataLen = 0;
    LONG w_nResult = 0;
    for (w_nSendedCnt = 0; w_nSendedCnt < p_nDataLen; w_nSendedCnt += w_wPacketDataLen)
    {
        w_wPacketDataLen = p_nDataLen - w_nSendedCnt;
        if (w_wPacketDataLen > MAX_DATA_LEN)
        {
            w_wPacketDataLen = MAX_DATA_LEN;
        }
        InitPacket(p_wCMD, FALSE);
        g_pPacketBuffer->wDataLen = (WORD) w_wPacketDataLen;
        memcpy(g_pPacketBuffer->tbCMDData, p_pData + w_nSendedCnt, w_wPacketDataLen);
        AddCheckSum(FALSE);
        if (SendDataPacket(nFile, p_wCMD) == FALSE)
        {
            return FALSE;
        }
    }
    return TRUE;
}
/***************************************************************************/
BOOL SendDataPacket(int nFile, WORD p_wCMD)
{
    if (write(nFile, g_Packet, DATA_PACKET_LEN + 2) != DATA_PACKET_LEN + 2)
    {
        return FALSE;
    }
    return ReceiveAck(nFile, p_wCMD, FALSE);
}
/***************************************************************************/
BOOL ReceiveData(int nFile, WORD p_wCMD, int p_nDataLen, PBYTE p_pData)
{
    int w_nReceivedCnt;
    int w_wPacketDataLen = 0;
    LONG w_nResult = 0;
    for (w_nReceivedCnt = 0; w_nReceivedCnt < p_nDataLen; w_nReceivedCnt += w_wPacketDataLen)
    {
        w_wPacketDataLen = p_nDataLen - w_nReceivedCnt;
        if (w_wPacketDataLen > MAX_DATA_LEN)
        {
            w_wPacketDataLen = MAX_DATA_LEN;
        }
        if (ReceiveDataPaket(nFile, p_wCMD, w_wPacketDataLen) == FALSE)
        {
            return FALSE;
        }
        memcpy(p_pData + w_nReceivedCnt, g_pPacketBuffer->stRcmData.tbRcmData, DATA_PACKET_LEN - 2);
    }
    return TRUE;
}
/***************************************************************************/
BOOL ReceiveDataPaket(int nFile, WORD p_wCMD, WORD p_wDataLen)
{
    if (!RecvFinderData(nFile, g_Packet, p_wDataLen + 10))
    {
        return FALSE;
    }
    return CheckReceive(RCM_DATA_PREFIX_CODE, p_wCMD);
}
/***************************************************************************/
char* GetErrorMsg(WORD p_wErrorCode)
{
    char* w_ErrMsg;
    unsigned char errorCode = p_wErrorCode & 0xFF;
    switch (errorCode)
    {
    case ERR_VERIFY:
        w_ErrMsg = "Verify NG";
        break;
    case ERR_IDENTIFY:
        w_ErrMsg = "Identify NG";
        break;
    case ERR_EMPTY_ID_NOEXIST:
        w_ErrMsg = "Empty Template no Exist";
        break;
    case ERR_BROKEN_ID_NOEXIST:
        w_ErrMsg = "Broken Template no Exist";
        break;
    case ERR_TMPL_NOT_EMPTY:
        w_ErrMsg = "Template of this ID Already Exist";
        break;
    case ERR_TMPL_EMPTY:
        w_ErrMsg = "This Template is Already Empty";
        break;
    case ERR_INVALID_TMPL_NO:
        w_ErrMsg = "Invalid Template No";
        break;
    case ERR_ALL_TMPL_EMPTY:
        w_ErrMsg = "All Templates are Empty";
        break;
    case ERR_INVALID_TMPL_DATA:
        w_ErrMsg = "Invalid Template Data";
        break;
    case ERR_INVALID_SEC_VAL:
        w_ErrMsg = "Invalid Security Level";
        break;
    case ERR_INVALID_TIME_OUT:
        w_ErrMsg = "Invalid Time Out Value";
        break;
    case ERR_DUPLICATION_ID:
        //      w_ErrMsg.Format("Duplicated ID : %d.", HIBYTE(p_wErrorCode));
        w_ErrMsg = "Duplicated ID : ";
        break;
    case ERR_BAD_QUALITY:
        w_ErrMsg = "Bad Quality Image";
        break;
    case ERR_TIME_OUT:
        w_ErrMsg = "Time Out";
        break;
    case ERR_GENERALIZE:
        w_ErrMsg = "Fail to Generalize";
        break;
    case ERR_DEVICE_ID_EMPTY:
        w_ErrMsg = "Device ID yet not set";
        break;
    case ERR_EXCEPTION:
        w_ErrMsg = "Exception Error ";
        break;
    case ERR_MEMORY:
        w_ErrMsg = "Memory Error ";
        break;
    case ERR_INVALID_DUP_VAL:
        w_ErrMsg = "Invalid Duplication Check Option";
        break;
    case ERR_INVALID_PARAM:
        w_ErrMsg = "Invalid Parameter";
        break;
    case ERR_INTERNAL:
        w_ErrMsg = "Internal Error.";
        break;
    default:
        w_ErrMsg = "Fail";
        break;
    }
    return w_ErrMsg;
}

/*****************************************************************

******************************************************************/

#include <fcntl.h>
#include <sys/socket.h>

#include "log.h"

#include "UdpApp.h"
#include "ringbuffer.h"
#include "threadpool.h"
#include "AudioPlayer.h"

#include "main.h"
#include "sip.h"
#include "linkbuffer.h"
#include "NetWork.h"
#include "common.h"


int PublicGroudSockFd = -1;
int AudioCtrlSockFd = -1;
int AudioDataSockFd = -1;
BOOL isOnline = FALSE;
BOOL KeepAliveFlag = FALSE;

SEND_WAIT SendWait;

LINK SendLink;
LINK AudioPlayCmdLink;

//指向网络播放的任务节点，初始时指向PlayTask[0]
//避免pNetTask指向非法值执行时产生段错误，当有
//网络任务时申请一个任务节点，pNetTask指向该节点
//该节点将不会释放，一直给网络任务用。
pPLAY_TASK_INFO pNetTask = &PlayTask[0];


#if 1 //指令处理

//发出的指令数据格式
const char Register_cmd[]  = "{\"Cmd\":\"Register\",\"DevType\":\"%d\",\"SubType\":\"%d\",\"ID\":\"%d\",\"SrcNum\":\"%d\",\"IP\":\"%s\",\"Status\":\"\",\"Session\":\"\"}";
const char Online_cmd[]    = "{\"Cmd\":\"Online\",\"SrcNum\":\"%d\",\"SipNum\":\"%s\",\"Call\":\"%d\",\"Meeting\":\"%d\",\"Monitor\":\"%d\",\"BcFile\":\"%d\",\"BcMic\":\"%d\",\"BcAudio\":\"%d\",\"BcRecv\":\"%d\",\"Session\":\"0\"}";
const char AudioPlay_cmd[] = "{\"Cmd\":\"AudioPlay\",\"SrcType\":\"%d\",\"SrcNum\":\"%d\",\"PlayID\":\"%d\",\"PlayNum\":\"%d\",\"StartTime\":\"%d\",\"Priority\":\"%d\",\"Control\":\"%d\",\"Volume\":\"%d\",\"Format\":\"%d\",\"SampleRate\":\"%d\",\"Channels\":\"%d\",\"BitsPerSample\":\"%d\",\"FileName\":\"%s\",\"AddGroud\":\"%s\",\"DevList\":\"%s\",\"Session\":\"\"}";
const char Reply_cmd[]     = "{\"Cmd\":\"Reply\",\"RepCmd\":\"%s\",\"SrcNum\":\"%d\",\"DestNum\":\"%d\",%s\"Status\":\"%d\",\"Session\":\"%s\"}";

const char CheckMulticast_cmd[] = "{\"Cmd\":\"CheckMulticast\",\"SrcNum\":\"%d\",\"Addr\":\"%u\",\"Type\":\"%d\",\"Session:\"1\"}";

//指令字符
const char cmd_Register[]    = "Register";
const char cmd_Online[]      = "Online";
const char cmd_Trusteeship[] = "Trusteeship";
const char cmd_Output[]      = "Output";
const char cmd_OpenDoor[]    = "OpenDoor";
const char cmd_Reboot[]      = "Reboot";
const char cmd_RebootTime[]  = "RebootTime";
const char cmd_AudioPlay[]   = "AudioPlay";
const char cmd_Reply[]       = "Reply";
const char cmd_CheckMulticast[] = "CheckMulticast";


//指令数据中的关键字
const char key_Cmd[] =     "Cmd";
const char key_SrcType[] = "SrcType";
const char key_SrcNum[] = "SrcNum";
const char key_PlayID[] = "PlayID";
const char key_PlayNum[] = "PlayNum";
const char key_StartTime[] = "StartTime";
const char key_Priority[] = "Priority";
const char key_Control[] = "Control";
const char key_Format[] = "Format";
const char key_Volume[] = "Volume";
const char key_SampleRate[] = "SampleRate";
const char key_Channels[] = "Channels";
const char key_BitsPerSample[] = "BitsPerSample";
const char key_FileName[] = "FileName";
const char key_AddGroud[] = "AddGroud";
const char key_DevList[] = "DevList";
const char key_Addr[] = "Addr";
const char key_Type[] = "Type";

const char key_DestNum[] = "DestNum";
const char key_Channel[] = "Channel";
const char key_Time[] = "Time";
const char key_RepCmd[] = "RepCmd";
const char key_Session[] = "Session";

void ReplyMsg(pNET_RECV pNetRecv, const char *pbody, const char *other);
void SendCheckMulticast(int type);

int HandleCmdTrusteeship(const char *pdata)
{
    NetCmdCallJava(pdata);
}

int HandleCmdOutput(const char *pdata)
{
    NetCmdCallJava(pdata);
}

int HandleCmdReboot(pNET_RECV pNetRecv, const char *pdata)
{
    ReplyMsg(pNetRecv, pdata, NULL);
    
    system("reboot");
}

int HandleCmdRebootTime(pNET_RECV pNetRecv, const char *pdata)
{
    #if 1
    char *p;
    char time[6];//"00:00"

    ReplyMsg(pNetRecv, pdata, NULL);

    if(!get_value_char(key_Time, pdata, time, sizeof(time))){
        return -1;
    }
    if(!(p = strstr(time, ":"))){
        return -1;
    }
    *p = 0;
    
    pSysInfo->RebootTime = (atoi(time)<<8)|atoi(p+1);
    #else
    int h,m;
    
    sscanf(pdata, "\"Time\":\"%u:%u\"",&h, &m);
    pSysInfo->RebootTime = (h<<8) | m;

    LOGD("HandleCmdRebootTime h=%d, m=%d", h, m);

    #endif
}

/*******************************************************************************
标题：HandleCmdAudioPlay
功能：处理来自网络的播放控制指令"AudioPlay"
输入：pdata: 播放控制参数指针
输出：
返回值: 0: 有效处理，其他: 无效处理
*******************************************************************************/
int HandleCmdAudioPlay2(const char *pdata)
{
    int NewFlag = 0; //
    PLAY_TASK_INFO Task;

    if(!pdata){
        return -1;
    }

    if(GetNetPlayInfo(NULL, &Task, pdata, 1) < 0){
        return -1;
    }

    if(Task.PlayCtrl==pNetTask->PlayCtrl && Task.SrcDevNum==pNetTask->SrcDevNum && \
        Task.SrcPlayID==pNetTask->SrcPlayID && Task.SrcPlayNum==pNetTask->SrcPlayNum && \
        Task.volume==pNetTask->volume){
        pNetTask->LastTime = time(NULL);
        return -1;
    }

    LOGV("have new");

    if(Task.PlayCtrl == PLAY){
        if(pNetTask->PlayCtrl == PLAY){
            if(pNetTask->SrcDevNum == Task.SrcDevNum && pNetTask->SrcPlayID == Task.SrcPlayID){
                if(pNetTask->SrcPlayNum == Task.SrcPlayNum){
                    //同一源设备同一播放任务同一曲目，音量变化
                    if(0==HandSetVol(pNetTask->TaskIndex, Task.volume) && pNetTask->isLocalPlay){
                        PlayLocalHaveNew(CH_VOLUME);
                    }
                    return 0;
                }
                //同一源设备同一播放任务曲目不同
            }
            else{
                //源设备不同或//同一源设备播放任务不同
                NewFlag = 2;
            }
        }
        else{
            //本机当前没有网络任务
            NewFlag = 1;
        }
    }
    else if(Task.SrcDevNum==pNetTask->SrcDevNum && Task.SrcPlayID==pNetTask->SrcPlayID){
        HandPlayTaskStatus(pNetTask->TaskIndex, STOP);
        PlayCallJava(pNetTask->TaskIndex, STOP, " ");
        //#if USE_MULTICAST_AUDIO_DATA
        pNetTask->GroupIP.ip[0]= 0;
        pNetTask->GroupFlag = TRUE; //让音频接收线程退出组播组
        UDP_SendSelf(PublicGroudSockFd, AUDIO_DATA_PORT);
        //#endif

        LOGD("STOP");
        return 0;
    }
    else{
        return -1;
    }

    if(GetNetPlayInfo(NULL, &Task, pdata, 2) < 0){
        return -1;
    }

    if(!pNetTask->isUse){
        int TaskIndex = AllocPlayTask(PLAY_NET," ", NULL, 0, 1, Task.volume);
        if(!isVALID_TASK_ID(TaskIndex)){
            return -1;
        }
        pNetTask = &PlayTask[TaskIndex];
        LOGV("NEW");
    }
    else if(NewFlag == 2){
        if(!PriorityCmp(pNetTask, &Task) && time(NULL)<pNetTask->LastTime+RECV_PLAY_CTRL_TIMEOUT){
            //新的任务比当前任务权限低&&当前任务接收控制指令没超时，则新任务不能抢占当前任务。
            return -1;
        }
        //抢占当前的网络任务，需将当前的释放
        HandPlayTaskStatus(pNetTask->TaskIndex, STOP);
        LOGV("pNetTask->StartTime=%d, Task.StartTime=%d", pNetTask->StartTime, Task.StartTime);
    }
    
    pNetTask->LastTime = time(NULL);

    _lock(&PlayTaskMutex);
    pNetTask->format = 0;
    GetNetPlayInfo(pNetTask, &Task, pdata, 3);
    //#if USE_MULTICAST_AUDIO_DATA
    if(strcmp(pNetTask->GroupIP.ip, Task.GroupIP.ip)){
        memcpy(pNetTask->GroupIP.ip, Task.GroupIP.ip, sizeof(pNetTask->GroupIP.ip));
        pNetTask->GroupFlag = TRUE;
        //触发音频接收线程退出阻塞，加速响应
        UDP_SendSelf(PublicGroudSockFd, AUDIO_DATA_PORT);
    }
    //#endif
    _unlock(&PlayTaskMutex);
    
    #if 0
    LOGV("StartTime=%d", pNetTask->StartTime);
    LOGV("SrcDevType=%d", pNetTask->SrcDevType);
    LOGV("SrcDevNum=%d", pNetTask->SrcDevNum);
    LOGV("SrcPlayID=%d", pNetTask->SrcPlayID);
    LOGV("SrcPlayNum=%d", pNetTask->SrcPlayNum);
    LOGV("volume=%d", pNetTask->volume);
    LOGV("format=%d", pNetTask->format);
    LOGV("rate=%d", pNetTask->pcm.rate);
    LOGV("channel=%d", pNetTask->pcm.channels);
    LOGV("bitsPerSample=%d", pNetTask->pcm.bitsPerSample);
    LOGV("GroupIP=%s", pNetTask->GroupIP.ip);
    #endif

    if(NewFlag){
        HandPlayTaskStatus(pNetTask->TaskIndex, PLAY);
        LOGD("PLAY");
    }
    else if(pNetTask->isLocalPlay){
        PlayLocalHaveNew(CH_FILE);
        LOGD("CH_FILE");
    }
    NetBcCallJava(pNetTask->TaskIndex, pNetTask->SrcDevNum, NewFlag==1?PLAY:CURRENT, pNetTask->FileList);
    
    return 0;
}

int HandleCmdAudioPlay(pNET_RECV pNetRecv, const char *pdata)
{
    int  DestNum;

    DestNum = get_value_num(key_SrcNum, pdata);
    if(DestNum == 0){
        //认为是服务器下发的指令，需回复
        ReplyMsg(pNetRecv, pdata, NULL);
    }
    HandleCmdAudioPlay2(pdata);
    return 0;
}

int HandleCmdCheckMulticast(pNET_RECV pNetRecv, const char *pdata)
{
    int i;
    int type;
    int SrcNum;

    type = get_value_num(key_Type, pdata);
    SrcNum = get_value_num(key_SrcNum, pdata);

    if(type){
        SendCheckMulticast(0);
    }

    if(SrcNum > 0 && SrcNum < pSysInfo->DevNums){
        _lock(&pSysInfo->lock);
        pSysInfo->pDevIP[SrcNum].isMC = pSysInfo->CheckMulticastCnt;
        _unlock(&pSysInfo->lock);
        //LOGV("Multicast:%d", SrcNum);
    }
    
    return 0;
}

/*******************************************************************************
标题：HandleReply
功能：
输入：pdata: 
输出：
返回值:
*******************************************************************************/
int HandleCmdReply(pNET_RECV pNetRecv, const char *pdata)
{
    const char *pcmd;
    char cmd[128];

    pcmd = get_value_char2(key_RepCmd, pdata, cmd, sizeof(cmd));

    if(SendWait.flag == TRUE){
        if(!strcmp(SendWait.cmd, pcmd)){
            SendWait.flag = TRUE2;
            _sem_post(&SendWait.sem);
            return 0;
        }
    }
    
    if(!strcmp(cmd_Online, pcmd) || !strcmp(cmd_Register, pcmd)){
        LOGV("pcmd=%s",pcmd);
        KeepAliveFlag = TRUE;
        isOnline = TRUE;
    }
    else{
        LOGV("pcmd=%s",pcmd);
        NetCmdCallJava(pdata);
    }

    return 0;
}


const FUNC_ROUTER FuncRouter[] =
{
    //将调用越频繁的放在越靠前，会减少匹配指令带来的开销
    {CMD_REPLY,         cmd_Reply,          HandleCmdReply},
    {CMD_AUDIO_PALY,    cmd_AudioPlay,      HandleCmdAudioPlay},
	{CMD_CHECK_MC,      cmd_CheckMulticast, HandleCmdCheckMulticast},
    {CMD_REBOOT,        cmd_Reboot,         HandleCmdReboot},
    {CMD_REBOOT_TIME,   cmd_RebootTime,     HandleCmdRebootTime},
};

void ReplyMsg(pNET_RECV pNetRecv, const char *pbody, const char *other)
{
    int len;
    char buf[sizeof(Reply_cmd)+256];
    char RepCmd[64];
    int  DestNum;
    char Session[64];
    char SendBuf[1500];

    DestNum = get_value_num(key_SrcNum, pbody);
    get_value_char(key_Cmd, pbody, RepCmd,  sizeof(RepCmd));
    get_value_char(key_Session, pbody, Session, sizeof(Session));
    
    snprintf(buf, sizeof(buf), Reply_cmd, RepCmd, pSysInfo->DevNum, DestNum, other?other:"", 0, Session);
    LOGV("ReplyMsg--->ip=%s,%s", pNetRecv->ip, buf);
    len = sip_make_ok(pNetRecv->pdata, buf, SendBuf, sizeof(SendBuf));
    UDP_SendData(PublicGroudSockFd, pNetRecv->ip, pNetRecv->port, SendBuf, len);
}

int HandleMsg(pNET_RECV pNetRecv)
{
    int i;
    const char *pbody;
    const char *pcmd;
    char buf[1024];

    //找出消息体
    if((pbody = sip_find_msg_body(pNetRecv->pdata))){
    }
    else if(!(pbody = sip_find_ok_body(pNetRecv->pdata))){
        return -1;
    }

    #if 0
    //判断设备列表中是否包含本设备
    if(get_value_char(key_DevList, pbody, buf, sizeof(buf))){//掩码方式
        if(strcmp(buf, "") && !isHaveSelf(buf)){
            return -1;
        }
    }
    else if(!isSelf(get_value_num(key_DestNum, pbody))){//编号方式
        return -1;
    }
    #endif

    //取出指令值
    pcmd = get_value_char2(key_Cmd, pbody, buf, sizeof(buf));

    //查找是否jni层指令，是的话在jni层执行
    for(i=0; i<sizeof(FuncRouter)/sizeof(FuncRouter[0]); i++)
    {
        if(!strcmp(pcmd, FuncRouter[i].CmdStr)){
            if(FuncRouter[i].func){
                FuncRouter[i].func(pNetRecv, pbody);
            }
            return 0;
        }
    }

    //不是jni层的消息，回复后上传给ui
    ReplyMsg(pNetRecv, pbody, NULL);
    NetCmdCallJava(pbody);

    return 0;
}

#endif



/*******************************************************************************
标题：
功能：
输入：
输出：
返回值:
*******************************************************************************/
int SendMsg(int sock, const char *ip, uint16 port, const char *data)
{
    int len;
    char SendBuf[1500];
    
    len = sip_make_msg(ip, port, data, SendBuf, sizeof(SendBuf), NULL);
    UDP_SendData(sock, ip, port, SendBuf, len);

    return len;
}

/*******************************************************************************
标题：SendMstWait
功能：发送数据并等待回复，如没有回复共尝试发送3次，间隔0.5秒
输入：
输出：
返回值:
注意: 发送时没有回复函数会最长阻塞1.5秒
*******************************************************************************/
int SendMsgWait(int sock, const char *ip, uint16 port, const char *data)
{
    int i;
    int len;
    char SendBuf[1500];
    struct timespec ts;

    if(!isOnline){
        return -1;
    }

    _lock(&SendWait.lock);
    SendWait.flag = TRUE;
    len = sip_make_msg(ip, port, data, SendBuf, sizeof(SendBuf), &SendWait.sn);
    get_value_char(key_Cmd, data, SendWait.cmd, sizeof(SendWait.cmd));//???
    for(i=0; i<3; i++)
    {
        UDP_SendData(sock, ip, port, SendBuf, len);

        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += 500000000;//等待间隔0.5s
        ts.tv_sec += ts.tv_nsec/1000000000;
        ts.tv_nsec = ts.tv_nsec%1000000000;
        sem_timedwait(&SendWait.sem, &ts);
        if(SendWait.flag == TRUE2){
            goto out;
        }
    }
    len = -1;

    out:
    SendWait.flag = FALSE;
    SendWait.cmd[0] = 0;
    _unlock(&SendWait.lock);

    return len;    
}


/*******************************************************************************
标题：SendMsgMulti
功能：根据发送数据中的目标设备列表或编号，发送数据。
输入：data: 发送的数据
      WaitFlag: 发送是否等待对方回复标识
输出：
返回值:
*******************************************************************************/
int SendMsgMulti(char *data, enSEND_WAIT_FLAG WaitFlag)
{
    if(!data){
        return -1;
    }

    pSendFun pSendF = WaitFlag ? SendMsgWait:SendMsg;

#if USE_PUBLIC_GROUP //使用公共组播方式
    pSendF(PublicGroudSockFd, pSysInfo->PublicGroudIP, PUBLIC_GROUP_PORT, data);
#else //使用P2P发送方式
    int i,j;
    int DestNum = 0;
    int DevListBytes;
    char DevList[1024];

    //LOGD("SendMsgMulti--->%s", data);

    //取出目标设备列表或编号
    if(get_value_char(key_DevList, data, DevList, sizeof(DevList))){//掩码方式
        DevListBytes = strlen(DevList);
    }
    else if(!(DestNum = get_value_num(key_DestNum, data))){//编号方式
        return -1;
    }

    if(DestNum){
        //LOGD("SendMsgMulti--->%s, %s", &pSysInfo->pDevIP[DestNum].ip, data);
        SendMsg(PublicGroudSockFd, pSysInfo->pDevIP[DestNum].ip, PUBLIC_GROUP_PORT, data);
    }
    else{
        for(i=0; i<DevListBytes && pSysInfo->bRun; i++){
            for(j=1; j!=0x80; j<<=1,DestNum++){
                if((DevList[i]&j) && DestNum!=pSysInfo->DevNum && DestNum<pSysInfo->DevNums){//注意要将本设备过滤掉
                    pSendF(PublicGroudSockFd, pSysInfo->pDevIP[DestNum].ip, PUBLIC_GROUP_PORT, data);
                    //LOGD("SendMsgMulti--->%s, %s", pSysInfo->pDevIP[DestNum].ip, data);
                }
            }
        }
    }
#endif

    return 0;
}

/*******************************************************************************
标题：SendToSvr
功能：发送数据给服务器
输入：
输出：
返回值:
*******************************************************************************/
int SendToSvr(const char *data, enSEND_WAIT_FLAG WaitFlag)
{
    if(WaitFlag == SEND_NO_WAIT){
        //SendMsg(PublicGroudSockFd, "192.168.80.28", SVR_PORT, data);
        return SendMsg(PublicGroudSockFd, pSysInfo->SvrIP, SVR_PORT, data);
    }
    else{
        return SendMsgWait(PublicGroudSockFd, pSysInfo->SvrIP, SVR_PORT, data);
    }
}

/*******************************************************************************
标题：SendToSvr
功能：发送数据给服务器
输入：
输出：
返回值:
*******************************************************************************/
int SendToDev(const char * ip, const char *pdata, enSEND_WAIT_FLAG WaitFlag)
{
    if(WaitFlag == SEND_NO_WAIT){
        return SendMsg(PublicGroudSockFd, ip, PUBLIC_GROUP_PORT, pdata);
    }
    else{
        return SendMsgWait(PublicGroudSockFd, ip, PUBLIC_GROUP_PORT, pdata);
    }
}

/*******************************************************************************
标题：SendAudioData
功能：发送音频数据，除了组播还对不支持组播的设备进行点对点发送，
      以pRtpPacket->header.m位来标识组播(值0)或点对点(值1)
输入：
输出：
返回值:
*******************************************************************************/

void SendAudioData(pPLAY_TASK_INFO pTask, pRTP_PACKET pRtpPacket, int size)
{
    int i,j;
    int num;

    //if(pSysInfo->SendMode == SEND_MODE_MC)
    {
        pRtpPacket->header.m = 0;
        
        UDP_SendData2(AudioDataSockFd, pTask->GroupIP.addr, AUDIO_DATA_PORT, pRtpPacket, size);
    }
    #if 1
    //else
    {
        int sock_switch = 0;
        struct sockaddr_in  dest;

        //为提高效率dest相同内容只填一次
    	dest.sin_family = AF_INET;
    	dest.sin_port = htons(AUDIO_DATA_PORT);

        pRtpPacket->header.m = 1;

        for(i=0,num=0; i<pTask->DevListBytes; i++){
            for(j=1; j!=0x80; j<<=1,num++){
                if(!pSysInfo->pDevIP[num].isMC && (pTask->DevList[i]&j) && num!=pSysInfo->DevNum){//注意要将本设备过滤掉
                    dest.sin_addr.s_addr = pSysInfo->pDevIP[num].addr;
                    #if(SEND_AUDIO_SOCK_NUM > 0)
                    if(++sock_switch >= SEND_AUDIO_SOCK_NUM){
                        sock_switch = 0;
                    }
                    sendto(pSysInfo->SockFd[sock_switch], pRtpPacket, size, MSG_DONTWAIT, (struct sockaddr*)&dest, sizeof(dest));
                    #else
                    sendto(AudioDataSockFd, pRtpPacket, size, MSG_DONTWAIT, (struct sockaddr*)&dest, sizeof(dest));
                    #endif
                }
            }
        }
    }
    #endif
}

void SendAudioDataMC(pPLAY_TASK_INFO pTask, char *data, int size)
{
    UDP_SendData2(AudioDataSockFd, pTask->GroupIP.addr, AUDIO_DATA_PORT, data, size);
}

void SendAudioDataP2P_(pPLAY_TASK_INFO pTask, char *data, int size)
{
    int i,j;
    int num;
	struct sockaddr_in	dest;

	dest.sin_family = AF_INET;
	dest.sin_port = htons(AUDIO_DATA_PORT);

    for(i=0,num=0; i<pTask->DevListBytes; i++){
        for(j=1; j!=0x80; j<<=1,num++){
            if((pTask->DevList[i]&j) && num!=pSysInfo->DevNum && num<pSysInfo->DevNums){//注意要将本设备过滤掉
                dest.sin_addr.s_addr = pSysInfo->pDevIP[num].addr;
                sendto(AudioDataSockFd, data, size, MSG_DONTWAIT, (struct sockaddr*)&dest, sizeof(dest));
            }
        }
    }
}

void SendAudioDataP2P(pPLAY_TASK_INFO pTask, char *data, int size)
{
    int i,j;
    int num;
    int sock_switch = 0;
	struct sockaddr_in	dest;

    //为提高效率dest相同内容只填一次
	dest.sin_family = AF_INET;
	dest.sin_port = htons(AUDIO_DATA_PORT);

    for(i=0,num=0; i<pTask->DevListBytes; i++){
        for(j=1; j!=0x80; j<<=1,num++){
            if((pTask->DevList[i]&j) && num!=pSysInfo->DevNum && num<pSysInfo->DevNums){//注意要将本设备过滤掉
                dest.sin_addr.s_addr = pSysInfo->pDevIP[num].addr;
                #if(SEND_AUDIO_SOCK_NUM > 0)
                if(++sock_switch >= SEND_AUDIO_SOCK_NUM){
                    sock_switch = 0;
                }
                sendto(pSysInfo->SockFd[sock_switch], data, size, MSG_DONTWAIT, (struct sockaddr*)&dest, sizeof(dest));
                #else
                sendto(AudioDataSockFd, data, size, MSG_DONTWAIT, (struct sockaddr*)&dest, sizeof(dest));
                #endif
            }
        }
    }
}

/*******************************************************************************
标题：
功能：
输入：
输出：
返回值:
*******************************************************************************/
int SendAudioCtrl(pPLAY_TASK_INFO pTask)
{
    int i,j;
    int num;
    int sock_switch = 0;
    char *pDevList;
    
    if(!pTask){
        return -1;
    }

    if(pTask->PlayCtrl==PLAY){
        if(!pTask->isBroadcast){
            return 0;
        }
        pDevList = pTask->DevList;
    }
    else{
        pDevList = pTask->DevList2;
    }

    char buf[1024];
    snprintf(buf, sizeof(buf), AudioPlay_cmd, pSysInfo->DevType, pSysInfo->DevNum, pTask->SrcPlayID,        \
             pTask->SrcPlayNum, pTask->StartTime, pTask->priority, pTask->PlayCtrl, pTask->volume,          \
             pTask->format, pTask->pcm.rate, pTask->pcm.channels, pTask->pcm.bitsPerSample, pTask->CurFile,\
             pTask->GroupIP.ip, /*pDevList*/" ");

#if USE_MULTICAST_AUDIO_CTRL 
    //使用组播发送播放控制
    SendMsg(AudioCtrlSockFd, pSysInfo->PublicGroudIP, PUBLIC_GROUP_PORT, buf);
#else 
    //使用P2P发送播放控制
    for(i=0,num=0; i<pTask->DevListBytes; i++){
        for(j=1; j!=0x80; j<<=1,num++){
            if((pDevList[i]&j) && num!=pSysInfo->DevNum && num<pSysInfo->DevNums){//注意要将本设备过滤掉
                //LOGD("SendAudioCtrl------->Task=%d, num=%d, ip=%s", pTask->TaskIndex, num, pSysInfo->pDevIP[num].ip);
                #if(SEND_AUDIO_SOCK_NUM > 0)
                if(++sock_switch >= SEND_AUDIO_SOCK_NUM){
                    sock_switch = 0;
                }
                SendMsg(pSysInfo->SockFd[sock_switch], pSysInfo->pDevIP[num].ip, AUDIO_CTRL_PORT, buf);
                #else
                SendMsg(AudioCtrlSockFd, pSysInfo->pDevIP[num].ip, AUDIO_CTRL_PORT, buf);
                #endif
            }
        }
    }
#endif

    return 0;
}

void SendCheckMulticast(int type)
{
    char buf[sizeof(CheckMulticast_cmd) + 128];

    snprintf(buf, sizeof(buf), CheckMulticast_cmd, pSysInfo->DevNum, pSysInfo->addr, type); 
    SendToDev(pSysInfo->PublicGroudIP, buf, SEND_NO_WAIT);
}

/*******************************************************************************
标题：
功能：
输入：
输出：
返回值:
//由于任务播放过程中定时重复发送播放控命令，在没有曲目、音量等变化时，
//命令数据都是一样的，第1次先解释出判断变化必要的数据到临时pTmpTask中，
//通过这些数据判断出任务是否有变化，有变化再解释其他数据，这样避免做过多无用功。
*******************************************************************************/
int GetNetPlayInfo(pPLAY_TASK_INFO pTask, pPLAY_TASK_INFO pTmpTask, const char *pdata, int flag)
{
    if(!pTmpTask || !pdata){
        return -1;
    }

    if(flag == 1){
        //pTask->SrcDevType = get_value_num(key_SrcType, pdata);
        pTmpTask->SrcDevNum = get_value_num(key_SrcNum, pdata);
        pTmpTask->SrcPlayID = get_value_num(key_PlayID, pdata);
        pTmpTask->SrcPlayNum = get_value_num(key_PlayNum, pdata);
        pTmpTask->PlayCtrl = get_value_num(key_Control, pdata);
        pTmpTask->volume = get_value_num(key_Volume, pdata);
    }
    else if(flag == 2){
        pTmpTask->StartTime = get_value_num(key_StartTime, pdata);
        pTmpTask->priority = get_value_num(key_Priority, pdata);
        pTmpTask->pcm.rate = get_value_num(key_SampleRate, pdata);
        pTmpTask->pcm.channels = get_value_num(key_Channels, pdata);
        pTmpTask->pcm.bitsPerSample = get_value_num(key_BitsPerSample, pdata);
        pTmpTask->format = get_value_num(key_Format, pdata);
    }
    else if(pTask){
        pTask->StartTime = pTmpTask->StartTime;
        pTask->priority = pTmpTask->priority;
        //pTask->SrcDevType = pTmpTask->SrcDevType;
        pTask->SrcDevNum = pTmpTask->SrcDevNum;
        pTask->SrcPlayID = pTmpTask->SrcPlayID;
        pTask->SrcPlayNum = pTmpTask->SrcPlayNum;
        pTask->PlayCtrl = pTmpTask->PlayCtrl;
        pTask->volume = pTmpTask->volume;
        pTask->pcm.rate = pTmpTask->pcm.rate;
        pTask->pcm.channels = pTmpTask->pcm.channels;
        pTask->pcm.bitsPerSample = pTmpTask->pcm.bitsPerSample;
        pTask->format = pTmpTask->format;
        if(pTask->FileList){
            get_value_char(key_FileName, pdata, pTask->FileList, NET_PLAY_NAME_SIZE);
        }
        get_value_char(key_AddGroud, pdata, pTmpTask->GroupIP.ip, sizeof(pTmpTask->GroupIP.ip));
    }
    else{
        return -1;
    }

    return sizeof(PLAY_TASK_INFO);
}



/*******************************************************************************
标题：HandleNetPlayCtrl
功能：处理网络播放控制指
输入：pRecvBuf: 接收到的网络数据缓冲区指针
      len: 数据长度
输出：
返回值: 0: 有效处理，其他: 无效处理
*******************************************************************************/
int HandleNetPlayCtrl(pNET_RECV pNetRecv)
{
    //超时没收到数据或socket异常
    if(pNetRecv->dataLen <= 0){
        if(pNetTask->PlayCtrl != STOP){
            //#if USE_MULTICAST_AUDIO_CTRL
            if(time(NULL) > (pNetTask->LastTime+RECV_PLAY_CTRL_TIMEOUT))
            //#endif
            {
                HandPlayTaskStatus(pNetTask->TaskIndex, STOP);
                PlayCallJava(pNetTask->TaskIndex, STOP, " ");
                //#if USE_MULTICAST_AUDIO_DATA
                pNetTask->GroupIP.ip[0]= 0;
                pNetTask->GroupFlag = TRUE; //让音频接收线程退出组播组
                //#endif
                LOGD("net play end");
            }
        }
        return -1;
    }

    LOGV("%s->%d", pNetRecv->ip, pNetRecv->dataLen);

    //接收到数据处理
    pNetRecv->pdata[pNetRecv->dataLen] = 0;
    HandleMsg(pNetRecv);

    return 0;
}


void *ThreadHandleCmdAudioPlay(void *p)
{
    LOGV("Thread start");

    char *pCmd = (char *)_malloc(512);
    if(!pCmd){
        return NULL;
    }
    
    CreateLink(&AudioPlayCmdLink, 512);
    
    while(pSysInfo->bRun)
    {
        _sem_wait(&AudioPlayCmdLink.sem);
        while(1)
        {
            if(LinkGetData(&AudioPlayCmdLink, pCmd, 512) <= 0){
                break;
            }
            HandleCmdAudioPlay2(pCmd);
        }
    }

    DelLink(&AudioPlayCmdLink);
    _free(pCmd);

    LOGV("Thread end");
    jThreadDetach();
}

/*******************************************************************************
标题：
功能：
输入：
输出：
返回值:
*******************************************************************************/

void *ThreadRecvAudioCtrl(void *p)
{
    LOGV("Thread start");
    
    NET_RECV NetRecv;

    NetRecv.pdata = (char *)_malloc(RECV_AUDIO_BUF_SIZE);

    while(pSysInfo->bRun)
    {
        if(AudioCtrlSockFd < 0){
            sleep(1);
            AudioCtrlSockFd = UDP_Init(NULL, AUDIO_CTRL_PORT, NULL);
            continue;
        }

        while(pSysInfo->bRun)
        {
            NetRecv.dataLen = UDP_RecvData(AudioCtrlSockFd, NetRecv.pdata, NetRecv.ip, &NetRecv.port, RECV_AUDIO_BUF_SIZE-1, RECV_PLAY_CTRL_TIMEOUT*1000);
            HandleNetPlayCtrl(&NetRecv);
            if(NetRecv.dataLen < 0){
                close(AudioCtrlSockFd);
                AudioCtrlSockFd = -1;
                break;
            }
        }   
    }

    if(NetRecv.pdata){
        _free(NetRecv.pdata);
    }
    if(AudioCtrlSockFd > 0){
        close(AudioCtrlSockFd);
        AudioCtrlSockFd = -1;
    }

    LOGV("Thread end");
    jThreadDetach();
}

/*******************************************************************************
标题：
功能：
输入：
输出：
返回值:
*******************************************************************************/

void *ThreadRecvAudioData(void *p)
{    
    LOGV("Thread start");
    
    int i;
    int ret;
    int AddGroupFlag = FALSE;
    char GroupIP[16] = {0};
    int mode = 0;
    int flag = FALSE;

    int nTXBufSize = 64 * 1024;
    
    char *pRecvBuf = (char *)_malloc(RECV_AUDIO_BUF_SIZE);
    pRTP_HEADER pRtpHeader = (pRTP_HEADER)pRecvBuf;

     
    if(!gs_PlayAudioData){
        gs_PlayAudioData = RingBuffCreate(NULL, AUDIO_BUF_SIZE, DYNAMIC_REQ_MEM);
    }

    for(i=0; i<SEND_AUDIO_SOCK_NUM; i++){
        pSysInfo->SockFd[i] = UDP_Init(NULL, 0, NULL);
        ret = setsockopt(pSysInfo->SockFd[i], SOL_SOCKET, SO_SNDBUF, (char *)&nTXBufSize, sizeof(int));
        msleep(20);
    }

    while(pSysInfo->bRun)
    {
        if(AudioDataSockFd < 0){
            sleep(1);
            AudioDataSockFd = UDP_Init(NULL, AUDIO_DATA_PORT, NULL);
            continue;
        }

        if(pNetTask->GroupFlag)
        {
            _lock(&PlayTaskMutex);
            pNetTask->GroupFlag = FALSE;
            memcpy(GroupIP, pNetTask->GroupIP.ip, sizeof(GroupIP));
            _unlock(&PlayTaskMutex);
            
            if(pNetTask->PlayCtrl == STOP || inet_addr(GroupIP) <= 0){
                continue;
            }

            msleep(80);
            
            for(i=0; i<10&&!pNetTask->GroupFlag&&pSysInfo->bRun; i++){
                if(0 == UDP_AddMembership(AudioDataSockFd, GroupIP)){
                    AddGroupFlag = TRUE;
                    break;
                }
                sleep(1);
            }
        }

        LOGD("3");
        
        flag = FALSE;
        
        while(pSysInfo->bRun && !pNetTask->GroupFlag)
        {
            ret = UDP_RecvData(AudioDataSockFd, pRecvBuf, NULL, NULL, RECV_AUDIO_BUF_SIZE, 2000);
            if(ret>sizeof(RTP_HEADER) && pNetTask->isLocalPlay && pNetTask->StartTime==pRtpHeader->ssrc){
                #if 1
                if(mode != pRtpHeader->m){
                    if(flag && pRtpHeader->m!=0){
                        continue;
                    }
                    flag = TRUE;
                    mode = pRtpHeader->m;
                }
                #endif
                ret -= sizeof(RTP_HEADER);
                int AvaDataSize;
                int len = WriteAudioData(gs_PlayAudioData, pRecvBuf+sizeof(RTP_HEADER), ret, &AvaDataSize);
                if(AvaDataSize<=ret || len!=ret){
                    PostAudioData(gs_PlayAudioData);
                    if(len != ret){
                        LOGD("size=%d, write=%d", ret, len);
                    }
                }
            }
            else if(ret < 0){
                close(AudioDataSockFd);
                AudioDataSockFd = -1;
                break;
            }
            else{
                flag = FALSE;
            }
        }

        if(AddGroupFlag)
        {
            AddGroupFlag = FALSE;
            for(i=0; pSysInfo->bRun; i++){
                if(0 == UDP_DropMembership(AudioDataSockFd, GroupIP)){
                    break;
                }
                if(i > 10){
                    close(AudioDataSockFd);
                    AudioDataSockFd = -1;
                    break;
                }
                sleep(1);
            }
        }
    }

    if(pRecvBuf){
        _free(pRecvBuf);
    }

    if(AudioDataSockFd > 0){
        close(AudioDataSockFd);
        AudioDataSockFd = -1;
    }

    for(i=0; i<SEND_AUDIO_SOCK_NUM; i++){
        if(pSysInfo->SockFd[i] > 0){
            close(pSysInfo->SockFd[i]);
        }
        pSysInfo->SockFd[i] = -1;
    }

    LOGV("Thread end");
    jThreadDetach();
}

/*******************************************************************************
标题：
功能：
输入：
输出：
返回值:
*******************************************************************************/

void *ThreadRecvPublicGroudData(void *p)
{    
    LOGV("Thread start");
    
    int RecvRet;
    int TimeOut;
    NET_RECV NetRecv;

    int nRXBufSize = 200 * 1024;

    
    NetRecv.pdata = (char *)_malloc(RECV_PUBLIC_GROUD_BUF_SIZE);

    pSysInfo->AddGroupFlag = FALSE;
    
    while(pSysInfo->bRun)
    {
        if(PublicGroudSockFd < 0){
            PublicGroudSockFd = UDP_Init(NULL, PUBLIC_GROUP_PORT, NULL);
            if(PublicGroudSockFd < 0){
                sleep(1);
                continue;
            }
            setsockopt(PublicGroudSockFd, SOL_SOCKET, SO_RCVBUF, (char *)&nRXBufSize, sizeof(int));

            TimeOut = 5000;
            if(UDP_AddMembership(PublicGroudSockFd, pSysInfo->PublicGroudIP) == 0){
                pSysInfo->AddGroupFlag = TRUE;
                TimeOut = -1;
            }
        }

        while(pSysInfo->bRun)
        {
            RecvRet = UDP_RecvData(PublicGroudSockFd, NetRecv.pdata, NetRecv.ip, &NetRecv.port, RECV_PUBLIC_GROUD_BUF_SIZE-1, TimeOut);
            if(RecvRet > 0){
                //LOGD("ThreadRecvPublicGroudData=%s", NetRecv.pdata);
                NetRecv.pdata[RecvRet] = 0;
                HandleMsg(&NetRecv);
            }
            else if(RecvRet < 0){
                close(PublicGroudSockFd);
                PublicGroudSockFd = -1;
                pSysInfo->AddGroupFlag = FALSE;
                break;
            }
            else if(!pSysInfo->AddGroupFlag){
                if(UDP_AddMembership(PublicGroudSockFd, pSysInfo->PublicGroudIP) == 0){
                    pSysInfo->AddGroupFlag = TRUE;
                    TimeOut = -1;
                }
            }
        }   
    }

    if(pSysInfo->AddGroupFlag){
        pSysInfo->AddGroupFlag = FALSE;
        UDP_DropMembership(PublicGroudSockFd, pSysInfo->PublicGroudIP);
    }
    if(PublicGroudSockFd > 0){
        //close(PublicGroudSockFd);
        //PublicGroudSockFd = -1;
    }
    if(NetRecv.pdata){
        _free(NetRecv.pdata);
    }

    LOGV("Thread end");
    jThreadDetach();
}


/*******************************************************************************
标题：ThreadCheckMulticast
功能：探测其他设备是否支持组播。每个设备都加入一个固定组播组，定时向该组发送指令，
      其他设备能收到便认为相互可以组播。发送音频数据时除了组播，还对不支持组播的
      设备进行点对点发送。
      发送探测指令Type值有2种，1表示希望对方回复，0不回复。收到探测指令表示发送者
      支持组播。本机加入公共组播组后首先发1类型探测指令(希望得到回复，及早知道网络
      中支持组播的设备)，以后发0类型指令(为其他设备探测)。
输入：
输出：
返回值:
*******************************************************************************/
void* ThreadCheckMulticast(void* p)
{ 
    LOGV("Thread start");
    
    int i;
    int SendTimeCnt = 0;
    int CheckTimeCnt = 0;
    char buf[sizeof(CheckMulticast_cmd) + 128];

    while(pSysInfo->bRun)
    {
        if(PublicGroudSockFd > 0 && pSysInfo->AddGroupFlag){
            pSysInfo->CheckMulticastCnt = 1;
            SendCheckMulticast(1);
            break;
        }
        sleep(1);
    }
    
    while(pSysInfo->bRun)
    {
        if(PublicGroudSockFd > 0 && pSysInfo->AddGroupFlag){            
            if(++SendTimeCnt >= 30){
                SendTimeCnt = 0;
                SendCheckMulticast(0);
            }
        }
        
        if(++CheckTimeCnt >= 32){
            CheckTimeCnt = 0;
            _lock(&pSysInfo->lock);
            for(i=0; i<pSysInfo->DevNums; i++){
                if(pSysInfo->pDevIP[i].isMC < pSysInfo->CheckMulticastCnt){
                    pSysInfo->pDevIP[i].isMC = 0;
                }
            }
            _unlock(&pSysInfo->lock);
            pSysInfo->CheckMulticastCnt++;
        }
    
        sleep(1);
    }

    LOGV("Thread end");
    jThreadDetach();
}
/*******************************************************************************
标题：
功能：
输入：
输出：
返回值:
*******************************************************************************/
void* ThreadKeepAlive(void* p)
{    
    LOGV("Thread start");
    
    int i;
    uint8 VoipStatus = 0;
    uint8 PlayStatus = 0;
    int no_answer_cnt = 0;
    char *pbuf = (char *)_malloc(1024);

    isOnline = FALSE;
    pSysInfo->VoipStatus = 0;
    pSysInfo->PlayStatus = 0;

    while(pSysInfo->bRun)
    {   
        snprintf(pbuf, 1024, Register_cmd, pSysInfo->DevType, pSysInfo->SubType, pSysInfo->SN, pSysInfo->DevNum, pSysInfo->IP);
        while(pSysInfo->bRun && !isOnline)
        {
            //LOGV("send Register MyNum=%d", pSysInfo->DevNum);
            SendToSvr(pbuf, SEND_NO_WAIT);
            for(i=0; i<3 && pSysInfo->bRun; i++){
                sleep(1);
            }
        }

        PlayStatus = ~0;
        while(pSysInfo->bRun && isOnline)
        {
            if(VoipStatus!=pSysInfo->VoipStatus || PlayStatus!=pSysInfo->PlayStatus){
                VoipStatus = pSysInfo->VoipStatus;
                PlayStatus = pSysInfo->PlayStatus;
                snprintf(pbuf, 1024, Online_cmd, pSysInfo->DevNum,\
                         pSysInfo->SipNum, VoipStatus&0x03, (VoipStatus>>2)&0x03, (VoipStatus>>4)&0x03,\
                         PlayStatus&0x03, (PlayStatus>>2)&0x03, (PlayStatus>>4)&0x03, (PlayStatus>>6)&0x03\
                         );
            }
            //LOGV("send Online-->VoipStatus=%02X, PlayStatus=%02X", VoipStatus, PlayStatus);
            KeepAliveFlag = FALSE;
            SendToSvr(pbuf, SEND_NO_WAIT);
            
            for(i=0; i<3 && pSysInfo->bRun; i++){
                sleep(1);
            }

            if(!KeepAliveFlag){
                if(++no_answer_cnt >= 3){
                    no_answer_cnt = 0;
                    isOnline = FALSE;
                }
            }
            else{
                no_answer_cnt = 0;
            }
        }
    }

    isOnline = FALSE;
    KeepAliveFlag = FALSE;
    _free(pbuf);

    LOGV("Thread end");
    jThreadDetach();
}


/*******************************************************************************
标题：
功能：
输入：
输出：
返回值:
*******************************************************************************/
void* ThreadSendData(void* p)
{
    LOGV("Thread start");

    char *pSendbuf = (char *)_malloc(1024);
    if(!pSendbuf){
        return NULL;
    }
    
    CreateLink(&SendLink, 1024);
    
    while(pSysInfo->bRun)
    {
        _sem_wait(&SendLink.sem);
        while(1)
        {
            if(LinkGetData(&SendLink, pSendbuf, 1024) <= 0){
                break;
            }
            LOGV("ThreadSendData-->%s", pSendbuf);
            //SendMsgMulti(pSendbuf, SEND_NO_WAIT); 
        }
    }

    DelLink(&SendLink);
    _free(pSendbuf);

    LOGV("Thread end");
    jThreadDetach();
}


/*******************************************************************************
标题：
功能：
输入：
输出：
返回值:
*******************************************************************************/
void* ThreadSearchService(void* p)
{      
    LOGV("Thread start");
    
    int SockFd = -1;
    NET_RECV NetRecv;
    
    NetRecv.pdata = (char *)_malloc(RECV_SEARCH_BUF_SIZE);

    LOGV("1");

    while(pSysInfo->bRun)
    {
        if(SockFd < 0){
            sleep(1);
            SockFd = UDP_Init(NULL, AUDIO_SEARCH_PORT, NULL);
            continue;
        }

        LOGV("2");

        while(pSysInfo->bRun)
        {
            NetRecv.dataLen = UDP_RecvData(SockFd, NetRecv.pdata, NetRecv.ip, &NetRecv.port, RECV_SEARCH_BUF_SIZE-1, -1);
            if(NetRecv.dataLen > 0){
                //LOGD("ThreadSearchService -->%d", NetRecv.dataLen);
                HandleNetConfig(SockFd, &NetRecv);
            }
            else if(NetRecv.dataLen < 0){
                close(SockFd);
                SockFd = -1;
                break;
            }
        }   
    }

    if(NetRecv.pdata){
        _free(NetRecv.pdata);
    }
    if(SockFd > 0){
        close(SockFd);
    }

    LOGV("Thread end");
    jThreadDetach();
}


/*******************************************************************************
标题：
功能：
输入：
输出：
返回值:
*******************************************************************************/

int InitNetWork(void)
{
    LOGV("InitNetWork");

    _sem_init(&SendWait.sem,0,0);
    _lock_init(&SendWait.lock, NULL);
    pNetTask = &PlayTask[0];

    //ThreadCreate(ThreadHandleCmdAudioPlay, NULL);
    ThreadCreate(ThreadRecvAudioCtrl, NULL);
    ThreadCreate(ThreadRecvAudioData, NULL);
    ThreadCreate(ThreadRecvPublicGroudData, NULL);
    ThreadCreate(ThreadCheckMulticast, NULL);
    ThreadCreate(ThreadKeepAlive, NULL);
    //ThreadCreate(ThreadSendData, NULL);
    ThreadCreate(ThreadSearchService, NULL);
}


int DelInitNetWork(void)
{
    int i;
    
    for(i=PLAY_TASK_START_INDEX; i<PLAY_TASK_MAX; i++){
        PlayTask[i].PlayCtrl = STOP;
        if(PlayTask[i].pRecordBuf){
            PostAudioData(PlayTask[i].pRecordBuf);
        }
    }
    if(pNetTask){
        FreePlayTask(pNetTask);
    }
    
    //发送数据触发相应UDP接收线程退出阻塞
    UDP_SendSelf(PublicGroudSockFd, AUDIO_DATA_PORT);
    UDP_SendSelf(PublicGroudSockFd, AUDIO_CTRL_PORT);
    UDP_SendSelf(PublicGroudSockFd, PUBLIC_GROUP_PORT);
    UDP_SendSelf(PublicGroudSockFd, AUDIO_SEARCH_PORT);
    close(PublicGroudSockFd);
    PublicGroudSockFd = -1;
    //LinkAddData(&AudioPlayCmdLink, " ", 2);
    //LinkAddData(&SendLink, " ", 2);

    sleep(1);

    //DelLink(&AudioPlayCmdLink);
    //DelLink(&SendLink);

    _sem_destroy(&SendWait.sem); 
    _lock_destroy(&SendWait.lock);

    return 0;
}


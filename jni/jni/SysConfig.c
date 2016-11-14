#include "stdio.h"
#include "string.h"

#include "SysConfig.h"
#include "md5.h"
#include "main.h"

#include "log.h"

DEV_NET_INFO DevNetInfo;


const char key_Model[]   = "Model";
const char key_DevType[] = "DevType";
const char key_SubType[] = "SubType";
const char key_DevName[] = "DevName";
const char key_Num[]     = "Num";
const char key_SN[]      = "SN";
const char key_IP[]      = "IP";
const char key_Mask[]    = "Mask";
const char key_Gateway[] = "Gateway";
const char key_Mac[]     = "Mac";
const char key_SvrIP[]   = "SvrIP";
const char key_Sip[]     = "Sip";

const char SetNetwork_cmd[] = "{\"Cmd\":\"SetNetwork\",\"IP\":\"%s\",\"Mask\":\"%s\",\"Gateway\":\"%s\",\"SvrIP\":\"%s\",\"Session\":\"\"}";

char c2b(char c)
{
    if(c >= 'a'){
        return c-'a'+0x0a;
    }
    else if(c >= 'A'){
        return c-'A'+0x0a;
    }
    else{
        return c-'0';
    }
}

/*传入字符串MAC格式如:AA:11:22:33:44:55*/
int MAC_Str2Byte(const char *mac_str, char *mac_byte, int size)
{
    int i;

    for(i=0; i<6&&i<size; i++)
    {
        mac_byte[i] = c2b(*mac_str++) << 4;
        mac_byte[i] |= c2b(*mac_str++);
        mac_str++; //跳过':'号
    }

    return 0;
}


/*
{"Cmd":"GetDevInfo","DevType":"2","Gateway":"192.168.80.1","IP":"192.168.80.158","Mac":"ae:11:22:33:44:55","Mask":"255.255.255.0","Model":"nvis_ext","Num":"-1","SN":"","SubType":"","SvrIP":""}
*/
int GetLocalInfo(const char *pdata)
{
    char mac_str[20];
    pDEV_INFO pDevInfoVar = &DevNetInfo.DevInfoVar;
    pNETWORK_INFO pNetWorkVar = &DevNetInfo.NetWorkVar;

    get_value_char(key_IP, pdata, pNetWorkVar->NetIpaddr, sizeof(pNetWorkVar->NetIpaddr));
    get_value_char(key_Mask, pdata, pNetWorkVar->NetSmaddr, sizeof(pNetWorkVar->NetSmaddr));
    get_value_char(key_Gateway, pdata, pNetWorkVar->NetGwaddr, sizeof(pNetWorkVar->NetGwaddr));
    get_value_char(key_SvrIP, pdata, pNetWorkVar->ServerIpaddr, sizeof(pNetWorkVar->ServerIpaddr));
    get_value_char(key_Mac, pdata, mac_str, sizeof(mac_str));
    MAC_Str2Byte(mac_str, pNetWorkVar->NetMac, sizeof(pNetWorkVar->NetMac));

    pSysInfo->DevType = get_value_num(key_DevType, pdata);
    pSysInfo->SubType = get_value_num(key_SubType, pdata);
    pSysInfo->DevNum = get_value_num(key_Num, pdata);
    pSysInfo->SN = get_value_num(key_SN, pdata);
    DevNetInfo.SN = pSysInfo->SN;
    memcpy(pSysInfo->SvrIP, pNetWorkVar->ServerIpaddr, sizeof(pSysInfo->SvrIP));
    memcpy(pSysInfo->IP, pNetWorkVar->NetIpaddr, sizeof(pSysInfo->IP));
    pSysInfo->addr = inet_addr(pSysInfo->IP);
    
    pDevInfoVar->HarDevType = pSysInfo->DevType+0x60;//在搜索工具上0x61是主机，0x62是分机
    get_value_char(key_Model, pdata, pDevInfoVar->HarDevMn, sizeof(pDevInfoVar->HarDevMn));
    get_value_char(key_DevName, pdata, DevNetInfo.DevName, sizeof(DevNetInfo.DevName));
    
    #if 1
    LOGV("GetLocalInfo Model=%s", pDevInfoVar->HarDevMn);
    LOGV("GetLocalInfo DevType=%d", pSysInfo->DevType);
    LOGV("GetLocalInfo SubType=%d", pSysInfo->SubType);
    LOGV("GetLocalInfo DevName=%d", DevNetInfo.DevName);
    LOGV("GetLocalInfo Num=%d", pSysInfo->DevNum);
    LOGV("GetLocalInfo SN=%d", pSysInfo->SN);
    LOGV("GetLocalInfo IP=%s", pNetWorkVar->NetIpaddr);
    LOGV("GetLocalInfo Mask=%s", pNetWorkVar->NetSmaddr);
    LOGV("GetLocalInfo Gateway=%s", pNetWorkVar->NetGwaddr);
    LOGV("GetLocalInfo SvrIP=%s", pNetWorkVar->ServerIpaddr);
    int i;
    for(i=0; i<6; i++){
        sprintf(mac_str+i*3, "%02X ", pNetWorkVar->NetMac[i]);
    }
    LOGV("GetLocalInfo MAC=%s", mac_str);
    #endif
    
    return 0;
}
/*******************************************************************************
标题：GetDevNetConfigInfo
功能：读取设备相关信息
格式：
pDEV_INFO GetDevNetConfigInfo(pDEV_NET_INFO pDevNetVar)

输入：pDevData-->设备传入指针
输出：
返回值:pHEA_BEA_PAK类型数据/NULL
异常：暂没有发现
*******************************************************************************/
pDEV_NET_INFO GetDevNetConfigInfo(void)
{
    int static LoadDevFlag = FALSE;
	pDEV_NET_INFO pDevNetVar = &DevNetInfo;


	if(LoadDevFlag)
	{
        return pDevNetVar;
	}

	//从Flash直接读取信息
    LoadDevFlag = TRUE;
    strncpy(pDevNetVar->DevInfoVar.HarDevVer, SVN_VERSION, sizeof(pDevNetVar->DevInfoVar.HarDevVer));
    //test
    #if 0
    pDevNetVar->DevInfoVar.HarDevType = 0x62;
    memcpy(pDevNetVar->NetWorkVar.NetIpaddr, pSysInfo->IP, sizeof(DevNetInfo.NetWorkVar.NetIpaddr));
    memcpy(pDevNetVar->NetWorkVar.NetGwaddr, "192.168.80.1", sizeof(DevNetInfo.NetWorkVar.NetGwaddr));
    memcpy(pDevNetVar->NetWorkVar.NetSmaddr, "255.255.255.0", sizeof(DevNetInfo.NetWorkVar.NetSmaddr));
    memcpy(pDevNetVar->NetWorkVar.ServerIpaddr, pSysInfo->SvrIP, sizeof(DevNetInfo.NetWorkVar.ServerIpaddr));
    memcpy(pDevNetVar->NetWorkVar.NetMac, "123456", sizeof(DevNetInfo.NetWorkVar.NetMac));
    strcpy(pDevNetVar->DevInfoVar.HarManuf,"AEbell");
    strcpy(pDevNetVar->DevInfoVar.HarDevMn,"NVIS");
    strcpy(pDevNetVar->DevName,"一号房");
    pDevNetVar->SN = 123456789;
    #endif
    
	//判断新FLASH是否是第一次上电，将使用默认配置
	//if(USER_CONFIG_FLAG != pDevNetVar->FlashFlag)
    if(0)
	{
		//写标志
		pDevNetVar->FlashFlag = USER_CONFIG_FLAG;

		//厂商
		strcpy(pDevNetVar->DevInfoVar.HarManuf,"AEbell");
		//设备类型
		pDevNetVar->DevInfoVar.HarDevType = VOIP_APM_DEV;

		//strcpy(pDevNetVar->DevInfoVar.HarDevType,"音频播放器");
		//设备型号
		strcpy(pDevNetVar->DevInfoVar.HarDevMn,"E2013");
		//设备版本
		strcpy(pDevNetVar->DevInfoVar.HarDevVer,"Ver2.0");

		//网络通讯端口
		pDevNetVar->NetWorkVar.NetPort = NETWORK_SOCKET_PORT;
		//网络通讯模式
		pDevNetVar->NetWorkVar.NetMode = TCP_MODE;
		//终端心跳时间
		pDevNetVar->NetWorkVar.NetHpTime = 3;
		//设备MAC地址
		strncpy((CHAR *)pDevNetVar->NetWorkVar.NetMac,"\x10\x11\x12\x13\x14\x15",\
			sizeof(pDevNetVar->NetWorkVar.NetMac));
		//设备IP地址
		strncpy(pDevNetVar->NetWorkVar.NetIpaddr,"192.168.2.100",\
			sizeof(pDevNetVar->NetWorkVar.NetIpaddr));
		//设备网关地址
		strncpy(pDevNetVar->NetWorkVar.NetGwaddr,"192.168.2.1",\
			sizeof(pDevNetVar->NetWorkVar.NetGwaddr));
		//服务器IP
		strncpy(pDevNetVar->NetWorkVar.ServerIpaddr,"192.168.0.1",\
			sizeof(pDevNetVar->NetWorkVar.ServerIpaddr));
		//设备子网掩码
		strncpy(pDevNetVar->NetWorkVar.NetSmaddr,"255.255.255.0",\
			sizeof(pDevNetVar->NetWorkVar.NetSmaddr));
		//
		strcpy(pDevNetVar->NetWorkVar.CommPasswd,"AEBELL");
		//默认不用定时重启
		pDevNetVar->RebootTime = 0xffff;
		//保存设置信息
		//SaveDevNetInfo(0,(UINT8 *)pDevNetVar,sizeof(DEV_NET_INFO),CONFIG_USER_INFO);
	}

	return pDevNetVar;

}

/*******************************************************************************
标题：fill_send_data
功能：填充回复数据
输入：无
输出：无
返回值:
*******************************************************************************/
int fill_send_data(UINT16 cmd, pNET_CONFIG pNetConfigVar, void *pdata, int len)
{
    if(pNetConfigVar == NULL){
        return 0;
    }

    //报文头
    strncpy(pNetConfigVar->Synch,NET_CON_HEADER,sizeof(NET_CON_HEADER)-1);
    //pNetConfigVar->Order = 0x00;
    pNetConfigVar->CtrlCmd = cmd;
    pNetConfigVar->DataSize = sizeof(NET_CONFIG)+len;
    if(len>0 && pdata!=NULL){
        memcpy((UINT8 *)&pNetConfigVar->pData, (UINT8 *)pdata, len);
        pNetConfigVar->DataSize -= sizeof(pNetConfigVar->pData);
    }

    return pNetConfigVar->DataSize;
}

int set_network(pDEV_NET_INFO pDevNetVar)
{
    char buf[512];
    pNETWORK_INFO NetWorkVar = &pDevNetVar->NetWorkVar;

    snprintf(buf, sizeof(buf), SetNetwork_cmd, NetWorkVar->NetIpaddr, NetWorkVar->NetSmaddr, NetWorkVar->NetGwaddr, NetWorkVar->ServerIpaddr);

    return SetNetworkCallJava(buf);
}
/*******************************************************************************
标题：ProcNetCongfig
功能：处理网络配置数据
格式：
UINT16 ProcNetCongfig(void *Data,INT32 DataSize,INT16 *Flag)
输入：无
输出：
返回值:无
异常：暂没有发现
*******************************************************************************/
INT16 ProcNetCongfig(void *Data, INT32 DataSize, int *Flag)
{
	pNET_CONFIG 	 pNetConfigVar;
	pDEV_NET_INFO	 pDevNetVar = NULL;
	DWORD			 CurIpAddr;
	CHAR			 *pRingData = NULL;
	UINT16			 nSize = 0;
	static UINT32	 Count = 0;
    pFILE_CHECK_INFO	pFileCheVar;
    UCHAR				nMd5Buff[16] = {0};
    static UCHAR		RecvMd5Buff[16]= {0};
    struct MD5Context 	Ctx;
    UINT32				Ret;
    WORD                Mac;


	*Flag = SYS_NORMAL;

	if(NULL == Data || DataSize <=0)
	{
		return -1;
	}

	pNetConfigVar = (pNET_CONFIG)Data;

	//判断头
	if( 0 != strncmp(pNetConfigVar->Synch,NET_CON_HEADER, sizeof(NET_CON_HEADER)-1) )
	{
		DEBUGLOG("网络配置失败，协议头不一致!\r\n");
		return -1;
	}
	//获取设备网络等信息
	pDevNetVar = GetDevNetConfigInfo();


	/*****************1、获取IP信息协议头-请求*******************/
	if(CONFIG_HEAD_G_REQ == pNetConfigVar->CtrlCmd )
	{
		fill_send_data(CONFIG_HEAD_G_RER, pNetConfigVar, pDevNetVar, sizeof(DEV_NET_INFO_UP));

		LOGD("---NetIpaddr:%s, NetGwaddr:%s, ServerIpaddr:%s---",\
			pDevNetVar->NetWorkVar.NetIpaddr,pDevNetVar->NetWorkVar.NetGwaddr,\
			pDevNetVar->NetWorkVar.ServerIpaddr);

        return pNetConfigVar->DataSize;
    }
    else{
        Mac = *(WORD *)&pDevNetVar->NetWorkVar.NetMac[4];
        if(pNetConfigVar->Order!=Mac /*|| pNetConfigVar->DestAddr!=CurIpAddr*/){
            return 0;
        }
    }


    /*****************2、修改设备相关参数存入flash*******************/
    if(CONFIG_HEAD_M_REQ == pNetConfigVar->CtrlCmd)
	{
        if(set_network((pDEV_NET_INFO)&pNetConfigVar->pData) != 0){
            return 0;
        }
        pDevNetVar->SystemFlag = SYS_RUN_APP_FLAG ;
        memcpy(pDevNetVar, (char *)&pNetConfigVar->pData, sizeof(DEV_NET_INFO_UP));
		
		fill_send_data(CONFIG_HEAD_M_RER, pNetConfigVar, NULL, 0);

		LOGD("---NetIpaddr:%s, NetGwaddr:%s, ServerIpaddr:%s---",\
			pDevNetVar->NetWorkVar.NetIpaddr,\
			pDevNetVar->NetWorkVar.NetGwaddr,\
			pDevNetVar->NetWorkVar.ServerIpaddr);
		//*Flag = SYS_REBOOT;
	}
	else if(SYS_REBOOT_REQ == pNetConfigVar->CtrlCmd)
	{
        DEBUGLOG("收到重启指令\r\n");
		fill_send_data(SYS_REBOOT_RER, pNetConfigVar, NULL, 0);

		*Flag = SYS_REBOOT;
	}
	else
	{
		return 0;
	}

	return pNetConfigVar->DataSize;
}


int HandleNetConfig(int sock, pNET_RECV pNetRecv)
{
    int flag;
    int DataSize;
    
    if((DataSize = ProcNetCongfig(pNetRecv->pdata, pNetRecv->dataLen, &flag)) > 0)
    {
        UDP_SendData(sock, BRO_ADDR, pNetRecv->port, pNetRecv->pdata, DataSize);
        if(flag == SYS_REBOOT){
            msleep(10);
            system("reboot");
        }
    }
}

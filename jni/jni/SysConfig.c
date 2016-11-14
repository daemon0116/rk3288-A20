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

/*�����ַ���MAC��ʽ��:AA:11:22:33:44:55*/
int MAC_Str2Byte(const char *mac_str, char *mac_byte, int size)
{
    int i;

    for(i=0; i<6&&i<size; i++)
    {
        mac_byte[i] = c2b(*mac_str++) << 4;
        mac_byte[i] |= c2b(*mac_str++);
        mac_str++; //����':'��
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
    
    pDevInfoVar->HarDevType = pSysInfo->DevType+0x60;//������������0x61��������0x62�Ƿֻ�
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
���⣺GetDevNetConfigInfo
���ܣ���ȡ�豸�����Ϣ
��ʽ��
pDEV_INFO GetDevNetConfigInfo(pDEV_NET_INFO pDevNetVar)

���룺pDevData-->�豸����ָ��
�����
����ֵ:pHEA_BEA_PAK��������/NULL
�쳣����û�з���
*******************************************************************************/
pDEV_NET_INFO GetDevNetConfigInfo(void)
{
    int static LoadDevFlag = FALSE;
	pDEV_NET_INFO pDevNetVar = &DevNetInfo;


	if(LoadDevFlag)
	{
        return pDevNetVar;
	}

	//��Flashֱ�Ӷ�ȡ��Ϣ
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
    strcpy(pDevNetVar->DevName,"һ�ŷ�");
    pDevNetVar->SN = 123456789;
    #endif
    
	//�ж���FLASH�Ƿ��ǵ�һ���ϵ磬��ʹ��Ĭ������
	//if(USER_CONFIG_FLAG != pDevNetVar->FlashFlag)
    if(0)
	{
		//д��־
		pDevNetVar->FlashFlag = USER_CONFIG_FLAG;

		//����
		strcpy(pDevNetVar->DevInfoVar.HarManuf,"AEbell");
		//�豸����
		pDevNetVar->DevInfoVar.HarDevType = VOIP_APM_DEV;

		//strcpy(pDevNetVar->DevInfoVar.HarDevType,"��Ƶ������");
		//�豸�ͺ�
		strcpy(pDevNetVar->DevInfoVar.HarDevMn,"E2013");
		//�豸�汾
		strcpy(pDevNetVar->DevInfoVar.HarDevVer,"Ver2.0");

		//����ͨѶ�˿�
		pDevNetVar->NetWorkVar.NetPort = NETWORK_SOCKET_PORT;
		//����ͨѶģʽ
		pDevNetVar->NetWorkVar.NetMode = TCP_MODE;
		//�ն�����ʱ��
		pDevNetVar->NetWorkVar.NetHpTime = 3;
		//�豸MAC��ַ
		strncpy((CHAR *)pDevNetVar->NetWorkVar.NetMac,"\x10\x11\x12\x13\x14\x15",\
			sizeof(pDevNetVar->NetWorkVar.NetMac));
		//�豸IP��ַ
		strncpy(pDevNetVar->NetWorkVar.NetIpaddr,"192.168.2.100",\
			sizeof(pDevNetVar->NetWorkVar.NetIpaddr));
		//�豸���ص�ַ
		strncpy(pDevNetVar->NetWorkVar.NetGwaddr,"192.168.2.1",\
			sizeof(pDevNetVar->NetWorkVar.NetGwaddr));
		//������IP
		strncpy(pDevNetVar->NetWorkVar.ServerIpaddr,"192.168.0.1",\
			sizeof(pDevNetVar->NetWorkVar.ServerIpaddr));
		//�豸��������
		strncpy(pDevNetVar->NetWorkVar.NetSmaddr,"255.255.255.0",\
			sizeof(pDevNetVar->NetWorkVar.NetSmaddr));
		//
		strcpy(pDevNetVar->NetWorkVar.CommPasswd,"AEBELL");
		//Ĭ�ϲ��ö�ʱ����
		pDevNetVar->RebootTime = 0xffff;
		//����������Ϣ
		//SaveDevNetInfo(0,(UINT8 *)pDevNetVar,sizeof(DEV_NET_INFO),CONFIG_USER_INFO);
	}

	return pDevNetVar;

}

/*******************************************************************************
���⣺fill_send_data
���ܣ����ظ�����
���룺��
�������
����ֵ:
*******************************************************************************/
int fill_send_data(UINT16 cmd, pNET_CONFIG pNetConfigVar, void *pdata, int len)
{
    if(pNetConfigVar == NULL){
        return 0;
    }

    //����ͷ
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
���⣺ProcNetCongfig
���ܣ�����������������
��ʽ��
UINT16 ProcNetCongfig(void *Data,INT32 DataSize,INT16 *Flag)
���룺��
�����
����ֵ:��
�쳣����û�з���
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

	//�ж�ͷ
	if( 0 != strncmp(pNetConfigVar->Synch,NET_CON_HEADER, sizeof(NET_CON_HEADER)-1) )
	{
		DEBUGLOG("��������ʧ�ܣ�Э��ͷ��һ��!\r\n");
		return -1;
	}
	//��ȡ�豸�������Ϣ
	pDevNetVar = GetDevNetConfigInfo();


	/*****************1����ȡIP��ϢЭ��ͷ-����*******************/
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


    /*****************2���޸��豸��ز�������flash*******************/
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
        DEBUGLOG("�յ�����ָ��\r\n");
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

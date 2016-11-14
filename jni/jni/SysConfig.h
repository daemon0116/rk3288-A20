#ifndef _SYS_CONFIG_H_
#define _SYS_CONFIG_H_

#include "TypepDef.h"
#include "NetWork.h"


#define SVN_VERSION "APP V4.0 56881"	//�ַ�����󳤶�Ϊ17


//flashд��־
#define USER_CONFIG_FLAG	0xAA55AA55


enum SysRunMode
{
	SYS_UPDATE_FLAG = 0x5555AAAA,	 //ϵͳ������־
	SYS_RUN_APP_FLAG = 0xAAAA5555	 //ϵͳ����APP��־
};


//����ͨ��ģʽ
enum ProtocolType
{
	TCP_MODE,
	UDP_MODE
};

/*****************�������ò���UDP�㲥����***************/
//������������ʹ�þ������ն��ڲ�������ͨѸЭ�鲻��Ҫ���ڸ���
//Synch��ͬ��ͷ��+ DataSize (���ݳ���)+ CtrlCmd���������+Data�����ݣ�+ EndId����������

//Synch��ͬ��ͷ����"AENC"
//DataSize (���ݳ���)�����ܳ��ȣ���2���ֽ�
//CtrlCmd�������������2���ֽ�
//EndId�������������̶�Ϊ0xFFFF ��2���ֽ�

#define 	NET_CON_HEADER		"AENC"

//UDP���޹㲥��ַ
#define BRO_ADDR				"255.255.255.255"

//VOIP����
#define VOIP_GW                 "192.168.2.1"

//��voipģ���豸�ı���IP
#define LOCAL_IP                "192.168.2.10"

//
#define SUB_MASK                "255.255.255.0"


//���ض˿�
#define CON_LOCAL_PORT          5555
#define	NETWORK_SOCKET_PORT     8888


enum  DevType
{
	VOIP_APM_DEV = 1,	//1//IP��Ƶ������	Audio Player Machine
	VOIP_ADM_DEV,	 	//2//IP��Ƶ������	Audio Decoder Machine
	VOIP_AAM_DEV,    	//3//IP����		Audio Amplifier Machine
	VOIP_TAM_DEV,    	//4//IP�Խ�����վ	Talkback Station Machine
	VOIP_1KH_DEV,    	//5//һ��������one key help Machine
	VOIP_EAM_DEV,    	//6//IP32·��������	Environment Alarm Machine
	VOIP_PMM_DEV,    	//7//IP16·��Դ������	Power Manager Machine
	VOIP_ACM_DEV,   	//8//IP��Ƶ�ɼ���	Audio Collect Machine
	VOIP_20KH_DEV,		//9//��ʮ��������
	VOIP_SFH_DEV,		//10//IPƽ��������
	RADIO_DEV,          //11//����������
	TELECONTROLLER_DEV, //12//����ң����
	MULTIMEDIA_DEV,     //13//��ý�������ն�
	VOIP_SFH_DEV2,		//14//��Ϣ�����ն�
    
	VL_ALARM_DEV = 21,	//21//���ⱨ����
	EM_CTRL_8_DEV,		//22//8��λӦ�������ն�
	EM_CTRL_12_DEV,		//23//12��λӦ�������ն�
	SENTRY_DEV,			//24//��λ̨
	EM_CALL_DEV,		//25//��������վ

	VOIP_ALL_DEV=0xff	////�㲥--�����ն��豸���Դ�������Ч
};


enum SysConfig
{
	//�޸���ϢЭ��ͷ-����
	CONFIG_HEAD_M_REQ = 1,
	//�޸���ϢЭ��ͷ-��Ӧ
	CONFIG_HEAD_M_RER,

	//��ȡ��ϢЭ��ͷ-����
	CONFIG_HEAD_G_REQ,
	//��ȡ��ϢЭ��ͷ-��Ӧ
	CONFIG_HEAD_G_RER,

	//ϵͳ����--����
	UPDATE_DATA,
	//������ʼ--����
	UPDATE_DATA_START_REQ,
	//������ʼ--��Ӧ
	UPDATE_DATA_START_RER,
	//��������--����
	UPDATE_DATA_END_REQ,
	//��������--��Ӧ
	UPDATE_DATA_END_RER,
	//��������
	UPDATE_DATA_ERR,
	//ͬ��ʱ����Ϣ
	UPDATE_TIME_REQ,
	//ͬ��ʱ����Ϣ-��Ӧ
	UPDATE_TIME_M_RER,
	//����bootloder����,(���̺�����������һ���ģ���ֻ�ǿ�ʼ����һ������)
	UPDATE_BOOTLODER_REQ,
	//VOIPģ����NATģʽ�������޸ĵװ�IP
    VOIP_MODIFY_BOARD_IP_REQ,
    VOIP_MODIFY_BOARD_IP_RER,

    //�޸�SIP��Ϣ-����(SIP���ָ��ֻ��VOIPģ��ʹ��)
    SET_SIP_REQ=20,
    //�޸�SIP��Ϣ-��Ӧ
    SET_SIP_RER,
    //��ȡSIP��Ϣ-����
    GET_SIP_REQ,
    //��ȡSIP��Ϣ-��Ӧ
    GET_SIP_RER,
    //�����豸-����
    SYS_REBOOT_REQ,
    //�����豸-��Ӧ
    SYS_REBOOT_RER,

    //��ȡsip��Ϣ-����
    SVR_GET_SIP_REQ,
    //��ȡsip��Ϣ-��Ӧ
    SVR_GET_SIP_RER,

    //����sip��Ϣ-����
    SVR_SET_SIP_REQ,
    //����sip��Ϣ-��Ӧ
    SVR_SET_SIP_RER,

    //���ý�������վ���뱾-����
	SET_NUMBER_LIST_REQ,
	//���ý�������վ���뱾-��Ӧ
	SET_NUMBER_LIST_RER,

	//����LED������-����
	SET_LED_SCREEN_PARA_REQ,
	//����LED������-��Ӧ
	SET_LED_SCREEN_PARA_RER,

	//��ȡ��������վ���뱾��Ϣ-����
	GET_NUMBER_LIST_REQ,
	//��ȡ��������վ���뱾��Ϣ-��Ӧ
	GET_NUMBER_LIST_RER
};


enum SysOper
{
	SYS_NORMAL = 0,
	SYS_REBOOT ,	//ϵͳ����
	APP_TO_BOOT,	//Ӧ��APP��ת��BOOTLoad
	BOOT_TO_APP,		//BOOTLoad��ת��Ӧ��APP
	SYS_UPDATE_RER,  //ϵͳ������Ӧ
	SYS_UPDATE_ERR	//����ʧ��
};

#define DEV_NET_INFO_EX_SIZE                64
#define  	MAX_NET_STRING_TYPE_SIZE		17

#pragma pack(1)
//�豸��Ϣ
typedef struct DevInfo
{
	CHAR	HarManuf[MAX_NET_STRING_TYPE_SIZE];//����
	BYTE	HarDevType;//�豸����
	CHAR	HarDevMn[MAX_NET_STRING_TYPE_SIZE];//�豸�ͺ�
	CHAR	HarDevVer[MAX_NET_STRING_TYPE_SIZE];//�豸�汾
	DWORD	SerSysRunTime;//����������ʱ��-ms

}DEV_INFO,*pDEV_INFO;
//������Ϣ
typedef struct NetWorkInfo
{
	WORD	NetPort;//����ͨѶ�˿�
	BYTE	NetMode;//����ͨѶģʽ
	BYTE	NetHpTime;//�ն�����ʱ��
	BYTE	NetMac[6];//�豸MAC��ַ
	CHAR	NetIpaddr[16];//�豸IP��ַ
	CHAR	NetGwaddr[16];//�豸���ص�ַ
	CHAR	NetSmaddr[16];//�豸��������
	CHAR	ServerIpaddr[16];//������IP��ַ
	CHAR	CommPasswd[16];//ͨѶ����
	INT32	Socketfd;//�ļ�������
	BYTE	NetConnSta;//��������
}NETWORK_INFO,*pNETWORK_INFO;


//PC�·������ϴ����豸��Ϣ��������Ϣ
typedef	struct DevNetInfoUp
{
	UINT32			FlashFlag;
	DEV_INFO		DevInfoVar;
	NETWORK_INFO	NetWorkVar;
	DWORD 			RelaySta;//���ڱ����Դ�������̵���״̬
	UINT32			SystemFlag;//������־
	WORD			RebootTime;
	UINT32          SN;
    char            DevName[20];
	char            res[56];
}DEV_NET_INFO_UP,*pDEV_NET_INFO_UP;

//�豸��Ϣ��������Ϣ,�豣����Flash
typedef	struct DevNetInfo
{
	//char			ProHead[sizeof(CONFIG_HEAD_G_REQ)-1];//Э��ͷ
	UINT32			FlashFlag;
	DEV_INFO		DevInfoVar;
	NETWORK_INFO	NetWorkVar;
	DWORD 			RelaySta;//���ڱ����Դ�������̵���״̬
	UINT32			SystemFlag;//������־
	WORD			RebootTime;
	UINT32          SN;
    char            DevName[20];
	char            res[56];
    CHAR            ex[DEV_NET_INFO_EX_SIZE];
}DEV_NET_INFO,*pDEV_NET_INFO;

#pragma pack()


typedef struct FileCheckInfo
{
	UCHAR		Md5Buff[16];
	UINT32		SoftVer;
}FILE_CHECK_INFO,*pFILE_CHECK_INFO;


#pragma pack(1)
//��������(��λ������)--�����޸�������Ϣ��ϵͳ��������������(�Խ�)
typedef struct 	NetConfig
{
	CHAR		Synch[sizeof(NET_CON_HEADER)-1];//ͬ��ͷ
	WORD		DataSize;//Э�������ݵĳ���
	WORD		Order;
	WORD		CtrlCmd;//��������
	DWORD		DestAddr;//Ŀ��IP
	void		*pData;//����
	//WORD		EndID;//������
}NET_CONFIG,*pNET_CONFIG;
#pragma pack()

int HandleNetConfig(int sock, pNET_RECV pNetRecv);
int fill_send_data(UINT16 cmd, pNET_CONFIG pNetConfigVar, void *pdata, int len);

#endif

#ifndef _SYS_CONFIG_H_
#define _SYS_CONFIG_H_

#include "TypepDef.h"
#include "NetWork.h"


#define SVN_VERSION "APP V4.0 56881"	//字符串最大长度为17


//flash写标志
#define USER_CONFIG_FLAG	0xAA55AA55


enum SysRunMode
{
	SYS_UPDATE_FLAG = 0x5555AAAA,	 //系统升级标志
	SYS_RUN_APP_FLAG = 0xAAAA5555	 //系统运行APP标志
};


//网络通信模式
enum ProtocolType
{
	TCP_MODE,
	UDP_MODE
};

/*****************网络配置采用UDP广播进行***************/
//由于网络配置使用局限于终端内部，所以通迅协议不需要过于复杂
//Synch（同步头）+ DataSize (数据长度)+ CtrlCmd（控制命令）+Data（数据）+ EndId（结束符）

//Synch（同步头）："AENC"
//DataSize (数据长度)：包总长度，共2个字节
//CtrlCmd（控制命令）：共2个字节
//EndId（结束符）：固定为0xFFFF 共2个字节

#define 	NET_CON_HEADER		"AENC"

//UDP受限广播地址
#define BRO_ADDR				"255.255.255.255"

//VOIP网关
#define VOIP_GW                 "192.168.2.1"

//带voip模块设备的本机IP
#define LOCAL_IP                "192.168.2.10"

//
#define SUB_MASK                "255.255.255.0"


//本地端口
#define CON_LOCAL_PORT          5555
#define	NETWORK_SOCKET_PORT     8888


enum  DevType
{
	VOIP_APM_DEV = 1,	//1//IP音频播放器	Audio Player Machine
	VOIP_ADM_DEV,	 	//2//IP音频解码器	Audio Decoder Machine
	VOIP_AAM_DEV,    	//3//IP功放		Audio Amplifier Machine
	VOIP_TAM_DEV,    	//4//IP对讲呼叫站	Talkback Station Machine
	VOIP_1KH_DEV,    	//5//一键求助器one key help Machine
	VOIP_EAM_DEV,    	//6//IP32路消防主机	Environment Alarm Machine
	VOIP_PMM_DEV,    	//7//IP16路电源管理器	Power Manager Machine
	VOIP_ACM_DEV,   	//8//IP音频采集器	Audio Collect Machine
	VOIP_20KH_DEV,		//9//二十键求助器
	VOIP_SFH_DEV,		//10//IP平安求助器
	RADIO_DEV,          //11//网络收音机
	TELECONTROLLER_DEV, //12//网络遥控器
	MULTIMEDIA_DEV,     //13//多媒体离线终端
	VOIP_SFH_DEV2,		//14//信息发布终端
    
	VL_ALARM_DEV = 21,	//21//声光报警器
	EM_CTRL_8_DEV,		//22//8哨位应急控制终端
	EM_CTRL_12_DEV,		//23//12哨位应急控制终端
	SENTRY_DEV,			//24//哨位台
	EM_CALL_DEV,		//25//紧急呼叫站

	VOIP_ALL_DEV=0xff	////广播--所有终端设备均对此类型有效
};


enum SysConfig
{
	//修改信息协议头-请求
	CONFIG_HEAD_M_REQ = 1,
	//修改信息协议头-响应
	CONFIG_HEAD_M_RER,

	//获取信息协议头-请求
	CONFIG_HEAD_G_REQ,
	//获取信息协议头-响应
	CONFIG_HEAD_G_RER,

	//系统升级--数据
	UPDATE_DATA,
	//升级开始--请求
	UPDATE_DATA_START_REQ,
	//升级开始--响应
	UPDATE_DATA_START_RER,
	//升级结束--请求
	UPDATE_DATA_END_REQ,
	//升级结束--响应
	UPDATE_DATA_END_RER,
	//升级错误
	UPDATE_DATA_ERR,
	//同步时间信息
	UPDATE_TIME_REQ,
	//同步时间信息-响应
	UPDATE_TIME_M_RER,
	//升级bootloder请求,(过程和升级程序是一样的，就只是开始请求不一样而已)
	UPDATE_BOOTLODER_REQ,
	//VOIP模块在NAT模式下请求修改底板IP
    VOIP_MODIFY_BOARD_IP_REQ,
    VOIP_MODIFY_BOARD_IP_RER,

    //修改SIP信息-请求(SIP相关指令只给VOIP模块使用)
    SET_SIP_REQ=20,
    //修改SIP信息-响应
    SET_SIP_RER,
    //获取SIP信息-请求
    GET_SIP_REQ,
    //获取SIP信息-响应
    GET_SIP_RER,
    //重启设备-请求
    SYS_REBOOT_REQ,
    //重启设备-响应
    SYS_REBOOT_RER,

    //获取sip信息-请求
    SVR_GET_SIP_REQ,
    //获取sip信息-响应
    SVR_GET_SIP_RER,

    //设置sip信息-请求
    SVR_SET_SIP_REQ,
    //设置sip信息-响应
    SVR_SET_SIP_RER,

    //设置紧急呼叫站号码本-请求
	SET_NUMBER_LIST_REQ,
	//设置紧急呼叫站号码本-响应
	SET_NUMBER_LIST_RER,

	//设置LED屏参数-请求
	SET_LED_SCREEN_PARA_REQ,
	//设置LED屏参数-响应
	SET_LED_SCREEN_PARA_RER,

	//获取紧急呼叫站号码本信息-请求
	GET_NUMBER_LIST_REQ,
	//获取紧急呼叫站号码本信息-响应
	GET_NUMBER_LIST_RER
};


enum SysOper
{
	SYS_NORMAL = 0,
	SYS_REBOOT ,	//系统重启
	APP_TO_BOOT,	//应用APP跳转到BOOTLoad
	BOOT_TO_APP,		//BOOTLoad跳转到应用APP
	SYS_UPDATE_RER,  //系统升级响应
	SYS_UPDATE_ERR	//升级失败
};

#define DEV_NET_INFO_EX_SIZE                64
#define  	MAX_NET_STRING_TYPE_SIZE		17

#pragma pack(1)
//设备信息
typedef struct DevInfo
{
	CHAR	HarManuf[MAX_NET_STRING_TYPE_SIZE];//厂商
	BYTE	HarDevType;//设备类型
	CHAR	HarDevMn[MAX_NET_STRING_TYPE_SIZE];//设备型号
	CHAR	HarDevVer[MAX_NET_STRING_TYPE_SIZE];//设备版本
	DWORD	SerSysRunTime;//服务器运行时间-ms

}DEV_INFO,*pDEV_INFO;
//网络信息
typedef struct NetWorkInfo
{
	WORD	NetPort;//网络通讯端口
	BYTE	NetMode;//网络通讯模式
	BYTE	NetHpTime;//终端心跳时间
	BYTE	NetMac[6];//设备MAC地址
	CHAR	NetIpaddr[16];//设备IP地址
	CHAR	NetGwaddr[16];//设备网关地址
	CHAR	NetSmaddr[16];//设备子网掩码
	CHAR	ServerIpaddr[16];//服务器IP地址
	CHAR	CommPasswd[16];//通讯密码
	INT32	Socketfd;//文件描述符
	BYTE	NetConnSta;//网络连接
}NETWORK_INFO,*pNETWORK_INFO;


//PC下发、或上传的设备信息、网络信息
typedef	struct DevNetInfoUp
{
	UINT32			FlashFlag;
	DEV_INFO		DevInfoVar;
	NETWORK_INFO	NetWorkVar;
	DWORD 			RelaySta;//用于保存电源管理器继电器状态
	UINT32			SystemFlag;//升级标志
	WORD			RebootTime;
	UINT32          SN;
    char            DevName[20];
	char            res[56];
}DEV_NET_INFO_UP,*pDEV_NET_INFO_UP;

//设备信息、网络信息,需保存在Flash
typedef	struct DevNetInfo
{
	//char			ProHead[sizeof(CONFIG_HEAD_G_REQ)-1];//协议头
	UINT32			FlashFlag;
	DEV_INFO		DevInfoVar;
	NETWORK_INFO	NetWorkVar;
	DWORD 			RelaySta;//用于保存电源管理器继电器状态
	UINT32			SystemFlag;//升级标志
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
//网络配置(上位机配置)--用于修改网络信息、系统升级、铃声更新(对讲)
typedef struct 	NetConfig
{
	CHAR		Synch[sizeof(NET_CON_HEADER)-1];//同步头
	WORD		DataSize;//协议中数据的长度
	WORD		Order;
	WORD		CtrlCmd;//控制命令
	DWORD		DestAddr;//目标IP
	void		*pData;//参数
	//WORD		EndID;//结束符
}NET_CONFIG,*pNET_CONFIG;
#pragma pack()

int HandleNetConfig(int sock, pNET_RECV pNetRecv);
int fill_send_data(UINT16 cmd, pNET_CONFIG pNetConfigVar, void *pdata, int len);

#endif

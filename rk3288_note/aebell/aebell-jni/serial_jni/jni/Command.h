#if !defined(_COMMAND_H_)
#define _COMMAND_H_


#define __PACKED__        __attribute__ ((packed))

//#include "Serial.h"

// #define	TEMPLATE_DOWN


typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned char * PBYTE;
typedef unsigned char * LPBYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned int UINT;
typedef signed int LONG;

#define FALSE 0
#define TRUE  1

typedef enum
{
	FLOW_NONE,
	FLOW_HARDWARE,
	FLOW_SOFTWARE
}enFlowCtrl;




#define FINGER_OPERA_TIMEOUT			5		//指纹仪操作超时
#define FINGER_UART_READ_TIMEOUT	6		//指纹仪串口读超时


//////////////////////////////	DEFINE	////////////////////////////////////////////
#define CMD_PREFIX_CODE							0xAA55
#define RCM_PREFIX_CODE							0x55AA
#define CMD_DATA_PREFIX_CODE					0xA55A
#define RCM_DATA_PREFIX_CODE					0x5AA5

#define CMD_VERIFY_CODE							0x0101
#define CMD_IDENTIFY_CODE						0x0102
#define CMD_ENROLL_CODE							0x0103
#define CMD_ENROLL_ONETIME_CODE					0x0104
#define CMD_CLEAR_TEMPLATE_CODE					0x0105
#define CMD_CLEAR_ALLTEMPLATE_CODE				0x0106
#define CMD_GET_EMPTY_ID_CODE					0x0107
#define CMD_GET_TEMPLATE_STATUS_CODE			0x0108
#define CMD_GET_BROKEN_TEMPLATE_CODE			0x0109
#define CMD_READ_TEMPLATE_CODE					0x010A
#define CMD_WRITE_TEMPLATE_CODE					0x010B
#define CMD_SET_SECURITYLEVEL_CODE				0x010C
#define	CMD_GET_SECURITYLEVEL_CODE				0x010D
#define CMD_SET_FP_TIMEOUT_CODE					0x010E
#define CMD_GET_FP_TIMEOUT_CODE					0x010F
#define CMD_SET_DEV_ID_CODE						0x0110
#define CMD_GET_DEV_ID_CODE						0x0111
#define CMD_GET_FW_VERSION_CODE					0x0112
#define CMD_FINGER_DETECT_CODE					0x0113
#define	CMD_SET_BAUDRATE_CODE					0x0114
#define CMD_SET_DUP_CHECK_CODE					0x0115
#define	CMD_GET_DUP_CHECK_CODE					0x0116
#define	CMD_ENTER_STANDBY_MODE_CODE				0x0117
#define CMD_ENROLL_AND_STORE_CODE				0x0118
#define CMD_GET_ENROLL_DATA_CODE				0x0119
#define CMD_ENROLL_AND_READ_CODE				0x011A
#define CMD_VERIFY_WITH_TMPL_CODE				0x011B
#define CMD_IDENTIFY_WITH_TMPL_CODE				0x011C

#define CMD_GET_DEV_NAME_CODE					0x0121

#define CMD_FP_CANCEL_CODE						0x0130
#define CMD_TEST_CONNECTION_CODE				0x0150

#define RCM_INCORRECT_COMMAND_CODE				0x0160

/***************************************************************************/

#define	ERR_FAIL					1
#define	ERR_SUCCESS					0
#define	ERR_CONTINUE				2

#define	ERR_VERIFY					0x11
#define	ERR_IDENTIFY				0x12
#define	ERR_TMPL_EMPTY				0x13
#define	ERR_TMPL_NOT_EMPTY			0x14
#define	ERR_ALL_TMPL_EMPTY			0x15
#define	ERR_EMPTY_ID_NOEXIST		0x16
#define	ERR_BROKEN_ID_NOEXIST		0x17
#define	ERR_INVALID_TMPL_DATA		0x18
#define	ERR_DUPLICATION_ID			0x19
#define	ERR_BAD_QUALITY				0x21
#define	ERR_TIME_OUT				0x23

#define	ERR_GENERALIZE				0x30
#define	ERR_COM_TIMEOUT				0x40

#define	ERR_INTERNAL				0x50
#define	ERR_MEMORY					0x51
#define	ERR_EXCEPTION				0x52

#define	ERR_INVALID_TMPL_NO			0x60
#define	ERR_INVALID_SEC_VAL			0x61
#define	ERR_INVALID_TIME_OUT		0x62
#define	ERR_INVALID_BAUDRATE		0x63
#define	ERR_DEVICE_ID_EMPTY			0x64
#define	ERR_INVALID_DUP_VAL			0x65

#define	ERR_INVALID_PARAM			0x70

#define	GD_NEED_FIRST_SWEEP			0x81
#define	GD_NEED_SECOND_SWEEP		0x82
#define	GD_NEED_THIRD_SWEEP			0x83
#define	GD_NEED_RELEASE_FINGER		0x84

#define	GD_TEMPLATE_NOT_EMPTY		0x01
#define	GD_TEMPLATE_EMPTY			0x00

#define	GD_DETECT_FINGER			0x01
#define	GD_NO_DETECT_FINGER			0x00

#define	GD_DOWNLOAD_SUCCESS			0xA1

/***************************************************************************/
#define	MAX_DATA_LEN				512

#define	GD_LOW_SEC_LEVEL				1
#define	GD_HIGH_SEC_LEVEL				5

#define	GD_MAX_FP_TIME_OUT				10
#define	GD_DEFAUT_FP_TIME_OUT			5
#define	GD_MIN_FP_TIME_OUT				1

#define	FpSDK_Feature_Size				(380)
#define	FpSDK_FPIndex_Size				(116)
#define GD_TEMPLATE_SIZE				FpSDK_Feature_Size+FpSDK_FPIndex_Size
#define GD_RECORD_SIZE					(GD_TEMPLATE_SIZE + 2)	// CkeckSum len = 2
#define	GD_MAX_RECORD_COUNT				2000

#define	COMM_TIMEOUT				15000  //超时时间15秒
//////////////////////////////	STRUCT	////////////////////////////////////////////

struct _ST_COM_PACKET_
{
	WORD wPrefix;
	WORD wCMD_RCM;
	WORD wDataLen;
	union
	{
		BYTE tbCMDData[16];
		struct
		{
			WORD wRet;
			BYTE tbRcmData[14];
		} stRcmData;
	};
};//__PACKED__;

typedef struct _ST_COM_PACKET_ ST_COM_PACKET, *PST_COM_PACKET;

typedef struct _ST_COMMAND_
{
	char szCommandName[64];
	WORD wCode;
}ST_COMMAND, *PST_COMMAND;
//__PACKED__;

#define	COMMAND_DATALEN				(g_pPacketBuffer->wDataLen)
#define	COMMAND_DATA1				(*(WORD*)g_pPacketBuffer->tbCMDData)
#define	COMMAND_DATA2				(*(WORD*)&g_pPacketBuffer->tbCMDData[2])
#define	COMMAND_CHECK				*(WORD*)&g_Packet[CMD_PACKET_LEN]
#define	RESPONSE_DATA1				(*(WORD*)g_pPacketBuffer->stRcmData.tbRcmData)
#define	RESPONSE_DATA2				(*(WORD*)(g_pPacketBuffer->stRcmData.tbRcmData+2))
#define	RESPONSE_RET				(g_pPacketBuffer->stRcmData.wRet)

#define	CMD_PACKET_LEN				(sizeof(ST_COM_PACKET))
#define	DATA_PACKET_LEN				(g_pPacketBuffer->wDataLen + 6)
/////////////////////////////	Value	/////////////////////////////////////////////
//extern CSerial			g_Serial;
extern BYTE g_Packet[MAX_DATA_LEN + 10];
extern PST_COM_PACKET g_pPacketBuffer;
extern ST_COMMAND g_Commands[];

/////////////////////////////	Function	/////////////////////////////////////////////
WORD GetCheckSum(int bCmdData);
WORD AddCheckSum(int bCmdData);
BOOL CheckReceive(WORD p_Prefix, WORD p_CMD);
void InitPacket(WORD p_wCMD, BOOL p_bCmdData);
BOOL SendCommand(int file, WORD p_wCMD);
BOOL ReceiveAck(int file, WORD p_wCMD, BOOL p_bCmdData);

BOOL SendData(int file, WORD p_wCMD, int p_nDataLen, PBYTE p_pData);
BOOL SendDataPacket(int file, WORD p_wCMD);
BOOL ReceiveData(int file, WORD p_wCMD, int p_nDataLen, PBYTE p_pData);
BOOL ReceiveDataPaket(int file, WORD p_wCMD, WORD p_wDataLen);

char* GetErrorMsg(WORD p_wErrorCode);



#endif // !defined(AFX_COMMAND_H__140A1824_D14D_42C0_A498_48CE7FF70937__INCLUDED_)

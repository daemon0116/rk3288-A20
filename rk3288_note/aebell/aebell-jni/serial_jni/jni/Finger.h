#ifndef _FINGER_H_
#define _FINGER_H_

#include "Command.h"
#include "SerialPort.h"

//指纹仪操作指令
typedef enum _FINGERTASKTYPE
{
	emFingerAdd,    //下发指纹
	emFingerDel,    //删除指纹
	emFingerClear,  //清空指纹
	emFingerCheck,  //验证指纹
	emFingerSetTimeout,//设置录入指纹超时时间
} FINGERTASKTYPE;

//指纹仪操作接口参数
struct _FINGERTASK
{
	FINGERTASKTYPE type;
	int nId;
	BOOL bOperType;//操作员类型,True是换岗
	BYTE* pData;
} __PACKED__;
typedef struct _FINGERTASK FINGERTASK;

typedef struct _CFinger
{
	//属性
	void* this;
	int  m_nHandle;
	//操作
	BOOL (* Open)(char *filename, enBaudRate eBaudRate, enDataBits eDataBits, enStopBits eStopBits, enParityCheck eParityCheck);
	BOOL (*Write) (int nTempleId,BYTE* pFingerData,int nFingerDataSize);
	BOOL (*Check)(int* nTemplateId);
	BOOL (*Delete)(int nTempleId);
	BOOL (*Clean)(void);
	BOOL (*Close)(void);
	BOOL (*Cancel)(void);
	BOOL (*SetTimeout)(int timeoutVal);
}CFinger,*PCFinger;

/*******************************

********************************/
//打开指纹仪
BOOL FingerOpen(char *filename, int eBaudRate, enDataBits eDataBits, enStopBits eStopBits, enParityCheck eParityCheck);
//关闭指纹仪
BOOL FingerClose(void);

int FingerAdd(int nId, BYTE* pData);
int FingerDelete(int nId);
int FingerClean(void);
int FingerCheck(int *nId);
int FingerSetTimeout(int timeoutVal);

//操作指纹仪
int FingerDo(FINGERTASK* pTask);

#endif

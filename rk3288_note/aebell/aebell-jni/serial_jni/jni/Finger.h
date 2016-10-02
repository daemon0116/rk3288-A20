#ifndef _FINGER_H_
#define _FINGER_H_

#include "Command.h"
#include "SerialPort.h"

//ָ���ǲ���ָ��
typedef enum _FINGERTASKTYPE
{
	emFingerAdd,    //�·�ָ��
	emFingerDel,    //ɾ��ָ��
	emFingerClear,  //���ָ��
	emFingerCheck,  //��ָ֤��
	emFingerSetTimeout,//����¼��ָ�Ƴ�ʱʱ��
} FINGERTASKTYPE;

//ָ���ǲ����ӿڲ���
struct _FINGERTASK
{
	FINGERTASKTYPE type;
	int nId;
	BOOL bOperType;//����Ա����,True�ǻ���
	BYTE* pData;
} __PACKED__;
typedef struct _FINGERTASK FINGERTASK;

typedef struct _CFinger
{
	//����
	void* this;
	int  m_nHandle;
	//����
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
//��ָ����
BOOL FingerOpen(char *filename, int eBaudRate, enDataBits eDataBits, enStopBits eStopBits, enParityCheck eParityCheck);
//�ر�ָ����
BOOL FingerClose(void);

int FingerAdd(int nId, BYTE* pData);
int FingerDelete(int nId);
int FingerClean(void);
int FingerCheck(int *nId);
int FingerSetTimeout(int timeoutVal);

//����ָ����
int FingerDo(FINGERTASK* pTask);

#endif

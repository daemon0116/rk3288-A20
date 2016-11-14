/*****************************************************************

******************************************************************/

#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <pthread.h>
#include <sys/time.h>

#include "TypepDef.h"
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

#define V_ERROR	(-1)
#define V_OK	0

/*****************************************************************
* �궨��
******************************************************************/
#define TP_MAX_NUM          30 //�̳߳�����߳���
#define TP_MAX_STR_SIZE     36 //�߳���Ϣ����buf��󻺴���

/*****************************************************************
* �ṹ����
******************************************************************/
typedef struct {
	BOOL bRun; //�߳����б�ʶ V_TRUE��ʾ�����߳����� V_FALSE��ʾ�������߳�����
	BOOL bActivate; //�����ʶ V_TRUE��ʾ�ѱ����� V_FALSE��ʾû�б�����
	int  nIndex;	//������ʶ
	_pthread_t thrdId; //�߳�ID
	_lock_t thrdMutex; //�̻߳�����
	_cond_t thrdCond; //�߳���������
	pFUNC fnCallback; //�߳�������
	void *dwParam; //fnCallback�Ĳ���
	pFUNC fnErrCallback; //�߳������������
	void *dwErrParam; //fnErrCallback�Ĳ���
	INT nPolicy; //�̵߳��Ȳ��� ȡֵ:SCHED_OTHER CHED_RR SCHED_FIFO
	UINT32 dwPriority; //�߳����ȼ� ��CHED_RR����SCHED_FIFOģʽ����Ч ȡֵ:0-99 ��ֵԽ�����ȼ�Խ��,Ĭ�����ȼ�0

	int nState; //�߳�״̬��ʶ 0��ʾ���� -1��ʾ�̱߳��쳣	1��ʾ�߳������˳�	
	char pcName[TP_MAX_STR_SIZE]; //�߳���������
	UINT32 dwSysThrdId; //ϵͳ������̵߳�ID,���ڲ鿴�߳�cpuʱ��Ƭ��ע���thrdId����ͬһ������
	UINT32 dwCpuTimeStart; //ռ��CPUʱ��Ƭ��ʱ(jiffies)
	UINT32 dwCpuTimeEnd; //ռ��CPUʱ��Ƭ��ʱ(jiffies)
}THREAD_INFO, *pTHREAD_INFO;

/*****************************************************************
* ����ԭ������
******************************************************************/

/*****************************************************************
  Function: 	  �̳߳ع���ģ���ʼ��
  Description:	  �̳߳ع���ģ���ʼ��
  Return:		 �����ɹ����򷵻�0�����򷵻�<0		

******************************************************************/
int InitThrdPl(void);

/*****************************************************************
  Function: 	  �̳߳ع���ģ��ж��
  Description:	  �̳߳ع���ģ��ж��
  Return:		 �����ɹ����򷵻�0�����򷵻�<0		

******************************************************************/
int DelInitThrdPl(void);

/*****************************************************************
  Function:      �̳߳��߳�������亯��
  Description:   ���̳߳�����������̣߳������߳�ִ������
  Input:         fnCallback, �߳�Ҫִ�еĺ����ĵ�ַ��
  				 dwParam, fnCallback�������Ĳ����� 
  				 fnErrCallback,	�߳�Ҫִ�еĺ������������ĵ�ַ��
  				 dwErrParam, fnErrCallback�������Ĳ����� 
		       	 pName,		 �߳������ַ�����
  Output:		 pnIndex,	 �̷߳���ɹ�ʱ���������̳߳ض�Ӧ���߳������š�
  Return:        �����ɹ����򷵻�0�����򷵻�<0      

******************************************************************/
int ThrdPlAlloc(pFUNC fnCallback, void *dwParam, int ParamLen);

/*****************************************************************
  Function:      �̳߳��̹߳�����
  Description:   �����̡߳�
  Input:         nIndex,�̳߳����̵߳������ţ���OSL_TPAlloc���ص�pnIndex��
  Return:        �ɹ���ȡ���������򷵻�0�����򷵻�<0      

******************************************************************/
int ThrdPlSuspend(int nIndex);

/*****************************************************************
  Function:      �̳߳��̻߳��Ѻ���
  Description:   �����̡߳�
  Input:         nIndex,�̳߳����̵߳������ţ���OSL_TPAlloc���ص�pnIndex��
  Return:        �ɹ���ȡ���������򷵻�0�����򷵻�<0      

******************************************************************/
int ThrdPlResume(int nIndex);

/*****************************************************************
  Function:      �̳߳��̵߳������ȼ����ú���
  Description:   �����̳߳��̵߳������ȼ���
  Input:         nIndex,�̳߳����̵߳������ţ���OSL_TPAlloc���ص�pnIndex��
  				 nPolicy, �̵߳��Ȳ��� ȡֵ:SCHED_OTHER SCHED_RR SCHED_FIFO��
  				 dwPriority,�߳����ȼ� ��CHED_RR����SCHED_FIFOģʽ����Ч ȡֵ:0-99 ��ֵԽ�����ȼ�Խ��,Ĭ�����ȼ�0��
  Return:        �����ɹ����򷵻�0�����򷵻�<0      

******************************************************************/
int ThrdPlSetSchedParam(int nIndex,int nPolicy,UINT32 dwPriority);

/*****************************************************************
  Function:      �̳߳��̵߳������ȼ���ȡ����
  Description:   ��ȡ�̳߳��̵߳������ȼ���
  Input:         nIndex,�̳߳����̵߳������ţ���OSL_TPAlloc���ص�pnIndex��
  Output:		 pnPolicy, �̵߳��Ȳ��� ȡֵ:SCHED_OTHER SCHED_RR SCHED_FIFO��
  				 pdwPriority,�߳����ȼ� ��SCHED_RR����SCHED_FIFOģʽ����Ч ȡֵ:0-99 ��ֵԽ�����ȼ�Խ��,Ĭ�����ȼ�0��
  Return:        �����ɹ����򷵻�0�����򷵻�<0      

******************************************************************/
int ThrdPlGetSchedParam(int nIndex,int *pnPolicy,UINT32 *pdwPriority);

/*****************************************************************
  Function:      �̳߳��̼߳���״̬��ȡ����
  Description:   ��ȡ�̳߳��̼߳���״̬��
  Input:         nIndex,�̳߳����̵߳������ţ���OSL_TPAlloc���ص�pnIndex��
  Output:		 pbActivate, �̼߳���״̬��
  Return:        �����ɹ����򷵻�0�����򷵻�<0      

******************************************************************/
int ThrdPlGetActivate(int nIndex, BOOL *pbActivate);


/*****************************************************************
  Function:      �����߳�
  Description:   �򵥷�ʽ�����̡߳�
  Input:         fun:�߳�ִ�к�����p: �̴߳������ָ�롣
  Output:		 ��
  Return:        �����ɹ����򷵻�0�����򷵻�<0      

******************************************************************/
int ThreadCreate(pFUNC fun, void *p);

#ifdef __cplusplus
}
#endif

#endif


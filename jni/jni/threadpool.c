/*****************************************************************

******************************************************************/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "threadpool.h"

#include "log.h"

/*****************************************************************
* �궨��
******************************************************************/


/*****************************************************************
* �ṹ����
******************************************************************/


/*****************************************************************
* ��̬��������
******************************************************************/
static THREAD_INFO sstThrdPool[TP_MAX_NUM];
static _lock_t sthrdPlMutex;



/*****************************************************************
* ��������
******************************************************************/
void *ThrdPlCreate(void *pvParam)
{
    pthread_detach(pthread_self());
    
	int nRet, nIndex;
    
	memcpy(&nIndex, pvParam, sizeof(nIndex));

	if (0 > nIndex || TP_MAX_NUM <= nIndex) //�����ų���
	{
		return (void *)-1;
	}
    
	#if 0
	sstThrdPool[nIndex].dwSysThrdId = syscall(SYS_gettid);
	if (0 == sstThrdPool[nIndex].dwSysThrdId)
	{
		sstThrdPool[nIndex].nState = -1;		
		return (void *)-1;
	}
    #endif
	
	while(sstThrdPool[nIndex].bRun)
	{
		sstThrdPool[nIndex].bActivate = FALSE;
        
		ThrdPlSuspend(nIndex); //�����߳�

		//�߳�������ִ��		
		if ((TRUE == sstThrdPool[nIndex].bActivate) && (NULL != sstThrdPool[nIndex].fnCallback))
		{
			nRet = (int)sstThrdPool[nIndex].fnCallback(sstThrdPool[nIndex].dwParam);
            sstThrdPool[nIndex].fnCallback = NULL;
            
			if (V_OK != nRet)
			{				
				//�߳�������ִ�г�����
				if (NULL != sstThrdPool[nIndex].fnErrCallback)
				{
					sstThrdPool[nIndex].fnErrCallback(sstThrdPool[nIndex].dwErrParam);
                    sstThrdPool[nIndex].fnErrCallback = NULL;
				}
			}
		}
	}

    _lock_destroy(&(sstThrdPool[nIndex].thrdMutex));
    _cond_destroy(&(sstThrdPool[nIndex].thrdCond));

	sstThrdPool[nIndex].nState = 1; //1��ʾ�߳������˳�

    jThreadDetach();
    
	return  (void *)0;
}


/*****************************************************************
  Function: 	  �̳߳ع���ģ���ʼ��
  Description:	  �̳߳ع���ģ���ʼ��
  Return:		 �����ɹ����򷵻�0�����򷵻�<0		

******************************************************************/
int InitThrdPl(void)
{
	int nIndex, nRet, nCnt = 0;
	pthread_attr_t thrdAttr;

    LOGV("1");

	//��ʼ���̳߳ؾ�̬������
	nRet = _lock_init(&sthrdPlMutex, NULL);
	if (0 != nRet)
	{
		return V_ERROR;
	}
	
	//�߳����Գ�ʼ��
	pthread_attr_init(&thrdAttr);
	//�̰߳󶨣�ȷ��cpuʱ��Ƭ���õ�ʵʱ��
	pthread_attr_setscope(&thrdAttr, PTHREAD_SCOPE_SYSTEM);
	//�̷߳��룬ȷ���߳����ٺ���Դ���ϻ���
	//pthread_attr_setdetachstate(&thrdAttr, PTHREAD_CREATE_DETACHED);
	
	for (nIndex = 0; nIndex < TP_MAX_NUM; nIndex++)
	{
		sstThrdPool[nIndex].bRun = TRUE;
		sstThrdPool[nIndex].bActivate = FALSE;
		sstThrdPool[nIndex].nIndex = nIndex;
		sstThrdPool[nIndex].fnCallback = NULL;
		sstThrdPool[nIndex].dwParam = 0;
		sstThrdPool[nIndex].fnErrCallback = NULL;
		sstThrdPool[nIndex].dwErrParam = 0;

		sstThrdPool[nIndex].nState = 0;
		memset(sstThrdPool[nIndex].pcName, 0, sizeof(sstThrdPool[nIndex].pcName));
		sstThrdPool[nIndex].dwSysThrdId = 0;
		sstThrdPool[nIndex].dwCpuTimeStart = 0;
		sstThrdPool[nIndex].dwCpuTimeEnd = 0;

		nRet = _lock_init(&(sstThrdPool[nIndex].thrdMutex), NULL);
		if(0 != nRet)
		{
			sstThrdPool[nIndex].nState = -1;			
			sstThrdPool[nIndex].bRun = FALSE;
			continue;
		}

		nRet = _cond_init(&(sstThrdPool[nIndex].thrdCond), NULL);
		if(0 != nRet)
		{
			sstThrdPool[nIndex].nState = -1;
			sstThrdPool[nIndex].bRun = FALSE;
			continue;
		}

		nRet = pthread_create(&(sstThrdPool[nIndex].thrdId), (void *)&thrdAttr, (void *)ThrdPlCreate, (void *)&nIndex);
        //nRet = pthread_create(&(sstThrdPool[nIndex].thrdId), NULL, (void *)ThrdPlCreate, (void *)&nIndex);
        if(0 != nRet)
		{
			sstThrdPool[nIndex].nState = -1;			
			sstThrdPool[nIndex].bRun = FALSE;
			continue;
		}

        //LOGD("ThrdPlInit nState=%d", sstThrdPool[nIndex].nState);

//        ThrdPlSetSchedParam(nIndex, SCHED_RR, 24);
//        ThrdPlGetSchedParam(nIndex, &(sstThrdPool[nIndex].nPolicy), &(sstThrdPool[nIndex].dwPriority));
		nCnt++;
		msleep(2);
	}
	
	return V_OK;
}

/*****************************************************************
  Function: 	  �̳߳ع���ģ��ж��
  Description:	  �̳߳ع���ģ��ж��
  Return:		 �����ɹ����򷵻�0�����򷵻�<0		

******************************************************************/
int DelInitThrdPl(void)
{
	int nIndex;
	
	//�����˳��߳�
	for (nIndex = 0; nIndex < TP_MAX_NUM; nIndex++)
	{
		if (TRUE == sstThrdPool[nIndex].bRun)
		{
			sstThrdPool[nIndex].bRun = FALSE;
			ThrdPlResume(nIndex);
		}
	}

	sleep(1);

	//��û�������˳����߳̽���ǿ������ 
	for (nIndex = 0; nIndex < TP_MAX_NUM; nIndex++)
	{
		if ((0 == sstThrdPool[nIndex].nState)) //0��ʾ�����̳߳ɹ�������û�������˳��߳�
		{
			ThrdPlCancel(nIndex);
		}
	}

	//�ͷŻ����� �¼�����
	_lock_destroy(&sthrdPlMutex);

    #if 0
	for (nIndex = 0; nIndex < TP_MAX_NUM; nIndex++)
	{
		if (-1 != sstThrdPool[nIndex].nState) //-1��ʾ�߳�����ʼ��ʱ�Ѿ��������û�б�����
		{
			_lock_destroy(&(sstThrdPool[nIndex].thrdMutex));
			_cond_destroy(&(sstThrdPool[nIndex].thrdCond));
		}
	}
	#endif
}

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
int ThrdPlAlloc(pFUNC fnCallback, void *dwParam, int ParamLen)
{
	int nIndex;
		
	_lock(&sthrdPlMutex);
	for (nIndex = 0; nIndex < TP_MAX_NUM; nIndex++)
	{
        if ((FALSE == sstThrdPool[nIndex].bActivate) && (0 == sstThrdPool[nIndex].nState))
		{			
			sstThrdPool[nIndex].bActivate = TRUE;
			sstThrdPool[nIndex].fnCallback = fnCallback;
			sstThrdPool[nIndex].dwParam = dwParam;
			ThrdPlResume(nIndex);
			break;
		}
	}		
	_unlock(&sthrdPlMutex);

    LOGV("ThrdPlAlloc nIndex=%d", nIndex);
	
	if (nIndex >= TP_MAX_NUM)
	{
		return V_ERROR;
	}
	
	return V_OK;
}

/*****************************************************************
  Function:      �̳߳��߳����ٺ���
  Description:   �����̡߳������������̣߳�������ֹ�̡߳�
  Input:         nIndex,�̳߳����̵߳������ţ���OSL_TPAlloc���ص�pnIndex��
  Return:        �����ɹ����򷵻�0�����򷵻�<0      

******************************************************************/
int ThrdPlCancel(int nIndex)
{
	if (0 > nIndex || TP_MAX_NUM <= nIndex) //�����ų���
	{
		return V_ERROR;
	}

	_lock(&sthrdPlMutex);
	//pthread_cancel(sstThrdPool[nIndex].thrdId);//test �˴����������⣬���������
	sstThrdPool[nIndex].nState = -1;
	sstThrdPool[nIndex].bActivate = FALSE;
	sstThrdPool[nIndex].bRun = FALSE;
	_unlock(&sthrdPlMutex);

	return V_OK;
}

/*****************************************************************
  Function:      �̳߳��̹߳�����
  Description:   �����̡߳�
  Input:         nIndex,�̳߳����̵߳������ţ���OSL_TPAlloc���ص�pnIndex��
  Return:        �ɹ���ȡ���������򷵻�0�����򷵻�<0      

******************************************************************/
int ThrdPlSuspend(int nIndex)
{
	if (0 > nIndex || TP_MAX_NUM <= nIndex) //�����ų���
	{
		return V_ERROR;
	}

	_lock(&(sstThrdPool[nIndex].thrdMutex));
	_cond_wait(&(sstThrdPool[nIndex].thrdCond), &(sstThrdPool[nIndex].thrdMutex));
	_unlock(&(sstThrdPool[nIndex].thrdMutex));

	return V_OK;
}

/*****************************************************************
  Function:      �̳߳��̻߳��Ѻ���
  Description:   �����̡߳�
  Input:         nIndex,�̳߳����̵߳������ţ���OSL_TPAlloc���ص�pnIndex��
  Return:        �ɹ���ȡ���������򷵻�0�����򷵻�<0      

******************************************************************/
int ThrdPlResume(int nIndex)
{
	if (0 > nIndex || TP_MAX_NUM <= nIndex) //�����ų���
	{
		return V_ERROR;
	}

	_cond_signal(&(sstThrdPool[nIndex].thrdCond));
	return V_OK;
}

/*****************************************************************
  Function:      �̳߳��̵߳������ȼ����ú���
  Description:   �����̳߳��̵߳������ȼ���
  Input:         nIndex,�̳߳����̵߳������ţ���OSL_TPAlloc���ص�pnIndex��
  				 nPolicy, �̵߳��Ȳ��� ȡֵ:SCHED_OTHER SCHED_RR SCHED_FIFO��
  				 dwPriority,�߳����ȼ� ��CHED_RR����SCHED_FIFOģʽ����Ч ȡֵ:0-99 ��ֵԽ�����ȼ�Խ��,Ĭ�����ȼ�0��
  Return:        �����ɹ����򷵻�0�����򷵻�<0      

******************************************************************/
int ThrdPlSetSchedParam(int nIndex,int nPolicy,UINT32 dwPriority)
{
	int nRet;
	struct sched_param stSchedParm;
	
	if (0 > nIndex || TP_MAX_NUM <= nIndex) //�����ų���
	{		
		return V_ERROR;
	}

	if (!((SCHED_OTHER == nPolicy) || (SCHED_RR == nPolicy) || (SCHED_FIFO == nPolicy)))
	{
		return V_ERROR;
	}

	if ((0 > dwPriority) || (99 < dwPriority))
	{
		return V_ERROR;
	}

	stSchedParm.sched_priority = dwPriority;	
	
	_lock(&sthrdPlMutex);
	nRet = pthread_setschedparam(sstThrdPool[nIndex].thrdId, nPolicy, &stSchedParm);
	if (0 > nRet)
	{
		nRet = V_ERROR;
	}
	else
	{
		sstThrdPool[nIndex].nPolicy = nPolicy;
		sstThrdPool[nIndex].dwPriority = dwPriority;
		
		nRet = V_OK;
	}
	_unlock(&sthrdPlMutex);
	
	return nRet;
}

/*****************************************************************
  Function:      �̳߳��̵߳������ȼ���ȡ����
  Description:   ��ȡ�̳߳��̵߳������ȼ���
  Input:         nIndex,�̳߳����̵߳������ţ���OSL_TPAlloc���ص�pnIndex��
  Output:		 pnPolicy, �̵߳��Ȳ��� ȡֵ:SCHED_OTHER SCHED_RR SCHED_FIFO��
  				 pdwPriority,�߳����ȼ� ��CHED_RR����SCHED_FIFOģʽ����Ч ȡֵ:0-99 ��ֵԽ�����ȼ�Խ��,Ĭ�����ȼ�0��
  Return:        �����ɹ����򷵻�0�����򷵻�<0      

******************************************************************/
int ThrdPlGetSchedParam(int nIndex,int *pnPolicy,UINT32 *pdwPriority)
{
	int nRet;
	struct sched_param stSchedParm;
	
	if (0 > nIndex || TP_MAX_NUM <= nIndex || NULL == pnPolicy || NULL == pdwPriority) //�����ų���
	{
		return V_ERROR;
	}

	_lock(&sthrdPlMutex);
	nRet = pthread_getschedparam(sstThrdPool[nIndex].thrdId, pnPolicy, &stSchedParm);
	if (0 > nRet)
	{
		nRet = V_ERROR;
	}
	else
	{
		*pdwPriority = stSchedParm.sched_priority;
		nRet = V_OK;
	}	
	_unlock(&sthrdPlMutex);
	
	return nRet;
}

/*****************************************************************
  Function:      �̳߳��̼߳���״̬��ȡ����
  Description:   ��ȡ�̳߳��̼߳���״̬��
  Input:         nIndex,�̳߳����̵߳������ţ���OSL_TPAlloc���ص�pnIndex��
  Output:		 pbActivate, �̼߳���״̬��
  Return:        �����ɹ����򷵻�0�����򷵻�<0      

******************************************************************/
int ThrdPlGetActivate(int nIndex, BOOL *pbActivate)
{
	if (0 > nIndex || TP_MAX_NUM <= nIndex) //�����ų���
	{
		return V_ERROR;
	}
	
	if (NULL == pbActivate) //�����Ƿ�
	{
		return V_ERROR;
	}
	
	_lock(&sthrdPlMutex);
	*pbActivate = sstThrdPool[nIndex].bActivate;
	_unlock(&sthrdPlMutex);

	return V_OK;
}

int ThreadCreate(pFUNC fun, void *p)
{
    _pthread_t thread;

    pthread_create(&thread, NULL, fun, p);
    pthread_detach(thread);
}


/*****************************************************************

******************************************************************/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "threadpool.h"

#include "log.h"

/*****************************************************************
* 宏定义
******************************************************************/


/*****************************************************************
* 结构定义
******************************************************************/


/*****************************************************************
* 静态变量定义
******************************************************************/
static THREAD_INFO sstThrdPool[TP_MAX_NUM];
static _lock_t sthrdPlMutex;



/*****************************************************************
* 函数定义
******************************************************************/
void *ThrdPlCreate(void *pvParam)
{
    pthread_detach(pthread_self());
    
	int nRet, nIndex;
    
	memcpy(&nIndex, pvParam, sizeof(nIndex));

	if (0 > nIndex || TP_MAX_NUM <= nIndex) //索引号出错
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
        
		ThrdPlSuspend(nIndex); //挂起线程

		//线程任务函数执行		
		if ((TRUE == sstThrdPool[nIndex].bActivate) && (NULL != sstThrdPool[nIndex].fnCallback))
		{
			nRet = (int)sstThrdPool[nIndex].fnCallback(sstThrdPool[nIndex].dwParam);
            sstThrdPool[nIndex].fnCallback = NULL;
            
			if (V_OK != nRet)
			{				
				//线程任务函数执行出错处理
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

	sstThrdPool[nIndex].nState = 1; //1表示线程正常退出

    jThreadDetach();
    
	return  (void *)0;
}


/*****************************************************************
  Function: 	  线程池管理模块初始化
  Description:	  线程池管理模块初始化
  Return:		 操作成功，则返回0，无则返回<0		

******************************************************************/
int InitThrdPl(void)
{
	int nIndex, nRet, nCnt = 0;
	pthread_attr_t thrdAttr;

    LOGV("1");

	//初始化线程池静态互斥锁
	nRet = _lock_init(&sthrdPlMutex, NULL);
	if (0 != nRet)
	{
		return V_ERROR;
	}
	
	//线程属性初始化
	pthread_attr_init(&thrdAttr);
	//线程绑定，确保cpu时间片调用的实时性
	pthread_attr_setscope(&thrdAttr, PTHREAD_SCOPE_SYSTEM);
	//线程分离，确保线程销毁后资源马上回收
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
  Function: 	  线程池管理模块卸载
  Description:	  线程池管理模块卸载
  Return:		 操作成功，则返回0，无则返回<0		

******************************************************************/
int DelInitThrdPl(void)
{
	int nIndex;
	
	//正常退出线程
	for (nIndex = 0; nIndex < TP_MAX_NUM; nIndex++)
	{
		if (TRUE == sstThrdPool[nIndex].bRun)
		{
			sstThrdPool[nIndex].bRun = FALSE;
			ThrdPlResume(nIndex);
		}
	}

	sleep(1);

	//对没有正常退出的线程进行强行销毁 
	for (nIndex = 0; nIndex < TP_MAX_NUM; nIndex++)
	{
		if ((0 == sstThrdPool[nIndex].nState)) //0表示创建线程成功，但是没有正常退出线程
		{
			ThrdPlCancel(nIndex);
		}
	}

	//释放互斥锁 事件条件
	_lock_destroy(&sthrdPlMutex);

    #if 0
	for (nIndex = 0; nIndex < TP_MAX_NUM; nIndex++)
	{
		if (-1 != sstThrdPool[nIndex].nState) //-1表示线程做初始化时已经出错，因此没有被创建
		{
			_lock_destroy(&(sstThrdPool[nIndex].thrdMutex));
			_cond_destroy(&(sstThrdPool[nIndex].thrdCond));
		}
	}
	#endif
}

/*****************************************************************
  Function:      线程池线程请求分配函数
  Description:   从线程池中请求分配线程，启动线程执行任务。
  Input:         fnCallback, 线程要执行的函数的地址。
  				 dwParam, fnCallback任务函数的参数。 
  				 fnErrCallback,	线程要执行的函数出错处理函数的地址。
  				 dwErrParam, fnErrCallback任务函数的参数。 
		       	 pName,		 线程名称字符串。
  Output:		 pnIndex,	 线程分配成功时，传出的线程池对应的线程索引号。
  Return:        操作成功，则返回0，无则返回<0      

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
  Function:      线程池线程销毁函数
  Description:   销毁线程。非阻塞调用线程，马上终止线程。
  Input:         nIndex,线程池中线程的索引号，由OSL_TPAlloc返回的pnIndex。
  Return:        操作成功，则返回0，无则返回<0      

******************************************************************/
int ThrdPlCancel(int nIndex)
{
	if (0 > nIndex || TP_MAX_NUM <= nIndex) //索引号出错
	{
		return V_ERROR;
	}

	_lock(&sthrdPlMutex);
	//pthread_cancel(sstThrdPool[nIndex].thrdId);//test 此处编译有问题，后面需改正
	sstThrdPool[nIndex].nState = -1;
	sstThrdPool[nIndex].bActivate = FALSE;
	sstThrdPool[nIndex].bRun = FALSE;
	_unlock(&sthrdPlMutex);

	return V_OK;
}

/*****************************************************************
  Function:      线程池线程挂起函数
  Description:   挂起线程。
  Input:         nIndex,线程池中线程的索引号，由OSL_TPAlloc返回的pnIndex。
  Return:        成功获取互斥锁，则返回0，无则返回<0      

******************************************************************/
int ThrdPlSuspend(int nIndex)
{
	if (0 > nIndex || TP_MAX_NUM <= nIndex) //索引号出错
	{
		return V_ERROR;
	}

	_lock(&(sstThrdPool[nIndex].thrdMutex));
	_cond_wait(&(sstThrdPool[nIndex].thrdCond), &(sstThrdPool[nIndex].thrdMutex));
	_unlock(&(sstThrdPool[nIndex].thrdMutex));

	return V_OK;
}

/*****************************************************************
  Function:      线程池线程唤醒函数
  Description:   唤醒线程。
  Input:         nIndex,线程池中线程的索引号，由OSL_TPAlloc返回的pnIndex。
  Return:        成功获取互斥锁，则返回0，无则返回<0      

******************************************************************/
int ThrdPlResume(int nIndex)
{
	if (0 > nIndex || TP_MAX_NUM <= nIndex) //索引号出错
	{
		return V_ERROR;
	}

	_cond_signal(&(sstThrdPool[nIndex].thrdCond));
	return V_OK;
}

/*****************************************************************
  Function:      线程池线程调度优先级设置函数
  Description:   设置线程池线程调度优先级。
  Input:         nIndex,线程池中线程的索引号，由OSL_TPAlloc返回的pnIndex。
  				 nPolicy, 线程调度策略 取值:SCHED_OTHER SCHED_RR SCHED_FIFO。
  				 dwPriority,线程优先级 在CHED_RR或者SCHED_FIFO模式下生效 取值:0-99 数值越大优先级越高,默认优先级0。
  Return:        操作成功，则返回0，无则返回<0      

******************************************************************/
int ThrdPlSetSchedParam(int nIndex,int nPolicy,UINT32 dwPriority)
{
	int nRet;
	struct sched_param stSchedParm;
	
	if (0 > nIndex || TP_MAX_NUM <= nIndex) //索引号出错
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
  Function:      线程池线程调度优先级获取函数
  Description:   获取线程池线程调度优先级。
  Input:         nIndex,线程池中线程的索引号，由OSL_TPAlloc返回的pnIndex。
  Output:		 pnPolicy, 线程调度策略 取值:SCHED_OTHER SCHED_RR SCHED_FIFO。
  				 pdwPriority,线程优先级 在CHED_RR或者SCHED_FIFO模式下生效 取值:0-99 数值越大优先级越高,默认优先级0。
  Return:        操作成功，则返回0，无则返回<0      

******************************************************************/
int ThrdPlGetSchedParam(int nIndex,int *pnPolicy,UINT32 *pdwPriority)
{
	int nRet;
	struct sched_param stSchedParm;
	
	if (0 > nIndex || TP_MAX_NUM <= nIndex || NULL == pnPolicy || NULL == pdwPriority) //索引号出错
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
  Function:      线程池线程激活状态获取函数
  Description:   获取线程池线程激活状态。
  Input:         nIndex,线程池中线程的索引号，由OSL_TPAlloc返回的pnIndex。
  Output:		 pbActivate, 线程激活状态。
  Return:        操作成功，则返回0，无则返回<0      

******************************************************************/
int ThrdPlGetActivate(int nIndex, BOOL *pbActivate)
{
	if (0 > nIndex || TP_MAX_NUM <= nIndex) //索引号出错
	{
		return V_ERROR;
	}
	
	if (NULL == pbActivate) //参数非法
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


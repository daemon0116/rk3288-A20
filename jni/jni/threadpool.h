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
* 宏定义
******************************************************************/
#define TP_MAX_NUM          30 //线程池最大线程数
#define TP_MAX_STR_SIZE     36 //线程信息参数buf最大缓存量

/*****************************************************************
* 结构定义
******************************************************************/
typedef struct {
	BOOL bRun; //线程运行标识 V_TRUE表示允许线程运行 V_FALSE表示不允许线程运行
	BOOL bActivate; //分配标识 V_TRUE表示已被分配 V_FALSE表示没有被分配
	int  nIndex;	//索引标识
	_pthread_t thrdId; //线程ID
	_lock_t thrdMutex; //线程互斥锁
	_cond_t thrdCond; //线程事情条件
	pFUNC fnCallback; //线程任务函数
	void *dwParam; //fnCallback的参数
	pFUNC fnErrCallback; //线程任务出错处理函数
	void *dwErrParam; //fnErrCallback的参数
	INT nPolicy; //线程调度策略 取值:SCHED_OTHER CHED_RR SCHED_FIFO
	UINT32 dwPriority; //线程优先级 在CHED_RR或者SCHED_FIFO模式下生效 取值:0-99 数值越大优先级越高,默认优先级0

	int nState; //线程状态标识 0表示正常 -1表示线程被异常	1表示线程正常退出	
	char pcName[TP_MAX_STR_SIZE]; //线程任务名称
	UINT32 dwSysThrdId; //系统分配给线程的ID,用于查看线程cpu时间片。注意和thrdId不是同一个概念
	UINT32 dwCpuTimeStart; //占用CPU时间片耗时(jiffies)
	UINT32 dwCpuTimeEnd; //占用CPU时间片耗时(jiffies)
}THREAD_INFO, *pTHREAD_INFO;

/*****************************************************************
* 函数原型声明
******************************************************************/

/*****************************************************************
  Function: 	  线程池管理模块初始化
  Description:	  线程池管理模块初始化
  Return:		 操作成功，则返回0，无则返回<0		

******************************************************************/
int InitThrdPl(void);

/*****************************************************************
  Function: 	  线程池管理模块卸载
  Description:	  线程池管理模块卸载
  Return:		 操作成功，则返回0，无则返回<0		

******************************************************************/
int DelInitThrdPl(void);

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
int ThrdPlAlloc(pFUNC fnCallback, void *dwParam, int ParamLen);

/*****************************************************************
  Function:      线程池线程挂起函数
  Description:   挂起线程。
  Input:         nIndex,线程池中线程的索引号，由OSL_TPAlloc返回的pnIndex。
  Return:        成功获取互斥锁，则返回0，无则返回<0      

******************************************************************/
int ThrdPlSuspend(int nIndex);

/*****************************************************************
  Function:      线程池线程唤醒函数
  Description:   唤醒线程。
  Input:         nIndex,线程池中线程的索引号，由OSL_TPAlloc返回的pnIndex。
  Return:        成功获取互斥锁，则返回0，无则返回<0      

******************************************************************/
int ThrdPlResume(int nIndex);

/*****************************************************************
  Function:      线程池线程调度优先级设置函数
  Description:   设置线程池线程调度优先级。
  Input:         nIndex,线程池中线程的索引号，由OSL_TPAlloc返回的pnIndex。
  				 nPolicy, 线程调度策略 取值:SCHED_OTHER SCHED_RR SCHED_FIFO。
  				 dwPriority,线程优先级 在CHED_RR或者SCHED_FIFO模式下生效 取值:0-99 数值越大优先级越高,默认优先级0。
  Return:        操作成功，则返回0，无则返回<0      

******************************************************************/
int ThrdPlSetSchedParam(int nIndex,int nPolicy,UINT32 dwPriority);

/*****************************************************************
  Function:      线程池线程调度优先级获取函数
  Description:   获取线程池线程调度优先级。
  Input:         nIndex,线程池中线程的索引号，由OSL_TPAlloc返回的pnIndex。
  Output:		 pnPolicy, 线程调度策略 取值:SCHED_OTHER SCHED_RR SCHED_FIFO。
  				 pdwPriority,线程优先级 在SCHED_RR或者SCHED_FIFO模式下生效 取值:0-99 数值越大优先级越高,默认优先级0。
  Return:        操作成功，则返回0，无则返回<0      

******************************************************************/
int ThrdPlGetSchedParam(int nIndex,int *pnPolicy,UINT32 *pdwPriority);

/*****************************************************************
  Function:      线程池线程激活状态获取函数
  Description:   获取线程池线程激活状态。
  Input:         nIndex,线程池中线程的索引号，由OSL_TPAlloc返回的pnIndex。
  Output:		 pbActivate, 线程激活状态。
  Return:        操作成功，则返回0，无则返回<0      

******************************************************************/
int ThrdPlGetActivate(int nIndex, BOOL *pbActivate);


/*****************************************************************
  Function:      创建线程
  Description:   简单方式创建线程。
  Input:         fun:线程执行函数，p: 线程传入参数指针。
  Output:		 无
  Return:        操作成功，则返回0，无则返回<0      

******************************************************************/
int ThreadCreate(pFUNC fun, void *p);

#ifdef __cplusplus
}
#endif

#endif


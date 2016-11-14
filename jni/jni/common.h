/*****************************************************************

******************************************************************/

#ifndef __COMMON_H__
#define __COMMON_H__

#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>

#include "log.h"
#include "TypepDef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define msleep(m)       usleep(m*1000)


//#define MOMMON_TEST     //调试期间如需观察内存、互斥锁、信号量等的申请释放情况则定义MOMMON_TEST
#ifdef MOMMON_TEST

#define _malloc         test_malloc
#define _free           test_free

#define _lock_init      test_lock_init
#define _lock_destroy   test_lock_destroy

#define _sem_init       test_sem_init
#define _sem_destroy    test_sem_destroy

#define _cond_init      test_cond_init
#define _cond_destroy   test_cond_destroy

#else
#define _malloc         malloc
#define _free           free

#define _lock_init      pthread_mutex_init
#define _lock_destroy   pthread_mutex_destroy

#define _sem_init       sem_init
#define _sem_destroy    sem_destroy

#define _cond_init      pthread_cond_init
#define _cond_destroy   pthread_cond_destroy

#endif

#define _pthread_t      pthread_t

#define _lock_t         pthread_mutex_t
#define _lock           pthread_mutex_lock
#define _unlock         pthread_mutex_unlock

#define _sem_t          sem_t
#define _sem_post       sem_post 
#define _sem_wait       sem_wait

#define _cond_t         pthread_cond_t
#define _cond_signal    pthread_cond_signal
#define _cond_wait      pthread_cond_wait


void common_test_printf(void);


char *_inet_ntoa(uint32 addr);


#ifdef __cplusplus
}
#endif


#endif


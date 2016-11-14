
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "common.h"



#ifdef MOMMON_TEST
static int test_alloc_max = 0;
static int test_alloc_cnt = 0;
static int test_lock_cnt = 0;
static int test_sem_cnt = 0;
static int test_cond_cnt = 0;

void *test_malloc(int size)
{
    char *p = (char *)malloc(size + 4);
    if(!p){
        return NULL;
    }
    test_alloc_cnt += size;
    if(test_alloc_cnt > test_alloc_max){
        test_alloc_max = test_alloc_cnt;
        //LOGD("test_alloc_max ======= %d", test_alloc_max);
    }
    *(int *)p = size;
    return p + sizeof(int);
}

void test_free(void *p)
{
    char *_p = p;
    if(_p){
        _p -= sizeof(int);
        test_alloc_cnt -= *(int *)_p;
        free(_p);
    }
}

int test_lock_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
    int ret;

    ret = pthread_mutex_init(mutex, attr);
    if(ret == 0)
    {
        test_lock_cnt ++;
    }
    return ret;
}
int test_lock_destroy(pthread_mutex_t *mutex)
{
    int ret;

    ret = pthread_mutex_destroy(mutex);
    if(ret == 0)
    {
        test_lock_cnt --;
    }
    return ret;   
}

int test_sem_init(sem_t *sem, int pshared, unsigned int value)
{
    int ret;
    
    ret = sem_init(sem, pshared, value);
    if(ret == 0)
    {
        test_sem_cnt ++;
    }
    return ret;
}

int test_sem_destroy(sem_t *sem)
{
    int ret;
    
    ret = sem_destroy(sem);
    if(ret == 0)
    {
        test_sem_cnt --;
    }
    return ret;
}


int test_cond_init(pthread_cond_t *cond, pthread_condattr_t *cond_attr)
{
    int ret;
    
    ret = pthread_cond_init(cond, cond_attr);
    if(ret == 0)
    {
        test_cond_cnt ++;
    }
    return ret;
}

int test_cond_destroy(pthread_cond_t *cond)
{
    int ret;
    
    ret = pthread_cond_destroy(cond);
    if(ret == 0){
        test_cond_cnt --;
    }
    return ret;
}

void common_test_printf(void)
{
    LOGD("test_alloc_max ======= %d", test_alloc_max);
    LOGD("test_alloc_cnt ======= %d", test_alloc_cnt);
    LOGD("test_lock_cnt  ======= %d", test_lock_cnt);
    LOGD("test_sem_cnt   ======= %d", test_sem_cnt);
    LOGD("test_cond_cnt  ======= %d", test_cond_cnt);
    test_alloc_max = 0;
    test_alloc_cnt = 0;
    test_lock_cnt = 0;
    test_sem_cnt = 0;
    test_cond_cnt = 0;
}

#endif



char *_inet_ntoa(uint32 addr)
{
    struct in_addr in_addr;
    
    in_addr.s_addr = addr;
    return inet_ntoa(in_addr);
}
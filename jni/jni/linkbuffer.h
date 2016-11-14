/*****************************************************************

******************************************************************/

#ifndef __LINK_BUFFER_H__
#define __LINK_BUFFER_H__

#include <pthread.h>
#include <semaphore.h> 

#include "common.h"
#include "TypepDef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    void *next;
    int DataSize;
}LINK_HEADER, *pLINK_HEADER;

typedef struct
{
    BOOL isInit;
    int DefSize;
    //int NodeCnt;
    pLINK_HEADER start;
    pLINK_HEADER wr;
    pLINK_HEADER rd;
    _sem_t sem;
    _lock_t lock;
}LINK, *pLINK;


int LinkAddData(pLINK pLink, const char *data, int size);
int LinkGetData(pLINK pLink, char *data, int size);
int CleanLink(pLINK pLink);
int CreateLink(pLINK pLink, int DefSize);
int DelLink(pLINK pLink);


#ifdef __cplusplus
}
#endif


#endif


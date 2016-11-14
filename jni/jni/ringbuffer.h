#ifndef _lcthw_RingBuffer_h
#define _lcthw_RingBuffer_h

#include <pthread.h>
#include <semaphore.h> 

#include "common.h"
#include "TypepDef.h"


#ifndef 	DYNAMIC_REQ_MEM
#define 	DYNAMIC_REQ_MEM		1//动态申请
#endif
#ifndef 	STATIC_REQ_MEM
#define 	STATIC_REQ_MEM		0//静态申请
#endif

#define     OVER_WRITE_OLD_DATA 2   // 0:不覆盖旧数据, 1:覆盖旧数据, 2:空间不足不写入

#define	buf_lock(x)			_lock(&x->mutex)
#define	buf_unlock(x)		_unlock(&x->mutex)


typedef struct {
    char *buffer;
    int length;
    int rd;
    int wr;
	int flag;//标志是否是动态或静态申请内存
	_lock_t mutex;
    _sem_t sem;
} RingBuffer, *pRingBuffer;

RingBuffer *RingBuffCreate(char *pData,int length,int Flag);

void RingBuffDestroy(RingBuffer *buffer);

void RingBufferClean(RingBuffer *buffer);

int RingBuffRead(RingBuffer *buffer, char *target, int amount);

int RingBuffWrite(RingBuffer *buffer, char *data, int length);

int RingBuffEmpty(RingBuffer *buffer);

int RingBuffer_full(RingBuffer *buffer);

int RingBuffAvaiData(RingBuffer *buffer);

int RingBuffAvaiSpaData(RingBuffer *buffer);

/*
 * 组成一个环,start等于end时为空; start在end后面一个时为满;
 * 缓冲区大小为length - 1, 留一个区分空与满
 */

#define RingBuffSize(B) ((B)->length-1)

#define RingBuffAvaiData(B) (((B)->wr >= (B)->rd) ? ((B)->wr - (B)->rd) : ((B)->length - (B)->rd + (B)->wr))

#define RingBuffAvaiSpaData(B)  (RingBuffSize(B) - RingBuffAvaiData(B))

#define RingBufferFull(B) (RingBuffAvaiData((B)) == RingBuffSize(B))

#define RingBuffEmpty(B) ((B)->rd == (B)->wr)

#define RingBufferStartsAt(B) ((B)->buffer + (B)->rd)

#define RingBufferEndsAt(B) ((B)->buffer + (B)->wr)

#define RingBufferCommitRead(B, A) ((B)->rd = ((B)->rd + (A)) % (B)->length)

#define RingBufferCommitWrite(B, A) ((B)->wr = ((B)->wr + (A)) % (B)->length)

#define	GetRingBuffCurReadOffset(B)((B)->rd)

#define	GetRingBuffWriteOffset(B)((B)->wr)

#define SetRingBuffWriteOffset(B,A)((B)->wr = A)

#define SetRingBuffReadOffset(B,A)((B)->rd = A)

#define CountRingBuffInterval(A,B,C)(B<=C ? (C-B):((A)->length-(B-C)))



int WriteAudioData(pRingBuffer pbuf, CHAR *Data, UINT16 DataSize, int *AvaDataSize);
UINT32 GetAudioDataClr(pRingBuffer pbuf, CHAR *Data, UINT16 DataSize);
UINT32 GetAudioData(pRingBuffer pbuf, CHAR *Data, UINT16 DataSize);
UINT32 GetAudioDataAvaData(pRingBuffer pbuf);
UINT32 GetAudioBuffSize(pRingBuffer pbuf);

UINT32 GetNetWorkAvaData(pRingBuffer pbuf);
UINT32 GetAudioDataAvaSpaData(pRingBuffer pbuf);
UINT32 GetRingBuffNetData(pRingBuffer pbuf, CHAR *Data, UINT16 DataSize);


typedef struct {
    char *buffer;
    int cnt;
    int rd;
    int wr;
    int BufNum;
	int BufSize;
    int length;
	_lock_t mutex;
    _sem_t sem;
}BufferRing, *pBufferRing;

pBufferRing BufferRingCreate(int BufSize,int BufNum);
void BufferRingDestroy(pBufferRing pbuf);
char *BufferRingGetBuf(pBufferRing pbuf);
void BufferRingPush(pBufferRing pbuf, char *data, int len);
int BufferRingGetData(pBufferRing pbuf, char **pdata);
int BufferRingReadData(pBufferRing pbuf, char *pdata, int size);
void BufferRingPoll(pBufferRing pbuf);
void BufferRingClean(pBufferRing pbuf);


#endif

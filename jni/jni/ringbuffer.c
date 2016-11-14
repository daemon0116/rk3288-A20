
#include <stdlib.h>
#include <string.h>
#include "ringbuffer.h"

#include "log.h"

#if 1
RingBuffer *RingBuffCreate(char *pData,int length,int Flag)
{
    RingBuffer *buffer = (RingBuffer *)_malloc(sizeof(RingBuffer));
    if(!buffer){
        return NULL;
    }
    bzero(buffer, sizeof(RingBuffer));
    buffer->length  = length+1;
    buffer->rd = 0;
    buffer->wr = 0;
	buffer->flag = Flag;

	if(Flag == DYNAMIC_REQ_MEM)
	{
    	buffer->buffer = (char *)_malloc(buffer->length + 1);
        if(!buffer->buffer){
            goto err;
        }
	}
	else
	{
		if(pData == NULL)
		{
			goto err;
		}
		buffer->buffer = pData;
	}

    _lock_init(&buffer->mutex,NULL);
    _sem_init(&buffer->sem, 0, 0);     

    return buffer;

    err:
    if(buffer->buffer){
        _free(buffer->buffer);
    }
    if(buffer){
        _free(buffer);
    }
    return NULL;
}

void RingBuffDestroy(RingBuffer *buffer)
{
    if(buffer)
	{
        _sem_destroy(&buffer->sem);
        _lock_destroy(&buffer->mutex);
        
		//��̬������ͷ�����
		if(buffer->flag == DYNAMIC_REQ_MEM)
		{
            if(buffer->buffer){
        	    _free(buffer->buffer);
            }
		}
        _free(buffer);
    }
}

void RingBufferClean(RingBuffer *buffer)
{
    buffer->rd = 0;
    buffer->wr = 0;
}

int RingBuffWrite1(RingBuffer *buffer, char *data, int length)
{
	int i;
	int available_space;

	#if (OVER_WRITE_OLD_DATA == 0)
	available_space = RingBuffAvaiSpaData(buffer);
	if(length > available_space){
		length = available_space;
	}
	#endif
	
    //д����
    for(i = 0; i < length; i++)//
    {
        buffer->buffer[buffer->wr] = data[i];
        if(++buffer->wr >= buffer->length){
            buffer->wr = 0;
        }
		#if (OVER_WRITE_OLD_DATA != 0)
		if(buffer->wr == buffer->rd){
			if(++buffer->rd >= buffer->length){
				buffer->rd = 0;
			}
		}
		#endif
    }

	return length;
}


int RingBuffWrite(RingBuffer *buffer, char *data, int length)
{
	int i;
    int len;
	int available_space;

#if (OVER_WRITE_OLD_DATA == 0 || OVER_WRITE_OLD_DATA == 2)
	available_space = RingBuffAvaiSpaData(buffer);
	if(length > available_space){
        #if (OVER_WRITE_OLD_DATA == 2)
        return 0;
        #endif
		length = available_space;
	}
#else
    if(length > RingBuffSize(buffer)){
        data += (length - RingBuffSize(buffer));
        length = RingBuffSize(buffer);
    }
    available_space = RingBuffAvaiSpaData(buffer);
    if(length > available_space){
        buffer->rd += (buffer->wr+length+1);
        if(buffer->rd >= buffer->length){
            buffer->rd -= buffer->length;
        }
    }
#endif
    len = buffer->length - buffer->wr;
    if(length <= len){
        memcpy(&buffer->buffer[buffer->wr], data, length);
    }
    else{
        memcpy(&buffer->buffer[buffer->wr], data, len);
        memcpy(&buffer->buffer[0], data+len, length-len);
    }
    buffer->wr += length;
    if(buffer->wr >= buffer->length){
        buffer->wr -= buffer->length;
    }

	return length;
}

int RingBuffRead1(RingBuffer *buffer, char *target, int amount)
{
    int i;
	int available_data;

	available_data = RingBuffAvaiData(buffer);
    if(amount > available_data)
    {
        //Ҫ��ȡ�����ݸ���������Ч���ݸ���,��ֻ��ȡ��Ч���ݸ���
        amount = available_data;
    }

    for(i=0; i < amount; i++)
    {
        target[i] = buffer->buffer[buffer->rd++];
        if(buffer->rd >= buffer->length)
        {
            buffer->rd = 0;
        }
    }
    return amount;//����ʵ�ʶ�ȡ�����ݸ���

}


int RingBuffRead(RingBuffer *buffer, char *target, int amount)
{
    int i;
    int len;
	int available_data;

	available_data = RingBuffAvaiData(buffer);
    if(amount > available_data)
    {
        //Ҫ��ȡ�����ݸ���������Ч���ݸ���,��ֻ��ȡ��Ч���ݸ���
        amount = available_data;
    }

    len = buffer->length - buffer->rd;
    if(amount <= len){
        memcpy(target, &buffer->buffer[buffer->rd], amount);
    }
    else{
        memcpy(target,     &buffer->buffer[buffer->rd], len);
        memcpy(target+len, &buffer->buffer[0], amount-len);
    }
    buffer->rd += amount;
    if(buffer->rd >= buffer->length){
        buffer->rd -= buffer->length;
    }
    
    return amount;//����ʵ�ʶ�ȡ�����ݸ���
}

int RingBuffReadClr(RingBuffer *buffer, char *target, int amount)
{
    int i;
	int available_data;

	available_data = RingBuffAvaiData(buffer);
    if(amount > available_data)
    {
        //Ҫ��ȡ�����ݸ���������Ч���ݸ���,��ֻ��ȡ��Ч���ݸ���
        amount = available_data;
    }

    for(i = 0; i < amount; i++)
    {
        target[i] = buffer->buffer[buffer->rd];
		buffer->buffer[buffer->rd++] = 0;
        if(buffer->rd >= buffer->length)
        {
            buffer->rd = 0;
        }
    }
    return amount;//����ʵ�ʶ�ȡ�����ݸ���

}

int RingBuffSetValue(RingBuffer *buffer, int start, int end, char value)
{
    int addr;

	if(start>=buffer->length || end>=buffer->length){
		return -1;
	}

    for(addr=start; addr!=end;)
    {
		buffer->buffer[addr] = value;
        if(++addr >= buffer->length)
        {
            addr = 0;
        }
    }
    return 0;
}
#endif

#if 1

pBufferRing BufferRingCreate(int BufSize, int BufNum)
{
    pBufferRing pbuf = (pBufferRing)_malloc(sizeof(BufferRing));
    if(!pbuf){
        return NULL;
    }
    bzero(pbuf, sizeof(BufferRing));
    pbuf->cnt = 0;
    pbuf->rd = 0;
    pbuf->wr = 0;
    pbuf->BufNum = BufNum;
	pbuf->BufSize = BufSize+sizeof(int);
    pbuf->length  = pbuf->BufNum*pbuf->BufSize;

    pbuf->buffer = (char *)_malloc(pbuf->length);
    if(!pbuf->buffer){
        goto err;
    }

    _lock_init(&pbuf->mutex,NULL);
    _sem_init(&pbuf->sem, 0, 0);     

    return pbuf;

    err:
    if(pbuf->buffer){
        _free(pbuf->buffer);
    }
    if(pbuf){
        _free(pbuf);
    }
    return NULL;
}

void BufferRingDestroy(pBufferRing pbuf)
{
    if(pbuf)
    {
        _sem_destroy(&pbuf->sem);
        _lock_destroy(&pbuf->mutex);
        if(pbuf->buffer){
            _free(pbuf->buffer);
        }
        if(pbuf){
            _free(pbuf);
        }
    }
}

char *BufferRingGetBuf(pBufferRing pbuf)
{
    char *p;

    if(!pbuf){
        return;
    }

    buf_lock(pbuf);
    if(pbuf->cnt >= pbuf->BufNum){
        buf_unlock(pbuf);
        return NULL;
    }
    pbuf->wr += pbuf->BufSize;
    if(pbuf->wr >= pbuf->length){
        pbuf->wr = 0;
    }
    p = (pbuf->buffer+pbuf->wr+sizeof(int));
    buf_unlock(pbuf);
    
    return p;
}

void BufferRingPush(pBufferRing pbuf, char *data, int len)
{
    if(!pbuf){
        return;
    }

    buf_lock(pbuf);
    *(int *)(data-sizeof(int)) = len;
    pbuf->cnt++;
    if(pbuf->cnt<=1 || pbuf->cnt>=pbuf->BufNum){
        _sem_post(&pbuf->sem);
    }
    buf_unlock(pbuf);
}

int BufferRingGetData(pBufferRing pbuf, char **pdata)
{
    int len;

    if(!pbuf){
        return;
    }

    buf_lock(pbuf);
    if(pbuf->cnt <= 0){
        buf_unlock(pbuf);
        return -1;
    }
    pbuf->rd += pbuf->BufSize;
    if(pbuf->rd >= pbuf->length){
        pbuf->rd = 0;
    }
    *pdata = pbuf->buffer+pbuf->rd+sizeof(int);
    len = *(int *)(pbuf->buffer+pbuf->rd);
    buf_unlock(pbuf);

    return len;
}


int BufferRingReadData(pBufferRing pbuf, char *pdata, int size)
{
    int len;
    
    if(!pbuf || !pdata){
        return;
    }

    buf_lock(pbuf);
    if(pbuf->cnt <= 0){
        buf_unlock(pbuf);
        return 0;
    }
    pbuf->rd += pbuf->BufSize;
    if(pbuf->rd >= pbuf->length){
        pbuf->rd = 0;
    }
    len = *(int *)(pbuf->buffer+pbuf->rd);
    if(len > size){
        len = size;
    }
    memcpy(pdata, pbuf->buffer+pbuf->rd+sizeof(int), len);
    
    pbuf->cnt--;

    buf_unlock(pbuf);

    return len;
}


void BufferRingPoll(pBufferRing pbuf)
{
    if(!pbuf){
        return;
    }
    buf_lock(pbuf);
    pbuf->cnt--;
    buf_unlock(pbuf);
}

void BufferRingClean(pBufferRing pbuf)
{
    if(!pbuf){
        return;
    }
    buf_lock(pbuf);
    pbuf->cnt = 0;
    pbuf->rd = 0;
    pbuf->wr = 0;
    buf_unlock(pbuf);
}

#endif


#if 1
/*******************************************************************************
���⣺GetAudioDataNetData
���ܣ���ȡ��Ƶ��Ч����
��ʽ��UINT32 GetAudioDataAvaData(CHAR *Data, UINT16 DataSize)
���룺Data-->���ݴ��ָ��	DataSize-->���ݳ���
�����
����ֵ:ʵ����Ч����
�쳣����û�з���
*******************************************************************************/
UINT32 GetAudioDataAvaData(pRingBuffer pbuf)
{
	UINT8		RtnVal;
	UINT32		Ret;

	if(NULL == pbuf)
	{
		return 0;
	}
	//����
	buf_lock(pbuf);
	Ret = RingBuffAvaiData(pbuf);
	//����
	buf_unlock(pbuf);
	return Ret;


}

/*******************************************************************************
���⣺GetAudioDataAvaSpaData
���ܣ���ȡ��Ƶʣ��ռ�
��ʽ��UINT32 GetAudioDataAvaSpaData(CHAR *Data, UINT16 DataSize)
���룺Data-->���ݴ��ָ��	DataSize-->���ݳ���
�����
����ֵ:ʵ����Ч����
�쳣����û�з���
*******************************************************************************/
UINT32 GetAudioDataAvaSpaData(pRingBuffer pbuf)
{
	UINT8		RtnVal;
	UINT32		Ret;

	if(NULL == pbuf)
	{
		return 0;
	}
	//����
	buf_lock(pbuf);
	Ret = RingBuffAvaiSpaData(pbuf);
	//����
	buf_unlock(pbuf);
	return Ret;


}


/*******************************************************************************
���⣺WriteAudioDataNetData
���ܣ�д���ݵ���Ƶ����--���ڲ��š�ϵͳ�����ͶԽ�����
��ʽ��void WriteAudioData(CHAR *Data, UINT16 DataSize)
���룺Data-->�Ӹ�����ָ���������	DataSize-->���ݳ���
�����
����ֵ:ʵ����Ч����
�쳣����û�з���
*******************************************************************************/
int WriteAudioData(pRingBuffer pbuf, CHAR *Data, UINT16 DataSize, int *AvaDataSize)
{
    int len;

	if(NULL == pbuf)
	{
		return 0;
	}

	//����
	buf_lock(pbuf);
	//д�����ݵ����λ�����
	len = RingBuffWrite(pbuf,Data,DataSize);
    if(AvaDataSize){
        *AvaDataSize = RingBuffAvaiData(pbuf);
    }
	//����
	buf_unlock(pbuf);

    return len;
}

/*******************************************************************************
���⣺GetAudioDataNetData
���ܣ���ȡ��Ƶ��������--���ڲ��š�ϵͳ�����ͶԽ�����
��ʽ��UINT32 GetAudioDataNetDataClr(CHAR *Data, UINT16 DataSize)
���룺Data-->���ݴ��ָ��	DataSize-->���ݳ���
�����
����ֵ:ʵ����Ч����
�쳣����û�з���
*******************************************************************************/

UINT32 GetAudioData(pRingBuffer pbuf, CHAR *Data, UINT16 DataSize)
{
	UINT8		RtnVal;
	UINT32		Ret = 0;

	if(NULL == pbuf)
	{
		return Ret;
	}
	//����
	buf_lock(pbuf);
	//�ӻ��λ�������������
	//Ret = RingBuffRead(gs_AudioData,Data,DataSize);
	Ret = RingBuffRead(pbuf,Data,DataSize);
	//����
	buf_unlock(pbuf);

	return Ret;
}

UINT32 GetAudioDataClr(pRingBuffer pbuf, CHAR *Data, UINT16 DataSize)
{
	UINT8		RtnVal;
	UINT32		Ret = 0;

	if(NULL == pbuf)
	{
		return Ret;
	}
	//����
	buf_lock(pbuf);
	//�ӻ��λ�������������
	//Ret = RingBuffRead(gs_AudioData,Data,DataSize);
	Ret = RingBuffReadClr(pbuf,Data,DataSize);
	//����
	buf_unlock(pbuf);

	return Ret;
}

/*******************************************************************************
���⣺CleanAudioData
���ܣ�����Ƶ��������
��ʽ��void CleanAudioData()
���룺��
�����
����ֵ:��
�쳣����û�з���
*******************************************************************************/
void CleanAudioData(pRingBuffer pbuf)
{
	UINT8		RtnVal;
//	pAPP_GLOBAL_MEM pAppMem = NULL;

	if(NULL == pbuf)
	{
		return ;
	}
	//��ȡȫ�ֱ���
	//pAppMem = GetAppGlobalMemPTR(NULL);
	//����Ƶ����ʱ������
	//pAppMem->DevAudioVar.Volume = MUTE;
	//����
	buf_lock(pbuf);
	RingBufferClean(pbuf);
	//����
	buf_unlock(pbuf);
}

void PostAudioData(pRingBuffer pbuf)
{
	if(NULL == pbuf)
	{
		return ;
	}

    _sem_post(&pbuf->sem);
}

void WaitAudioData(pRingBuffer pbuf)
{
	if(NULL == pbuf)
	{
		return ;
	}

    _sem_wait(&pbuf->sem);
}

UINT32 GetAudioBuffSize(pRingBuffer pbuf)
{
    return RingBuffSize(pbuf);
}


#endif




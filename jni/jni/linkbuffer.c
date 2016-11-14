

#include "log.h"

#include "linkbuffer.h"


/*******************************************************************************
�����������ǵ������ڴ����������ǿյģ������ʼ��ʱ�͸���1���ڵ������Ĭ�ϳ���
�ռ䣬��������������ʱ������Ϊ�������ݳ��Ȳ�����Ĭ�ϳ���ʱ����ֱ�ӽ����ݸ��Ƶ���1
�ڵ�ռ��ϡ����������ڴ�ķ����ͷŵ��á�
*******************************************************************************/


/*******************************************************************************
���⣺LinkAddData
���ܣ���������ӵ�pLink
���룺pLink: ����ָ��
      data: ���������
      size: ��������ݵĳ���
�����
����ֵ:
*******************************************************************************/
int LinkAddData(pLINK pLink, const char *data, int size)
{
    if(!pLink || !pLink->isInit || !data || size<=0){
        return 0;
    }

    _lock(&pLink->lock);

    if(pLink->wr!=pLink->start || pLink->wr->DataSize || size>pLink->DefSize){
        pLink->wr->next = (pLINK_HEADER)_malloc(size + sizeof(LINK_HEADER));
        if(!pLink->wr->next){
            _unlock(&pLink->lock);
            return 0;
        }
        pLink->wr = pLink->wr->next;
    }
    pLink->wr->next = NULL;
    pLink->wr->DataSize = size;
    memcpy((char *)pLink->wr+sizeof(LINK_HEADER), data, size);

    if(!pLink->rd){
        pLink->rd = pLink->wr;
        _sem_post(&pLink->sem);
    }

    _unlock(&pLink->lock);

    return size;
}

/*******************************************************************************
���⣺LinkGetData
���ܣ���������ȡ������
���룺
�����
����ֵ:
*******************************************************************************/
int LinkGetData(pLINK pLink, char *data, int size)
{
    pLINK_HEADER rd;

    if(!pLink || !pLink->isInit || !data || size<=0){
        return 0;
    }

    if(!pLink->rd){
        return 0;
    }

    _lock(&pLink->lock);

    size = pLink->rd->DataSize>size ? size:pLink->rd->DataSize;
    memcpy(data, (char *)pLink->rd+sizeof(LINK_HEADER), size);

    rd = pLink->rd;

    pLink->rd->DataSize = 0;
    pLink->rd = pLink->rd->next;
    if(!pLink->rd){
        pLink->wr = pLink->start;
    }

    if(rd != pLink->start){
        _free(rd);
    }

    _unlock(&pLink->lock);

    return size;
}

/*******************************************************************************
���⣺
���ܣ�
���룺
�����
����ֵ:
*******************************************************************************/
int CleanLink(pLINK pLink)
{
    if(!pLink || !pLink->isInit){
        return -1;
    }

    _lock(&pLink->lock);

    while(pLink->rd){
        if(pLink->rd != pLink->start){
            _free(pLink->rd);
        }
        pLink->rd = pLink->rd->next;
    }
    
    pLink->wr = pLink->start;
    pLink->wr->DataSize = 0;

    _unlock(&pLink->lock);
}

/*******************************************************************************
���⣺
���ܣ�
���룺
�����
����ֵ:
*******************************************************************************/
int CreateLink(pLINK pLink, int DefSize)
{
    if(!pLink || pLink->isInit || DefSize<=0){
        return -1;
    }

    _sem_init(&pLink->sem, 0, 0); 
    _lock_init(&pLink->lock, NULL);

    pLink->start = (pLINK_HEADER)_malloc(DefSize + sizeof(LINK_HEADER));
    if(!pLink->start){
        return -1;
    }
    pLink->wr = pLink->start;
    pLink->rd = NULL;
    pLink->DefSize = DefSize;
    pLink->isInit = TRUE;
}

/*******************************************************************************
���⣺
���ܣ�
���룺
�����
����ֵ:
*******************************************************************************/
int DelLink(pLINK pLink)
{
    if(!pLink || !pLink->isInit){
        return -1;
    }

    pLink->isInit = FALSE;

    CleanLink(pLink);

    if(pLink->start){
        _free(pLink->start);
    }

    _lock_destroy(&pLink->lock);
    _sem_destroy(&pLink->sem);
}


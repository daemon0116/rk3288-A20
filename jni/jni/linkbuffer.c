

#include "log.h"

#include "linkbuffer.h"


/*******************************************************************************
单向链表，考虑到链表在大多数情况下是空的，链表初始化时就给第1个节点分配了默认长度
空间，往链表增加数据时，发现为空且数据长度不大于默认长度时，就直接将数据复制到第1
节点空间上。这样减少内存的分配释放调用。
*******************************************************************************/


/*******************************************************************************
标题：LinkAddData
功能：将数据添加到pLink
输入：pLink: 链表指针
      data: 加入的数据
      size: 加入的数据的长度
输出：
返回值:
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
标题：LinkGetData
功能：从链表中取出数据
输入：
输出：
返回值:
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
标题：
功能：
输入：
输出：
返回值:
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
标题：
功能：
输入：
输出：
返回值:
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
标题：
功能：
输入：
输出：
返回值:
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


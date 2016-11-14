/*******************************************************************************

  ���⣺UDPClient.c.c
  ���ܣ�
  ˵������ģ���ǻ��ڱ�׼UDP SOCKET�ӿ�,������WIN linux��LWIP�ȱ�׼ͨ��SOCKETͨ��

*******************************************************************************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <string.h>



// for __android_log_print(ANDROID_LOG_INFO, "YourApp", "formatted message");
 #include <android/log.h>

#include "log.h"


#include	"UdpApp.h"
#include    "string.h"

#ifdef VC
//�������Ӹÿ�
#pragma comment(lib, "Ws2_32.lib")
#endif
//�ȴ�����Socket�����ݰ��ĳ�ʱʱ��
#define DEF_WAIT_SOCKDATA_MAX_TIME			3000	//2000ms

#define EN_BRO_ADDR //ʹ�ܹ㲥


/*******************************************************************************
���⣺UDP_Init
���ܣ�UDP��ʼ��
��ʽ��
INT32 UDP_Init(struct sockaddr_in *pSock, UINT16 Port, INT8 *pUserIP)
���룺SerIP-->������IP   Port-->����˿�
�����
����ֵ:socket�ļ���������
�쳣����û�з���
*******************************************************************************/
INT32 UDP_Init(struct sockaddr_in *pAddrIn, UINT16 Port, INT8 *pUserIP)
{
	INT32					SocketFd = -1;
#ifdef VC
	WSADATA					WsaData;
#endif
	INT32					ErrCode = -1;
	INT32					result;
	INT32					opt = 1;
    struct sockaddr_in      addr;
    struct sockaddr_in      *pAddr;
	//UINT32						nSetRxBufRet;
	//UINT32						nSetTxBufRet;
	//UINT32						nRXBufSize = 10 * 1024 * 1024;
	//UINT32						nTXBufSize = 10 * 1024 * 1024;

    if(pAddrIn){
        pAddr = pAddrIn;
    }
    else{
        pAddr = &addr;
    }
	memset((char *)pAddr, 0, sizeof(struct sockaddr_in));

	pAddr->sin_family = AF_INET;

	//������˿ں�С�ڻ���С����ʱ�����󶨶˿�
	if(Port > 0)
	{
		pAddr->sin_port = htons((unsigned short)Port);
	}
	//����IPΪ��ֵʱ��ʹ��INADDR_ANY
	if(NULL == pUserIP)
	{
		pAddr->sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		pAddr->sin_addr.s_addr = inet_addr(pUserIP);
	}

#ifdef VC
	//����Windows Sockets DLL
	if (WSAStartup(MAKEWORD(2,1),&WsaData))
	{
	#ifdef PDEBUG
		PDEBUG("Winsock�޷���ʼ��!");
	#endif
		//��ֹWinsock 2 DLL (Ws2_32.dll) ��ʹ��.
		WSACleanup();
		return ErrCode;
    }
#endif


	SocketFd = socket(AF_INET,SOCK_DGRAM,0);
	if(SocketFd < 0)
	{
#ifdef PDEBUG
		PDEBUG("---------------�ͻ����޷�����SOCKET!--------------");
#endif//PDEBUG

		return SocketFd;
	}
#ifdef PDEBUG
//	PDEBUG("----------------�ͻ���SOCKET�����ɹ�!--------------");
#endif//PDEBUG


#if 0
	nSetRxBufRet = setsockopt(SocketFd, SOL_SOCKET, SO_RCVBUF, (char *)&nRXBufSize, sizeof(int));
	nSetTxBufRet = setsockopt(SocketFd, SOL_SOCKET, SO_SNDBUF, (char *)&nTXBufSize, sizeof(int));

	//���ö˿ڸ���

	result = setsockopt(SocketFd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
	if (result < 0)
	{
	#ifdef PDEBUG
		PDEBUG("setsockopt error!\r\n");
	#endif//PDEBUG

		return ErrCode;
	}
#endif
	//���ù㲥
	result = setsockopt(SocketFd, SOL_SOCKET, SO_BROADCAST, (char*)&opt, sizeof(opt));

	if (result < 0)
	{
	#ifdef PDEBUG
		PDEBUG("setsockopt error!");
	#endif//PDEBUG

		return ErrCode;
	}

 	result = bind(SocketFd, (struct sockaddr *)pAddr, sizeof(struct sockaddr_in));
 	if (result < 0)
 	{
 		#ifdef PDEBUG
 		PDEBUG("Socket bind error");
 		#endif//PDEBUG

 		return ErrCode;
 	}

#ifdef PDEBUG
	PDEBUG("----------------�ͻ��˻���UDP SOCKET��ʼ���ɹ�!--------------");
#endif//PDEBUG
	return SocketFd;

}

/*******************************************************************************
���ܣ������鲥��
���룺SocketFd: UDP socket������
      MCAST_ADDR Ҫ������鲥IP
�����
����ֵ: <0: ʧ�ܣ�����:�ɹ�
*******************************************************************************/

int UDP_AddMembership(int SocketFd, const char *MCAST_ADDR)
{
    int err;
    uint8 loop = 0;
    struct ip_mreq mreq;                                /*����ಥ��*/
    mreq.imr_multiaddr.s_addr = inet_addr(MCAST_ADDR);  /*�ಥ��ַ*/
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);      /*����ӿ�ΪĬ��*/
                                                        /*����������ಥ��*/
    
    err = setsockopt(SocketFd, IPPROTO_IP, IP_MULTICAST_LOOP,&loop, sizeof(loop));
    err = setsockopt(SocketFd, IPPROTO_IP, IP_ADD_MEMBERSHIP,&mreq, sizeof(mreq));
    if (err < 0)
    {
        LOGW("IP_ADD_MEMBERSHIP failed:%s", MCAST_ADDR);
        return -4;
    }

    LOGD("IP_ADD_MEMBERSHIP OK:%s", MCAST_ADDR);
    return 0;
}

/*******************************************************************************
���ܣ��˳��鲥��
���룺SocketFd: UDP socket������
      MCAST_ADDR Ҫ�˳����鲥IP
�����
����ֵ: <0: ʧ�ܣ�����:�ɹ�
*******************************************************************************/
int UDP_DropMembership(int SocketFd, const char *MCAST_ADDR)
{
    int err;
    struct ip_mreq mreq;                                /*����ಥ��*/
    mreq.imr_multiaddr.s_addr = inet_addr(MCAST_ADDR);  /*�ಥ��ַ*/
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);      /*����ӿ�ΪĬ��*/
                                                        /*����������ಥ��*/
    err = setsockopt(SocketFd, IPPROTO_IP, IP_DROP_MEMBERSHIP,&mreq, sizeof(mreq));
    if (err < 0)
    {
        LOGW("IP_DROP_MEMBERSHIP failed:%s", MCAST_ADDR);
        return -4;
    }

    LOGD("IP_DROP_MEMBERSHIP OK:%s", MCAST_ADDR);
    return 0;
}

int UDP_JoinGroud(const char *MulticastIP, uint16 port)
{
    int UdpSockFd;
    
    UdpSockFd = UDP_Init(NULL, port, NULL);
    if(UdpSockFd <= 0)
    {
        LOGW("UDP_Init fail");
        return -1;
    }

    if(UDP_AddMembership(UdpSockFd, MulticastIP) == 0){
    }
    else{
        LOGW("UDP_AddMembership fail");
        close(UdpSockFd);
        return -1;
    }
    
    return UdpSockFd;   
}



/*******************************************************************************
���⣺UDP_SendData
���ܣ���UDP��������
��ʽ��
BOOL UDP_SendData(SOCKET SocketFd,char *pSerIP,int Port,void *pBuff,int len)
���룺SocketFd-->socket�ļ���������  SerIP-->������IP
		Port-->����˿� pBuf-->��������
�����
����ֵ:TRUE/FALSE
�쳣����û�з���
*******************************************************************************/

BOOL UDP_SendData(SOCKET SocketFd,const INT8 *pSerIP,UINT16 Port,void *pBuff,UINT32 len)
{
	struct sockaddr_in	server;

	//memset(&server,0,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(pSerIP);
	server.sin_port = htons((unsigned short)Port);

	/******************************֧�ֹ㲥**************************/
#ifndef EN_BRO_ADDR
	if(server.sin_addr.s_addr == INADDR_NONE)
	{
		return  -1;
	}
#endif
	/*****************************************************************/

	return (sendto(SocketFd,(const char *)pBuff,len,0,(struct sockaddr*)&server, sizeof(server)) != SOCKET_ERROR) ? TRUE : FALSE;
}

void UDP_SendData2(SOCKET SocketFd, unsigned long s_addr,UINT16 Port,void *pBuff,UINT32 len)
{
	struct sockaddr_in	server;

	if(s_addr == INADDR_NONE)
	{
		return;
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = s_addr;
	server.sin_port = htons(Port);

	sendto(SocketFd,(const char *)pBuff,len,0,(struct sockaddr*)&server, sizeof(server));
}

void UDP_SendSelf(int sock, UINT16 Port)
{
    UDP_SendData(sock, LOCAL_HOST, Port, "", 1);
}

/*******************************************************************************
���⣺UDP_RecvData
���ܣ���UDP��������
��ʽ��
INT32 UDP_RecvData(SOCKET SocketFd,INT8 *pBuff,INT8 *RemoteIp,INT16 *RemotePort,INT32 len,UINT32 timeOut)
���룺SocketFd-->socket�ļ���������  SerIP-->������IP
		Port-->����˿� pBuf-->��������
		timeOut-->��ʱ: ms
�����
����ֵ:
�쳣����û�з���
*******************************************************************************/
#if 1

INT32 UDP_RecvData(SOCKET SocketFd,INT8 *pBuff,INT8 *RemoteIp,UINT16 *RemotePort,INT32 len,INT32 timeout)
{
    INT32 ret;
	fd_set Rfds;
	struct timeval Tv;

	struct sockaddr_in	server;
    UINT32 RcvSockLen = sizeof(server);


    if(timeout >= 0) //I/O����
    {
    	FD_ZERO(&Rfds);
        FD_SET(SocketFd, &Rfds);

    	Tv.tv_sec = timeout/1000;
    	Tv.tv_usec = (timeout%1000)*1000;

    	ret = select(SocketFd+1, &Rfds, NULL, NULL, &Tv);
    	if ((ret > 0) && (FD_ISSET(SocketFd, &Rfds))){
    		ret = recvfrom(SocketFd, pBuff, (UINT32)len, 0,(struct sockaddr*)&server,&RcvSockLen);
    	}
        else{
            return 0;
        }
    }
    else
    {
    	ret = recvfrom(SocketFd, pBuff, (UINT32)len, 0,(struct sockaddr*)&server,&RcvSockLen);
    }

    if(ret > 0)
    {   
        if(RemoteIp){
		    strcpy(RemoteIp,inet_ntoa(server.sin_addr));
        }
        if(RemotePort){
		    *RemotePort = htons(server.sin_port);
        }
        return ret;
    }
    
	return -1;
}

#else

INT32 UDP_RecvData(SOCKET SocketFd,INT8 *pBuff,INT8 *RemoteIp,UINT16 *RemotePort,INT32 len)
{
	struct sockaddr_in			server;
	INT32						RecLen = -1;
    UINT32						RcvSockLen = sizeof(server);

	//�����������������
	RecLen = recvfrom(SocketFd, pBuff, (UINT32)len, 0,(struct sockaddr*)&server,\
		(UINT32L *)&RcvSockLen);
	strcpy(RemoteIp,inet_ntoa(server.sin_addr));
	*RemotePort = htons(server.sin_port);

   	return RecLen;
}

#endif

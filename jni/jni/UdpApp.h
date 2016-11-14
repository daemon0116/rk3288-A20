#ifndef _UDP_APP_H_
#define _UDP_APP_H_


#include "TypepDef.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#ifndef SOCKET
#define SOCKET	INT32
#endif


#ifndef SOCKET_ERROR
#define SOCKET_ERROR		-1
#endif

#define LOCAL_HOST                  "127.0.0.1"


#ifdef VC
#include <Winsock2.h>
__declspec(dllexport)  int		UDP_Init(struct sockaddr_in *pSock, unsigned short Port, char *pUserIP);
__declspec(dllexport)  int		UDP_SendData(SOCKET SocketFd,char *pSerIP,unsigned short Port,void *pBuff,int len);
__declspec(dllexport)  int		UDP_RecvData(SOCKET SocketFd,char *pBuff,char *RemoteIp,unsigned short *RemotePort,int len);
#endif
#define USER_IPADDR

INT32 UDP_Init(struct sockaddr_in *pSock, UINT16 Port, INT8 *pUserIP);
int UDP_AddMembership(int SocketFd, const char *MCAST_ADDR);
int UDP_DropMembership(int SocketFd, const char *MCAST_ADDR);
int UDP_JoinGroud(const char *MulticastIP, uint16 port);
BOOL UDP_SendData(SOCKET SocketFd,const INT8 *pSerIP,UINT16 Port,void *pBuff,UINT32 len);
void UDP_SendData2(SOCKET SocketFd, unsigned long s_addr,UINT16 Port,void *pBuff,UINT32 len);
void UDP_SendSelf(int sock, UINT16 Port);
INT32 UDP_RecvData(SOCKET SocketFd,INT8 *pBuff,INT8 *RemoteIp,UINT16 *RemotePort,INT32 len,INT32 timeOut);

#endif //_UDP_APP_H_

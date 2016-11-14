
#include "log.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include "common.h"

static int log_sock = -1;
static struct sockaddr_in log_server;
static int log_level = 0;
_lock_t log_lock;

int log_udp_init(struct sockaddr_in *pAddrIn, unsigned short Port, char *pUserIP)
{
	int					SocketFd = -1;
	int					ErrCode = -1;
	int					result;
	int					opt = 1;
    struct sockaddr_in      addr;
    struct sockaddr_in      *pAddr;

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

	SocketFd = socket(AF_INET,SOCK_DGRAM,0);
	if(SocketFd < 0)
	{
		return SocketFd;
	}

	//���ù㲥
	result = setsockopt(SocketFd, SOL_SOCKET, SO_BROADCAST, (char*)&opt, sizeof(opt));

	if (result < 0)
	{
		return ErrCode;
	}

 	result = bind(SocketFd, (struct sockaddr *)pAddr, sizeof(struct sockaddr_in));
 	if (result < 0)
 	{
 		return ErrCode;
 	}

	return SocketFd;
}


void nvis_log(const char *fun, int level, const char *pFormat, ...)
{
	char etxt[2048];
	va_list args;

    const char level_conversion[] = {0, 7, 7, 6, 4, 3, 2, 1};
	
    if(log_sock > 0){
        //Զ����־��������ӡ
        int n=0;
        int len=0;
        time_t timep; 
        struct tm *ptm;
		
		if(level < log_level){
			return;
		}

        _lock(&log_lock);
        
        time(&timep); 
        ptm = localtime(&timep); 
        n = snprintf(etxt, sizeof(etxt), "<%d>[%02d-%02d %02d:%02d:%02d][JNI-BC]: %s: ",  \
                    level_conversion[level]+8,                                                                \
                    (ptm->tm_mon+1), ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec,\
                    fun);

        va_start(args, pFormat);
        len = n+vsnprintf(etxt+n, sizeof(etxt)-n-3, pFormat, args);
        va_end(args);

        etxt[len++] = '\r';
        etxt[len++] = '\n';
        sendto(log_sock, etxt, len, 0, (struct sockaddr*)&log_server, sizeof(log_server));

        _unlock(&log_lock);
    }
    else{
        //USB_OTG��ӡ
        va_start(args, pFormat);
        vsnprintf(etxt, sizeof(etxt), pFormat, args);
        va_end(args);
        
        __android_log_print(level, fun, etxt, __PRETTY_FUNCTION__);
    }
}


/*******************************************************************************
���⣺InitLog
���ܣ���ʼ����־����
���룺*ip: ��־������ip
      port: ��־�������˿�
�������
����ֵ: 0: ��ȷ; <0: ����
*******************************************************************************/
int InitLog(const char *ip, unsigned short port, int flag)
{
    int sock;
    
    if(log_sock > 0){
        DelInitLog();
    }

    if(!flag){
        return 0;
    }

    sock = log_udp_init(NULL, 0, NULL);
    if(sock < 0){
		LOGE("log_udp_init err log_sock is %d\n", sock);
        return -1;
    }
    _lock_init(&log_lock, NULL);
    memset(&log_server,0,sizeof(log_server));
    log_server.sin_family = AF_INET;
    log_server.sin_addr.s_addr = inet_addr(ip);
    log_server.sin_port = htons(port);
	log_level = flag;
    log_sock = sock;
	LOGI("InitLog is sec\n");
	
    return 0;
}

/*******************************************************************************
���⣺DelInitLog
���ܣ�����ʼ����־����
���룺��
�������
����ֵ: 0: ��ȷ
*******************************************************************************/
int DelInitLog(void)
{
    if(log_sock > 0){
        close(log_sock);
        log_sock = -1;
        _lock_destroy(&log_lock);
    }
    return 0;
}

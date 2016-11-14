
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

	//当传入端口号小于或者小于零时不作绑定端口
	if(Port > 0)
	{
		pAddr->sin_port = htons((unsigned short)Port);
	}
	//传入IP为空值时，使用INADDR_ANY
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

	//设置广播
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
        //远程日志服务器打印
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
        //USB_OTG打印
        va_start(args, pFormat);
        vsnprintf(etxt, sizeof(etxt), pFormat, args);
        va_end(args);
        
        __android_log_print(level, fun, etxt, __PRETTY_FUNCTION__);
    }
}


/*******************************************************************************
标题：InitLog
功能：初始化日志处理
输入：*ip: 日志服务器ip
      port: 日志服务器端口
输出：无
返回值: 0: 正确; <0: 错误
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
标题：DelInitLog
功能：反初始化日志处理
输入：无
输出：无
返回值: 0: 正确
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

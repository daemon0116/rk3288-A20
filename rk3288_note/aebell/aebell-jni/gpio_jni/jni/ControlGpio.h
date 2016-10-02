#ifndef __CONTROL_GPIO_H__
#define __CONTROL_GPIO_H__
#include <android/log.h>
#include <linux/ioctl.h>
#define	RK3288_GPIO_MAGIC		'k'
#define RK3288_REQ_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x20,int)//申请gpio口
#define RK3288_REL_GPIO			_IOWR(RK3288_GPIO_MAGIC,0x21,int)//释放gpio口
#define RK3288_INP_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x22,int)//设置gpio口为输入
#define RK3288_OUT_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x23,int)//设置gpio口为输入
#define RK3288_SET_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x24,int)//设置gpio口的值
#define RK3288_GET_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x25,int)//获取gpio口的值
#define RK3288_GPIO_MAXNR       0x25
#define RK3288_GPIO_MINNR       0x20
/*
    该结构体主要用于ioctl控制gpio口操作
*/
struct UserData{
    unsigned int gpio;  //用户的gpio口
    unsigned int state; //用户的gpio口状态
};
#define BUFFER_MAX      32
#define DIRECTION_MAX   64
#define IN              0
#define OUT             1
#define LOW             0
#define HIGH            1
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "ProjectName", __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG , "ProjectName", __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO  , "ProjectName", __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN  , "ProjectName", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "ProjectName", __VA_ARGS__)
enum{
    ERROK = 0,          //正常
    ERROPEN = -1,       //打开文件失败
    ERRCLOSE = -2,      //关闭文件失败
    ERRREAD = -3,       //读取数据失败
    ERRWRITE = -4,      //写入数据失败
    ERRPARA = -5,       //参数异常
    ERROTHER = -6,      //其他异常
};
struct KeyState{
	int code;			//按键code
	int value;			//按键值：按下或者抬起来
};
/*
struct timeval{
	long long tv_sec;
	long long tv_usec;
};
struct input_event { 
	struct timeval time;
	unsigned short type;
	unsigned short code;
	unsigned int value; 
}; 
*/
#endif

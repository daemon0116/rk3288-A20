#ifndef __CONTROL_GPIO_H__
#define __CONTROL_GPIO_H__
#include <android/log.h>
#include <linux/input.h>
#include <linux/ioctl.h>
//第一种控制gpio口的接口,使用的是在内核中添加驱动rk3288-gpio.ko中ioctl的方式控制;
//第二种方式是通过自带的export导出用户接口的方式控制;
#define	FIRST_MOTHED_CONTROL_GPIO	//使用第几种gpio口控制方式


#define	RK3288_GPIO_MAGIC		'k'
#define RK3288_REQ_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x20,int)//申请gpio口
#define RK3288_REL_GPIO			_IOWR(RK3288_GPIO_MAGIC,0x21,int)//释放gpio口
#define RK3288_INP_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x22,int)//设置gpio口为输入
#define	RK3288_OUT_GPIO			_IOWR(RK3288_GPIO_MAGIC,0x23,int)//设置gpio为输出
#define RK3288_SET_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x24,int)//设置gpio口的值
#define RK3288_GET_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x25,int)//获取gpio口的值
#define RK3288_DISABEL_WATCHDOG _IOWR(RK3288_GPIO_MAGIC,0x26,int)//禁用系统喂狗
#define RK3288_GPIO_MAXNR       0x26
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
enum{
    ERROK = 0,          //正常
    ERROPEN = -1,       //打开文件失败
    ERRCLOSE = -2,      //关闭文件失败
    ERRREAD = -3,       //读取数据失败
    ERRWRITE = -4,      //写入数据失败
    ERRPARA = -5,       //参数异常
    ERROTHER = -6,      //其他异常
};
enum{
	PA_NR_BASE = 0,         /* PA  24 */
	PB_NR_BASE = 24,  		/* PB  30 */
	PC_NR_BASE = 54,  		/* PC  31 */
	PD_NR_BASE = 85, 		/* PD  34 */
	PE_NR_BASE = 119,  		/* PE  18 */
	PF_NR_BASE = 137,  		/* PF  12 */
	PG_NR_BASE = 149,  		/* PG  18 */
	PH_NR_BASE = 167,  		/* PH  34 */
	PI_NR_BASE = 201,  		/* PI  28 */
	AXP_NR_BASE= 229,  		/* AXP 11 */
};
struct KeyState{
	int code;			//按键code
	int value;			//按键值：按下或者抬起来
};

#ifdef FIRST_MOTHED_CONTROL_GPIO
extern int AeGetGpioNumber(char* gpioName);
extern int AeOpenGpioDev(void);
extern int AeCloseGpioDev(int fd);
extern int AeRequestGpio(int fd, int num);
extern int AeReleaseGpio(int fd, int num);
extern int AeSetGpioInput(int fd, int num);
extern int AeSetGpioOutput(int fd, int num,int value);
extern int AeGetGpioState(int fd, int num);
extern int AeSetGpioState(int fd, int num, int state);
extern int AeOpenKeyDev(void);
extern int AeCloseKeyDev(int fd);
extern int AeControlMicInput(int channel);
extern int AeControlSystemWatchDog(int fd,int state);
#else
extern int AeGetGpioNumber(char* gpioName);
extern int AeGpioInit(int pin);
extern int AeGpioDestory(int pin);
extern int AeGetGpioDirection(int pin);
extern int AeSetGpioDirection(int pin, int dir);
extern int AeGetGpioValue(int pin);
extern int AeSetGpioValue(int pin, jint value);
#endif

#endif

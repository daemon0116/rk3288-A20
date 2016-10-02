#ifndef __RK3288_GPIO_H__
#define	__RK3288_GPIO_H__
#include <linux/ioctl.h>
#define	RK3288_GPIO_MAGIC		'k'
#define RK3288_REQ_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x20,int)//申请gpio口
#define RK3288_REL_GPIO			_IOWR(RK3288_GPIO_MAGIC,0x21,int)//释放gpio口
#define RK3288_INP_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x22,int)//设置gpio口为输入
#define RK3288_SET_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x23,int)//设置gpio口的值
#define RK3288_GET_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x24,int)//获取gpio口的值
#define RK3288_GPIO_MAXNR       0x24
#define RK3288_GPIO_MINNR       0x20
/*
    该结构体主要用于ioctl控制gpio口操作
*/
struct UserData{
    unsigned int gpio;  //用户的gpio口
    unsigned int state; //用户的gpio口状态
};
#endif
#ifndef __CONTROL_GPIO_H__
#define __CONTROL_GPIO_H__
#define BUFFER_MAX      32
#define DIRECTION_MAX   64
#define IN              0
#define OUT             1
#define LOW             0
#define HIGH            1
#if 1
#define DEBUGLOG_GPIO    printf
#else
#define DEBUGLOG_GPIO
#endif
enum{
    ERROK = 0,          //正常
    ERROPEN = -1,       //打开文件失败
    ERRCLOSE = -2,      //关闭文件失败
    ERRREAD = -3,       //读取数据失败
    ERRWRITE = -4,      //写入数据失败
    ERRPARA = -5,       //参数异常
    ERROTHER = -6,      //其他异常
};
extern int AebellGpioInit(int pin);
extern int AebellGpioDestory(int pin);
extern int AebellGetGpioDirection(int pin);
extern int AebellSetGpioDirection(int pin, int dir);
extern int AebellGetGpioValue(int pin);
extern int AebellSetGpioValue(int pin, int value);
#endif

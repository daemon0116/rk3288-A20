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
    ERROK = 0,          //����
    ERROPEN = -1,       //���ļ�ʧ��
    ERRCLOSE = -2,      //�ر��ļ�ʧ��
    ERRREAD = -3,       //��ȡ����ʧ��
    ERRWRITE = -4,      //д������ʧ��
    ERRPARA = -5,       //�����쳣
    ERROTHER = -6,      //�����쳣
};
extern int AebellGpioInit(int pin);
extern int AebellGpioDestory(int pin);
extern int AebellGetGpioDirection(int pin);
extern int AebellSetGpioDirection(int pin, int dir);
extern int AebellGetGpioValue(int pin);
extern int AebellSetGpioValue(int pin, int value);
#endif

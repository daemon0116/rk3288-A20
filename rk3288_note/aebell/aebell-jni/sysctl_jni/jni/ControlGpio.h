#ifndef __CONTROL_GPIO_H__
#define __CONTROL_GPIO_H__
#include <android/log.h>
#include <linux/input.h>
#include <linux/ioctl.h>
//��һ�ֿ���gpio�ڵĽӿ�,ʹ�õ������ں����������rk3288-gpio.ko��ioctl�ķ�ʽ����;
//�ڶ��ַ�ʽ��ͨ���Դ���export�����û��ӿڵķ�ʽ����;
#define	FIRST_MOTHED_CONTROL_GPIO	//ʹ�õڼ���gpio�ڿ��Ʒ�ʽ


#define	RK3288_GPIO_MAGIC		'k'
#define RK3288_REQ_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x20,int)//����gpio��
#define RK3288_REL_GPIO			_IOWR(RK3288_GPIO_MAGIC,0x21,int)//�ͷ�gpio��
#define RK3288_INP_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x22,int)//����gpio��Ϊ����
#define	RK3288_OUT_GPIO			_IOWR(RK3288_GPIO_MAGIC,0x23,int)//����gpioΪ���
#define RK3288_SET_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x24,int)//����gpio�ڵ�ֵ
#define RK3288_GET_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x25,int)//��ȡgpio�ڵ�ֵ
#define RK3288_DISABEL_WATCHDOG _IOWR(RK3288_GPIO_MAGIC,0x26,int)//����ϵͳι��
#define RK3288_GPIO_MAXNR       0x26
#define RK3288_GPIO_MINNR       0x20
/*
    �ýṹ����Ҫ����ioctl����gpio�ڲ���
*/
struct UserData{
    unsigned int gpio;  //�û���gpio��
    unsigned int state; //�û���gpio��״̬
};
#define BUFFER_MAX      32
#define DIRECTION_MAX   64
#define IN              0
#define OUT             1
#define LOW             0
#define HIGH            1
enum{
    ERROK = 0,          //����
    ERROPEN = -1,       //���ļ�ʧ��
    ERRCLOSE = -2,      //�ر��ļ�ʧ��
    ERRREAD = -3,       //��ȡ����ʧ��
    ERRWRITE = -4,      //д������ʧ��
    ERRPARA = -5,       //�����쳣
    ERROTHER = -6,      //�����쳣
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
	int code;			//����code
	int value;			//����ֵ�����»���̧����
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

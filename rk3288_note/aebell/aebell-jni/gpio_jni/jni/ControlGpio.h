#ifndef __CONTROL_GPIO_H__
#define __CONTROL_GPIO_H__
#include <android/log.h>
#include <linux/ioctl.h>
#define	RK3288_GPIO_MAGIC		'k'
#define RK3288_REQ_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x20,int)//����gpio��
#define RK3288_REL_GPIO			_IOWR(RK3288_GPIO_MAGIC,0x21,int)//�ͷ�gpio��
#define RK3288_INP_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x22,int)//����gpio��Ϊ����
#define RK3288_OUT_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x23,int)//����gpio��Ϊ����
#define RK3288_SET_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x24,int)//����gpio�ڵ�ֵ
#define RK3288_GET_GPIO        	_IOWR(RK3288_GPIO_MAGIC,0x25,int)//��ȡgpio�ڵ�ֵ
#define RK3288_GPIO_MAXNR       0x25
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
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "ProjectName", __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG , "ProjectName", __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO  , "ProjectName", __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN  , "ProjectName", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "ProjectName", __VA_ARGS__)
enum{
    ERROK = 0,          //����
    ERROPEN = -1,       //���ļ�ʧ��
    ERRCLOSE = -2,      //�ر��ļ�ʧ��
    ERRREAD = -3,       //��ȡ����ʧ��
    ERRWRITE = -4,      //д������ʧ��
    ERRPARA = -5,       //�����쳣
    ERROTHER = -6,      //�����쳣
};
struct KeyState{
	int code;			//����code
	int value;			//����ֵ�����»���̧����
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <jni.h>
#include "ControlGpio.h"
#include "log.h"

/*******************************************************************************
���⣺GetGpioNumber
���ܣ�ͨ��Gpio�ڵ����ֻ�ȡGpio���ű��
��ʽ��
���룺Gpio������
�����Gpio�����ű��
����ֵ:��
�쳣����û�з���
*******************************************************************************/
int AeGetGpioNumber(char* name)
{
	int number;
	if( name == NULL ||'9'<name[4] || name[4]<'0' || '9'<name[6] || name[6]<'0')
	{
		LOGE("gpio number err,please check gpio style");
		return -1;
	}
	if(!((name[5] >= 'a' && name[5] <= 'd') || (name[5] >= 'A' && name[5] <= 'D')))
	{
		LOGE("gpio number err,please check gpio style");
		return -1;
	}
	if((name[5] >= 'a' && name[5] <= 'd'))//�����Сд��ĸ
	{
		number = (name[4] - '0')*32 + (name[5] -'a')*8 + (name[6] - '0');
	}
	else if((name[5] >= 'A' && name[5] <= 'D')){//����Ǵ�д��ĸ
		number = (name[4] - '0')*32 + (name[5] -'A')*8 + (name[6] - '0');
	}
	//LOGD("%s is success",__func__);
	return number;
}

#ifdef FIRST_MOTHED_CONTROL_GPIO
/*******************************************************************************
���⣺AeOpenGpioDev
���ܣ����ַ��豸��ȡ�ļ�������
��ʽ��
���룺��
�������
����ֵ:�ļ�������
�쳣����û�з���
*******************************************************************************/
int AeOpenGpioDev()
{
	int fd;
	fd = open("/dev/rk3288_gpio", O_RDWR);
	if (fd < 0) {
		LOGE("%s:open /dev/rk3288_gpio err",__func__);
	}
	//LOGD("%s is success",__func__);
	return fd;
}
/*******************************************************************************
���⣺AeOpenGpioDev
���ܣ��ر��ַ��豸�Լ��ļ�������
��ʽ��
���룺�ļ�������
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
int AeCloseGpioDev(int fd)
{
	int ret=0;
	ret = close(fd);
	if (ret < 0) {
		LOGE("%s:close /dev/rk3288_gpio err",__func__);
	}
	//LOGD("%s is success",__func__);
	return ret;
}
/*******************************************************************************
���⣺AeRequestGpio
���ܣ�����Gpio��
��ʽ��
���룺fd���ļ������� num:Gpio�����ű��
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
int AeRequestGpio(int fd, int num)
{
	int ret=-1;
	struct UserData userdata;
	memset(&userdata,0x00, sizeof(userdata));
	userdata.gpio=num;
	userdata.state=0;

	ret = ioctl(fd, RK3288_REQ_GPIO, &userdata);
	if(ret < 0)
		LOGE("%s:request gpio%d err,ret = %d",__func__,num,ret);
	//LOGD("%s is success",__func__);
	return ret;
}
/*******************************************************************************
���⣺AeReleaseGpio
���ܣ��ͷ�Gpio��
��ʽ��
���룺fd���ļ������� num:Gpio�����ű��
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
int AeReleaseGpio(int fd, int num)
{
	int ret=-1;
	struct UserData userdata;
	memset(&userdata,0x00, sizeof(userdata));
	userdata.gpio=num;
	userdata.state=0;
	ret = ioctl(fd, RK3288_REL_GPIO, &userdata);
	if(ret < 0)
		LOGE("%s:release gpio%d err,ret = %d",__func__,num,ret);
	//LOGD("%s is success",__func__);
	return ret;
}
/*******************************************************************************
���⣺SetGpioInput
���ܣ�����Gpio��Ϊ����
��ʽ��
���룺fd���ļ������� num:Gpio�����ű��
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
int AeSetGpioInput(int fd, int num)
{
    int ret=-1;
    struct UserData userdata;
    memset(&userdata,0x00, sizeof(userdata));
    userdata.gpio=num;
    userdata.state=0;

    ret = ioctl(fd, RK3288_INP_GPIO, &userdata);
    if(ret<0){
		LOGE("%s:set gpio%d to input err,ret = %d",__func__, num,ret);
    }
	//LOGD("%s is success",__func__);
    return ret;
}
/******************************************************************************
*
���⣺AeSetGpioOutput
���ܣ�����Gpio��Ϊ���
��ʽ��
���룺fd���ļ������� num:Gpio�����ű��
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
int AeSetGpioOutput(int fd, int num,int value)
{
    int ret=-1;
    struct UserData userdata;
    memset(&userdata,0x00, sizeof(userdata));
    userdata.gpio=num;
    userdata.state=value;

    ret = ioctl(fd, RK3288_OUT_GPIO, &userdata);
    if(ret<0){
		LOGE("%s:set gpio %d to output err,ret = %d",__func__, num,ret);
    }
	//LOGD("%s is success",__func__);
    return ret;
}
/*******************************************************************************
���⣺AeGetGpioState
���ܣ���ȡGpio�ڵ�ֵ
��ʽ��
���룺fd���ļ������� num:Gpio�����ű��
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
int AeGetGpioState(int fd, int num)
{
    int ret=-1;
    struct UserData userdata;
    memset(&userdata,0x00, sizeof(userdata));
    userdata.gpio=num;
    userdata.state=0;
    ret = ioctl(fd, RK3288_GET_GPIO, &userdata);
    if(ret<0){
		LOGE("%s:get gpio%d status err,ret = %d",__func__, num,ret);
     	return -1;
    }
	//LOGD("%s is success",__func__);
    return userdata.state;
}

/*******************************************************************************
���⣺AeSetGpioState
���ܣ�����Gpio��Ϊ���������ֵ
��ʽ��
���룺fd���ļ������� num:Gpio�����ű�� state:����ֵ
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
int AeSetGpioState(int fd, int num, int state)
{
	int ret=-1;
	struct UserData userdata;
	memset(&userdata,0x00, sizeof(userdata));
	userdata.gpio=num;
	userdata.state=state;

	ret = ioctl(fd, RK3288_SET_GPIO, &userdata);
	if(ret<0){
		LOGE("%s:set gpio%d to value err,ret = %d",__func__, num,ret);
	}
	//LOGD("%s is success",__func__);
	return ret;
}
/*******************************************************************************
���⣺AeOpenKeyDev
���ܣ��򿪻�ȡ�����豸�Ľڵ�
��ʽ��
���룺��
�������
����ֵ:>0������ �������쳣
�쳣����û�з���
*******************************************************************************/
int AeOpenKeyDev()
{
	int count = 0;
	int fd = 0;
	char devName[64] = {"/dev/input/event3"};
	fd = open(devName,O_RDONLY);
	if(fd < 0)
	{
		LOGE("%s:open %s is failed",__func__,devName);
		return -1;
	}
	//LOGD("%s is success",__func__);
	return fd;
}
/*******************************************************************************
���⣺AeCloseKeyDev
���ܣ��رջ�ȡ�����豸�Ľڵ�
��ʽ��
���룺fd���ļ�������
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
int AeCloseKeyDev(int fd)
{
	close(fd);
	//LOGD("%s is success",__func__);
	return 0;
}
/*******************************************************************************
���⣺AeControlMicInput
���ܣ��л�micͨ��
��ʽ��
���룺��
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
int AeControlMicInput(int channel)
{
	int fd;
	fd = open("/sys/class/es8323/mic_state/mic_state", O_WRONLY);
    if (fd < 0) {
        LOGE("%s:Failed to open /sys/class/es8323/mic_state/mic_state for writing",__func__);
        return ERROPEN;
    }
    if (write(fd, channel==1 ? "1" : "2", 1) < 0) {
        LOGE("%s:Failed to write /sys/class/es8323/mic_state/mic_state",__func__);
        close(fd);
        return ERRWRITE;
    }
    close(fd);
	//LOGD("%s is success",__func__);
    return ERROK;
}

/*******************************************************************************
���⣺AeControlSystemWatchDog
���ܣ�����ϵͳ���Ź�����
��ʽ��
���룺fd���ļ������� 
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
int AeControlSystemWatchDog(int fd,int state)
{
	int ret=-1;
	struct UserData userdata;
	memset(&userdata,0x00, sizeof(userdata));
	userdata.state = state;

	ret = ioctl(fd, RK3288_DISABEL_WATCHDOG, &userdata);
	if(ret<0){
		LOGE("%s:disable system watchdog is failed",__func__);
	}
	//LOGD("%s is success",__func__);
	return ret;
}
#else
/*******************************************************************************
���⣺AeGpioIsInit
���ܣ��ж�Gpio���Ƿ��ʼ��
��ʽ��
int GpioIsInit(int pin)
���룺pin:Gpio�����ű��
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
static int AeGpioIsInit(int pin)
{
    char path[DIRECTION_MAX];
    snprintf(path,DIRECTION_MAX,"/sys/class/gpio/gpio%d",pin);
    if(access(path,F_OK) != 0)
    {
        return ERROTHER;
    }
	//LOGD("%s is success",__func__);
    return ERROK;
}
/*******************************************************************************
���⣺AeGpioInit
���ܣ�Gpio�ڳ�ʼ��
��ʽ��
���룺pin:Gpio�����ű��
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
int AeGpioInit(int pin)
{
    char buffer[BUFFER_MAX];
    int len;
    int fd;
    if(pin < 0)
        return ERRPARA;
    if(AeGpioIsInit(pin) != ERROK)
    {
        fd = open("/sys/class/gpio/export", O_WRONLY);
        if (fd < 0) {
            LOGE("%s:Failed to open export for writing...",__func__);
            return ERROPEN;
        }

        len = snprintf(buffer, BUFFER_MAX, "%d", pin);
        if (write(fd, buffer, len) < 0) {
            LOGE("%s:Fail to export gpio...",__func__);
            close(fd);
            return ERRWRITE;
        }
        close(fd);
    }
	//LOGD("%s is success",__func__);
    return ERROK;
}
/*******************************************************************************
���⣺AeGpioDestory
���ܣ�Gpio������
��ʽ��
���룺pin:Gpio�����ű��
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
int AeGpioDestory(int pin)
{
    char buffer[BUFFER_MAX];
    int len;
    int fd;
    if(pin < 0)
        return ERRPARA;
    if(AeGpioIsInit(pin) == ERROK)
    {
        fd = open("/sys/class/gpio/unexport", O_WRONLY);
        if (fd < 0) {
            LOGE("%s:Failed to open unexport for writing...",__func__);
            return ERROPEN;
        }

        len = snprintf(buffer, BUFFER_MAX, "%d", pin);
        if (write(fd, buffer, len) < 0) {
            LOGE("%s:Fail to unexport gpio...",__func__);
            close(fd);
            return ERRWRITE;
        }
        close(fd);
    }
	//LOGD("%s is success",__func__);
    return ERROK;
}
/*******************************************************************************
���⣺AeGetGpioDirection
���ܣ�Gpio�ڵ������������Ļ�ȡ
��ʽ��
���룺pin:Gpio�����ű��
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
int AeGetGpioDirection(int pin)
{
    char path[DIRECTION_MAX];
    char value_str[3] = {0};
    int fd;
    if(pin < 0)
        return ERRPARA;
    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        LOGE("%s:Failed to open gpio direction for writing...",__func__);
        return ERROPEN;
    }
    if (read(fd, value_str, 3) < 0) {
        LOGE("%s:Failed to read direction value...",__func__);
        close(fd);
        return ERRREAD;
    }

    close(fd);
    if(strcmp(value_str,"out") == 0)
        return 1;
	//LOGD("%s is success",__func__);
    return 0;
}
/*******************************************************************************
���⣺AeSetGpioDirection
���ܣ�����Gpio�ڵķ���Ϊ���
��ʽ��
���룺pin:Gpio�����ű�� dir:��������
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
int AeSetGpioDirection(int pin, int dir)
{
    static const char dir_str[] = "in\0out";
    char path[DIRECTION_MAX];
    int fd;
    if((dir != 0 && dir != 1) || (pin < 0))
        return ERRPARA;
    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_WRONLY);
    if (fd < 0) {
        LOGE("%s:Failed to open gpio direction for writing...",__func__);
        return ERROPEN;
    }
    if (write(fd, &dir_str[dir == IN ? 0 : 3], dir == IN ? 2 : 3) < 0) {
        LOGE("%s:Failed to read direction...",__func__);
        close(fd);
        return ERRWRITE;
    }
    close(fd);
	//LOGD("%s is success",__func__);
    return ERROK;
}
/*******************************************************************************
���⣺AeGetGpioValue
���ܣ���ȡGpio�ڵ�ֵ
��ʽ��
���룺pin:Gpio�����ű��
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
int AeGetGpioValue(int pin)
{
    char path[DIRECTION_MAX];
    char value_str[3]={0};
    int fd;
    if(pin < 0)
        return ERRPARA;
    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        LOGE("%s:Failed to open gpio value for reading...",__func__);
        return ERROPEN;
    }

    if (read(fd, value_str, 3) < 0) {
        LOGE("%s:Failed to read value...",__func__);
        close(fd);
        return ERRREAD;
    }
    close(fd);
	LOGI("%s is success",__func__);
    return (atoi(value_str));
}
/*******************************************************************************
���⣺AeSetGpioValue
���ܣ�����Gpio�ڵ�ֵ
��ʽ��
���룺pin:Gpio�����ű�� value:����ֵ
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
int AeSetGpioValue(int pin, int value)
{
    static const char values_str[] = "01";
    char path[DIRECTION_MAX];
    int fd;
    if((value != 0 && value != 1) || (pin < 0))
        return ERRPARA;
    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_WRONLY);
    if (fd < 0) {
        LOGE("%s:Failed to open gpio value for writing...",__func__);
        return ERROPEN;
    }

    if (write(fd, &values_str[value == LOW ? 0 : 1], 1) < 0) {
        LOGE("%s:Failed to write value...",__func__);
        close(fd);
        return ERRWRITE;
    }
    close(fd);
	//LOGD("%s is success",__func__);
    return ERROK;
}
#endif



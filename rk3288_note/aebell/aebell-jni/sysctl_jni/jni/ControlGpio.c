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
标题：GetGpioNumber
功能：通过Gpio口的名字获取Gpio引脚编号
格式：
输入：Gpio口名字
输出：Gpio口引脚编号
返回值:无
异常：暂没有发现
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
	if((name[5] >= 'a' && name[5] <= 'd'))//如果是小写字母
	{
		number = (name[4] - '0')*32 + (name[5] -'a')*8 + (name[6] - '0');
	}
	else if((name[5] >= 'A' && name[5] <= 'D')){//如果是大写字母
		number = (name[4] - '0')*32 + (name[5] -'A')*8 + (name[6] - '0');
	}
	//LOGD("%s is success",__func__);
	return number;
}

#ifdef FIRST_MOTHED_CONTROL_GPIO
/*******************************************************************************
标题：AeOpenGpioDev
功能：打开字符设备获取文件描述符
格式：
输入：无
输出：无
返回值:文件描述符
异常：暂没有发现
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
标题：AeOpenGpioDev
功能：关闭字符设备以及文件描述符
格式：
输入：文件描述符
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
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
标题：AeRequestGpio
功能：申请Gpio口
格式：
输入：fd：文件描述符 num:Gpio口引脚编号
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
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
标题：AeReleaseGpio
功能：释放Gpio口
格式：
输入：fd：文件描述符 num:Gpio口引脚编号
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
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
标题：SetGpioInput
功能：设置Gpio口为输入
格式：
输入：fd：文件描述符 num:Gpio口引脚编号
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
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
标题：AeSetGpioOutput
功能：设置Gpio口为输出
格式：
输入：fd：文件描述符 num:Gpio口引脚编号
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
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
标题：AeGetGpioState
功能：获取Gpio口的值
格式：
输入：fd：文件描述符 num:Gpio口引脚编号
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
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
标题：AeSetGpioState
功能：设置Gpio口为输出并设置值
格式：
输入：fd：文件描述符 num:Gpio口引脚编号 state:引脚值
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
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
标题：AeOpenKeyDev
功能：打开获取按键设备的节点
格式：
输入：无
输出：无
返回值:>0：正常 其他：异常
异常：暂没有发现
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
标题：AeCloseKeyDev
功能：关闭获取按键设备的节点
格式：
输入：fd：文件描述符
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
*******************************************************************************/
int AeCloseKeyDev(int fd)
{
	close(fd);
	//LOGD("%s is success",__func__);
	return 0;
}
/*******************************************************************************
标题：AeControlMicInput
功能：切换mic通道
格式：
输入：无
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
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
标题：AeControlSystemWatchDog
功能：禁用系统看门狗操作
格式：
输入：fd：文件描述符 
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
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
标题：AeGpioIsInit
功能：判断Gpio口是否初始化
格式：
int GpioIsInit(int pin)
输入：pin:Gpio口引脚编号
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
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
标题：AeGpioInit
功能：Gpio口初始化
格式：
输入：pin:Gpio口引脚编号
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
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
标题：AeGpioDestory
功能：Gpio口销毁
格式：
输入：pin:Gpio口引脚编号
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
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
标题：AeGetGpioDirection
功能：Gpio口的输入输出方向的获取
格式：
输入：pin:Gpio口引脚编号
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
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
标题：AeSetGpioDirection
功能：设置Gpio口的方向为输出
格式：
输入：pin:Gpio口引脚编号 dir:方向设置
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
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
标题：AeGetGpioValue
功能：获取Gpio口的值
格式：
输入：pin:Gpio口引脚编号
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
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
标题：AeSetGpioValue
功能：设置Gpio口的值
格式：
输入：pin:Gpio口引脚编号 value:引脚值
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
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



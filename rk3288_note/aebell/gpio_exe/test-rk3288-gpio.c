#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <android/log.h>
//#include <cutils/log.h>
#include "test-rk3288-gpio.h"

#define LOG_TAG "--JNI_GPIO_DEBUG--"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
//exp name : "gpio0B5";

static int getgpionumber(char* name)
{
	int number;

	if( name == NULL ||'9'<name[4] || name[4]<'0' || 'D'<name[5] || name[5]<'A' || '9'<name[6] || name[6]<'0')
	{
		LOGI("gpio number err,please check gpio style\n");
		return -1;
	}

	number = (name[4] - '0')*32 + (name[5] -'A')*8 + (name[6] - '0');
	return number;
}

static int requestGpio(int fd, int num){

	int ret=-1;
	struct UserData userdata;
	memset(&userdata,0x00, sizeof(userdata));
	//strlcpy(userdata.name, "gpio",10);
	userdata.gpio=num;
	userdata.state=0;

	ret = ioctl(fd, RK3288_REQ_GPIO, &userdata);
	if(ret < 0)
		LOGI("request gpio err \n");
	return ret;
}

static int releaseGpio(int fd, int num){

	int ret=-1;
	struct UserData userdata;
	memset(&userdata,0x00, sizeof(userdata));
	userdata.gpio=num;
	userdata.state=0;
	ret = ioctl(fd, RK3288_REL_GPIO, &userdata);
	return ret;
}

static int openGpioDev(void){
	int fd;

	fd = open("/dev/rk3288_gpio", O_RDWR);
	if (fd < 0) {
		LOGI("open /dev/rk3288_gpio err...!\n");
		return -1;
	}
	return fd;
}

static int closeGpioDev(int fd){


	int ret=0;

	ret = close(fd);
	if (fd < 0) {
		LOGI("close, close err...!\n");
		ret=-1;
	}
	return ret;
}

static int setGpioState(int fd, int num, int state) {
	int err=-1;
	struct UserData userdata;
	memset(&userdata,0x00, sizeof(userdata));
	userdata.gpio=num;
	userdata.state=state;

	err = ioctl(fd, RK3288_SET_GPIO, &userdata);
	if(err<0){
		LOGI("set gpio %d to value err!\n", num);	
		err=-1;
	}
	return err;
}

static int getGpioState(int fd, int num) {
    int status=-1;
    struct UserData userdata;
    memset(&userdata,0x00, sizeof(userdata));
    userdata.gpio=num;
    userdata.state=0;

    status = ioctl(fd, RK3288_GET_GPIO, &userdata);
    if(status<0){
		LOGI("get gpio %d status err!\n", num);
     	return -1;
     }
    return userdata.state;
}

static int setGpioInput(int fd, int num) {
    int err=-1;
    struct UserData userdata;
    memset(&userdata,0x00, sizeof(userdata));
    userdata.gpio=num;
    userdata.state=0;

    err = ioctl(fd, RK3288_INP_GPIO, &userdata);
    if(err<0){
		LOGI("set gpio %d to input err!\n", num);
        err=-1;
    }
    return err;
}

int main(void)
{
	int fd;
	char name[32] =	{0};
	char count = 0;
	int num = 0;
    int ret = -1;
    int value = 0;
	char res[16] = {0};
	
    while(1)
	{
        
    	printf(">>>>>>>>>>please input gpio name:");
    	scanf("%s",name);
		fd = openGpioDev();
    	num = getgpionumber(name);
    	ret = requestGpio(fd,num);
        if(ret < 0){
            printf("request gpio is failed !\r\n");
            continue;
			//return 0;
        }
    	while(count++<3)
    	{
            value = getGpioState(fd,num);
    		printf(">>>>>>>>>>output getGpioState:%d\r\n",value);
    		ret = setGpioState(fd,num,count%2);
            if(ret < 0){
                printf("set gpio value %d is failed !\r\n",count%2);
                releaseGpio(fd,num);
                closeGpioDev(fd);
                break;
				//return 0;
            }
            sleep(1);
    	}
        count = 0;
        if(ret < 0) return 0;
    	ret = setGpioInput(fd,num);
        if(ret < 0)
        {
            printf("set gpio as input is failed !\r\n");
            releaseGpio(fd,num);
            closeGpioDev(fd);
			//return 0;
			continue;
        }
        value = getGpioState(fd,num);
    	printf(">>>>>>>>>>input getGpioState:%d\r\n",value);
		releaseGpio(fd,num);
		closeGpioDev(fd);
		printf("Do you want to Continue(Y/N):");
		scanf("%s",res);
		if(res[0] == 'Y' || res[0] == 'y')
			continue;
		else
			break;
    }
	
	return 0;
}


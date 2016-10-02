#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <jni.h>
#include "ControlGpio.h"
#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
static const char *GpioClassPathName = "com/aebell/jni/aebelltest/ControlGpio";

//第一种控制gpio口的接口,使用的是在内核中添加驱动rk3288-gpio.ko中ioctl的方式控制;
//第二种方式是通过自带的export导出用户接口的方式控制,该种方式控制由于需要给每个导出来的io口配置控制权限,暂时使用第一种方式;
#define	FIRST_MOTHED_CONTROL_GPIO	//使用第几种gpio口控制方式
/*******************************************************************************
标题：GetGpioNumber
功能：通过Gpio口的名字获取Gpio引脚编号
格式：
输入：Gpio口名字(gpio5a6 gpio5A6 all nice)
输出：Gpio口引脚编号
返回值:无
异常：暂没有发现
*******************************************************************************/
jint GetGpioNumber(JNIEnv* env,jobject thiz,jstring gpioName)
{
	int number;
    char *name =(char *)(*env)->GetStringUTFChars(env,gpioName, NULL);
	if( name == NULL ||'9'<name[4] || name[4]<'0' || '9'<name[6] || name[6]<'0')
	{
		LOGI("gpio number err,please check gpio style\n");
		return -1;
	}
	if(!((name[5] >= 'a' && name[5] <= 'd') || (name[5] >= 'A' && name[5] <= 'D')))
	{
		LOGI("gpio number err,please check gpio style\n");
		return -1;
	}
	if((name[5] >= 'a' && name[5] <= 'd'))//如果是小写字母
	{
		number = (name[4] - '0')*32 + (name[5] -'a')*8 + (name[6] - '0');
	}
	else if((name[5] >= 'A' && name[5] <= 'D')){//如果是大写字母
		number = (name[4] - '0')*32 + (name[5] -'A')*8 + (name[6] - '0');
	}
    (*env)->ReleaseStringUTFChars(env, gpioName, name);
	return number;
}
#ifdef FIRST_MOTHED_CONTROL_GPIO
/*******************************************************************************
标题：OpenGpioDev
功能：打开字符设备获取文件描述符
格式：
输入：无
输出：无
返回值:文件描述符
异常：暂没有发现
*******************************************************************************/
jint OpenGpioDev(JNIEnv* env,jobject thiz)
{
	int fd;
	fd = open("/dev/rk3288_gpio", O_RDWR);
	if (fd < 0) {
		LOGI("open /dev/rk3288_gpio err...!\n");
		return -1;
	}
	return fd;
}
/*******************************************************************************
标题：OpenGpioDev
功能：关闭字符设备以及文件描述符
格式：
输入：文件描述符
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
*******************************************************************************/
jint CloseGpioDev(JNIEnv* env,jobject thiz,jint fd)
{
	int ret=0;
	ret = close(fd);
	if (fd < 0) {
		LOGI("close, close err...!\n");
		ret=-1;
	}
	return ret;
}
/*******************************************************************************
标题：RequestGpio
功能：申请Gpio口
格式：
输入：fd：文件描述符 num:Gpio口引脚编号
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
*******************************************************************************/
jint RequestGpio(JNIEnv* env,jobject thiz,jint fd, jint num)
{
	int ret=-1;
	struct UserData userdata;
	memset(&userdata,0x00, sizeof(userdata));
	userdata.gpio=num;
	userdata.state=0;

	ret = ioctl(fd, RK3288_REQ_GPIO, &userdata);
	if(ret < 0)
		LOGI("request gpio err \n");
	return ret;
}
/*******************************************************************************
标题：ReleaseGpio
功能：释放Gpio口
格式：
输入：fd：文件描述符 num:Gpio口引脚编号
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
*******************************************************************************/
jint ReleaseGpio(JNIEnv* env,jobject thiz,jint fd, jint num)
{
	int ret=-1;
	struct UserData userdata;
	memset(&userdata,0x00, sizeof(userdata));
	userdata.gpio=num;
	userdata.state=0;
	ret = ioctl(fd, RK3288_REL_GPIO, &userdata);
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
jint SetGpioInput(JNIEnv* env,jobject thiz,jint fd, jint num)
{
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
/*******************************************************************************
标题：SetGpioOutput
功能：设置Gpio口为输出
格式：
输入：fd：文件描述符 num:Gpio口引脚编号
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
*******************************************************************************/
jint SetGpioOutput(JNIEnv* env,jobject thiz,jint fd, jint num, jint value)
{
    int err=-1;
    struct UserData userdata;
    memset(&userdata,0x00, sizeof(userdata));
    userdata.gpio=num;
    userdata.state=value;

    err = ioctl(fd, RK3288_OUT_GPIO, &userdata);
    if(err<0){
		LOGI("set gpio %d to output err!\n", num);
        err=-1;
    }
    return err;
}
/*******************************************************************************
标题：GetGpioState
功能：获取Gpio口的值
格式：
输入：fd：文件描述符 num:Gpio口引脚编号
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
*******************************************************************************/
jint GetGpioState(JNIEnv* env,jobject thiz,jint fd, jint num)
{
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

/*******************************************************************************
标题：SetGpioState
功能：设置Gpio口为输出并设置值
格式：
输入：fd：文件描述符 num:Gpio口引脚编号 state:引脚值
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
*******************************************************************************/
jint SetGpioState(JNIEnv* env,jobject thiz,jint fd, jint num, jint state)
{
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
/*******************************************************************************
标题：OpenKeyDev
功能：打开获取按键设备的节点
格式：
输入：无
输出：无
返回值:>0：正常 其他：异常
异常：暂没有发现
*******************************************************************************/
jint OpenKeyDev(JNIEnv* env,jobject thiz)
{
	int count = 0;
	int fd = 0;
	char devName[64] = {"/dev/input/event2"};
	#if 0
	char devName[64] = {0};
	char bufName[32] = {0};
	//获取相应的节点的名称编号
	for(count = 0;count<20;count++)
	{
		memset(bufName,0,sizeof(bufName));
		memset(devName,0,sizeof(devName));
		sprintf(devName,"/sys/class/input/input%d",count);
		fd = open(devName,O_RDONLY);//打开节点
		if(fd < 0)
		{
			LOGI("get %s name is failed!\n",devName);
			return -1;
		}
		if(read(fd,bufName,sizeof(bufName)) < 0)//获取节点名字
		{
			LOGI("read %s file name is failed!\n",devName);
			close(fd);
			return -1;
		}
		//bufName:gpio_keys.28
		if(!strstr(bufName,"gpio_keys"))//通过名字查找节点名称，不是想要的节点名称
		{
			close(fd);
			continue;
		}
		else//通过名字查找节点名称，是想要的节点名称
		{
			close(fd);
			break;
		}
	}
	memset(devName,0,sizeof(devName));
	sprintf(devName,"/dev/input/event%d",count);
	fd = open(devName,O_RDONLY);
	#endif
	fd = open(devName,O_RDONLY);
	if(fd < 0)
	{
		LOGI("open %s is failed !\n",devName);
		return -1;
	}
	LOGI("open %s is success aebell\n",devName);
	return fd;
	
}
/*******************************************************************************
标题：ReadKeyStatus
功能：读取按键值和状态
格式：
输入：fd：文件描述符 keyobj:按键值和状态传出结构体
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
*******************************************************************************/
jint ReadKeyStatus(JNIEnv* env,jobject thiz,jint fd, jobject keyobj)
{
	struct KeyState keystate;
	struct input_event t;  
	jclass clazz;
	jfieldID fid;
	
	memset(&keystate,0,sizeof(keystate));
	if(read(fd,&t,sizeof(t)) == sizeof(t))
		if(t.type == EV_KEY && (t.value == 0 || t.value == 1))
		{
			keystate.code = t.code;
			keystate.value = t.value;
			//mapping keystate of c to keyobj
			clazz = (*env)->GetObjectClass(env,keyobj);
			if(0 == clazz)
			{
				LOGI("GetObjectClass is failed!\n");
				return -1;
			}
			fid = (*env)->GetFieldID(env,clazz,"code","I");
			(*env)->SetIntField(env,keyobj,fid,keystate.code);
			
			fid = (*env)->GetFieldID(env,clazz,"value","I");
			(*env)->SetIntField(env,keyobj,fid,keystate.value);
			LOGI("code:%d,value:%d\n",t.code,t.value);
			return 0;
		}
	return -1;
}
/*******************************************************************************
标题：CloseKeyDev
功能：关闭获取按键设备的节点
格式：
输入：fd：文件描述符
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
*******************************************************************************/
jint CloseKeyDev(JNIEnv* env,jobject thiz,jint fd)
{
	close(fd);
	return 0;
}

/*******************************************************************************
标题：ControlMicInput
功能：控制mic头输入选择
格式：
输入：fd：文件描述符
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
*******************************************************************************/
jint ControlMicInput(JNIEnv* env,jobject thiz,jint state)
{
	char cmdBuff[64] = {0};
	sprintf(cmdBuff,"echo %d > /sys/class/es8323/mic_state",!!state);
	system(cmdBuff);
	return 0;
}

static JNINativeMethod GpioNativeMethods[] = {
	//gpio output control
    {"OpenGpioDev",			"()I",			        (void*)OpenGpioDev},
	{"CloseGpioDev",		"(I)I",			        (void*)CloseGpioDev},
	{"RequestGpio",			"(II)I",			    (void*)RequestGpio},
	{"ReleaseGpio",			"(II)I",			    (void*)ReleaseGpio},
	{"GetGpioNumber",       "(Ljava/lang/String;)I",(void*)GetGpioNumber},
	{"SetGpioInput",    	"(II)I",		        (void*)SetGpioInput},
	{"SetGpioOutput",		"(III)I",				(void*)SetGpioOutput},
	{"GetGpioState",		"(II)I",			    (void*)GetGpioState},
	{"SetGpioState",		"(III)I",		        (void*)SetGpioState},
	//key input
	{"OpenKeyDev",			"()I",					(void*)OpenKeyDev},
	{"ReadKeyStatus",		"(ILcom/aebell/jni/aebelltest/KeyObj;)I",(void*)ReadKeyStatus},
	{"CloseKeyDev",			"(I)I",					(void*)CloseKeyDev},
	//mic select
	{"ControlMicInput",		"(I)I",					(void*)ControlMicInput},
};

#else
/*******************************************************************************
标题：GpioIsInit
功能：判断Gpio口是否初始化
格式：
int GpioIsInit(int pin)
输入：pin:Gpio口引脚编号
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
*******************************************************************************/
static int GpioIsInit(int pin)
{
    char path[DIRECTION_MAX];
    snprintf(path,DIRECTION_MAX,"/sys/class/gpio/gpio%d",pin);
    if(access(path,F_OK) != 0)
    {
        return ERROTHER;
    }
    return ERROK;
}
/*******************************************************************************
标题：GpioInit
功能：Gpio口初始化
格式：
输入：pin:Gpio口引脚编号
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
*******************************************************************************/
jint GpioInit(JNIEnv* env,jobject thiz,jint pin)
{
    char buffer[BUFFER_MAX];
    int len;
    int fd;
    if(pin < 0)
        return ERRPARA;
    if(GpioIsInit(pin) != ERROK)
    {
        fd = open("/sys/class/gpio/export", O_WRONLY);
        if (fd < 0) {
            LOGE("%s:Failed to open export for writing!\n",__func__);
            return ERROPEN;
        }

        len = snprintf(buffer, BUFFER_MAX, "%d", pin);
        if (write(fd, buffer, len) < 0) {
            LOGE("%s:Fail to export gpio!\n",__func__);
            close(fd);
            return ERRWRITE;
        }
        close(fd);
    }
    return ERROK;
}
/*******************************************************************************
标题：GpioDestory
功能：Gpio口销毁
格式：
输入：pin:Gpio口引脚编号
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
*******************************************************************************/
jint GpioDestory(JNIEnv* env,jobject thiz,jint pin)
{
    char buffer[BUFFER_MAX];
    int len;
    int fd;
    if(pin < 0)
        return ERRPARA;
    if(GpioIsInit(pin) == ERROK)
    {
        fd = open("/sys/class/gpio/unexport", O_WRONLY);
        if (fd < 0) {
            LOGE("%s:Failed to open unexport for writing!\n",__func__);
            return ERROPEN;
        }

        len = snprintf(buffer, BUFFER_MAX, "%d", pin);
        if (write(fd, buffer, len) < 0) {
            LOGE("%s:Fail to unexport gpio!\n",__func__);
            close(fd);
            return ERRWRITE;
        }
        close(fd);
    }
    return ERROK;
}
/*******************************************************************************
标题：GetGpioDirection
功能：Gpio口的输入输出方向的获取
格式：
输入：pin:Gpio口引脚编号
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
*******************************************************************************/
jint GetGpioDirection(JNIEnv* env,jobject thiz,jint pin)
{
    char path[DIRECTION_MAX];
    char value_str[3] = {0};
    int fd;
    if(pin < 0)
        return ERRPARA;
    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        LOGE("%s:Failed to open gpio direction for writing!\n",__func__);
        return ERROPEN;
    }
    if (read(fd, value_str, 3) < 0) {
        LOGE("%s:Failed to read direction value!\n",__func__);
        close(fd);
        return ERRREAD;
    }

    close(fd);
    if(strcmp(value_str,"out") == 0)
        return 1;
    return 0;
}
/*******************************************************************************
标题：SetGpioDirection
功能：设置Gpio口的方向为输出
格式：
输入：pin:Gpio口引脚编号 dir:方向设置
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
*******************************************************************************/
jint SetGpioDirection(JNIEnv* env,jobject thiz,jint pin, jint dir)
{
    static const char dir_str[] = "in\0out";
    char path[DIRECTION_MAX];
    int fd;
    if((dir != 0 && dir != 1) || (pin < 0))
        return ERRPARA;
    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_WRONLY);
    if (fd < 0) {
        LOGE("%s:Failed to open gpio direction for writing!\n",__func__);
        return ERROPEN;
    }
    if (write(fd, &dir_str[dir == IN ? 0 : 3], dir == IN ? 2 : 3) < 0) {
        LOGE("%s:Failed to read direction!\n",__func__);
        close(fd);
        return ERRWRITE;
    }
    close(fd);
    return ERROK;
}
/*******************************************************************************
标题：GetGpioValue
功能：获取Gpio口的值
格式：
输入：pin:Gpio口引脚编号
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
*******************************************************************************/
jint GetGpioValue(JNIEnv* env,jobject thiz,jint pin)
{
    char path[DIRECTION_MAX];
    char value_str[3]={0};
    int fd;
    if(pin < 0)
        return ERRPARA;
    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        LOGE("%s:Failed to open gpio value for reading!\n",__func__);
        return ERROPEN;
    }

    if (read(fd, value_str, 3) < 0) {
        LOGE("%s:Failed to read value!\n",__func__);
        close(fd);
        return ERRREAD;
    }
    close(fd);
    return (atoi(value_str));
}
/*******************************************************************************
标题：SetGpioValue
功能：设置Gpio口的值
格式：
输入：pin:Gpio口引脚编号 value:引脚值
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
*******************************************************************************/
jint SetGpioValue(JNIEnv* env,jobject thiz,jint pin, jint value)
{
    static const char values_str[] = "01";
    char path[DIRECTION_MAX];
    int fd;
    if((value != 0 && value != 1) || (pin < 0))
        return ERRPARA;
    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_WRONLY);
    if (fd < 0) {
        LOGE("%s:Failed to open gpio value for writing!\n",__func__);
        return ERROPEN;
    }

    if (write(fd, &values_str[value == LOW ? 0 : 1], 1) < 0) {
        LOGE("%s:Failed to write value!\n",__func__);
        close(fd);
        return ERRWRITE;
    }
    close(fd);
    return ERROK;
}
static JNINativeMethod GpioNativeMethods[] = {
	{"GpioInit",			"(I)I",			        (void*)GpioInit},
	{"GpioDestory",			"(I)I",			        (void*)GpioDestory},
	{"GetGpioNumber",       "(Ljava/lang/String;)I",(void*)GetGpioNumber},
	{"GetGpioDirection",	"(I)I",			        (void*)GetGpioDirection},
	{"SetGpioDirection",	"(II)I",		        (void*)SetGpioDirection},
	{"GetGpioValue",		"(I)I",			        (void*)GetGpioValue},
	{"SetGpioValue",		"(II)I",		        (void*)SetGpioValue},
};
#endif



/*******************************************************************************
标题：JNI_OnLoad
功能：JNI库的加载
格式：
输入：
输出：
返回值:
异常：暂没有发现
*******************************************************************************/
jint JNI_OnLoad(JavaVM* pVm, void* reserved) {
	JNIEnv* env;
	if ((*pVm)->GetEnv(pVm,(void **)&env, JNI_VERSION_1_6) != JNI_OK) {
		 return -1;
	}

	jclass cls = (*env)->FindClass(env,GpioClassPathName);
	//Register methods with env->RegisterNatives.
	(*env)->RegisterNatives(env,cls, GpioNativeMethods, NELEM(GpioNativeMethods));
	return JNI_VERSION_1_6;
}
/*******************************************************************************
标题：JNI_OnUnLoad
功能：JNI库的加载
格式：
输入：
输出：
返回值:
异常：暂没有发现
*******************************************************************************/
void JNI_OnUnload(JavaVM* pVm, void* reserved){

	JNIEnv* env;
	if ((*pVm)->GetEnv(pVm, (void **)&env, JNI_VERSION_1_6) != JNI_OK) {
		 return;
	}

	jclass cls = (*env)->FindClass(env,GpioClassPathName);
	if(cls != NULL){
		(*env)->UnregisterNatives(env,cls);
	}
}

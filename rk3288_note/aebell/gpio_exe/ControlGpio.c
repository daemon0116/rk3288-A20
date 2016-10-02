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
int AebellGpioInit(int pin)
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
            DEBUGLOG_GPIO("%s:Failed to open export for writing!\n",__func__);
            return ERROPEN;
        }

        len = snprintf(buffer, BUFFER_MAX, "%d", pin);
        if (write(fd, buffer, len) < 0) {
            DEBUGLOG_GPIO("%s:Fail to export gpio!\n",__func__);
            close(fd);
            return ERRWRITE;
        }
        close(fd);
    }
    return ERROK;
}
int AebellGpioDestory(int pin)
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
            DEBUGLOG_GPIO("%s:Failed to open unexport for writing!\n",__func__);
            return ERROPEN;
        }

        len = snprintf(buffer, BUFFER_MAX, "%d", pin);
        if (write(fd, buffer, len) < 0) {
            DEBUGLOG_GPIO("%s:Fail to unexport gpio!\n",__func__);
            close(fd);
            return ERRWRITE;
        }
        close(fd);
    }
    return ERROK;
}
int AebellGetGpioDirection(int pin)
{
    char path[DIRECTION_MAX];
    char value_str[3] = {0};
    int fd;
    if(pin < 0)
        return ERRPARA;
    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        DEBUGLOG_GPIO("%s:Failed to open gpio direction for writing!\n",__func__);
        return ERROPEN;
    }
    if (read(fd, value_str, 3) < 0) {
        DEBUGLOG_GPIO("%s:Failed to read direction value!\n",__func__);
        close(fd);
        return ERRREAD;
    }

    close(fd);
    if(strcmp(value_str,"out") == 0)
        return 1;
    return 0;
}
int AebellSetGpioDirection(int pin, int dir)
{
    static const char dir_str[] = "in\0out";
    char path[DIRECTION_MAX];
    int fd;
    if((dir != 0 && dir != 1) || (pin < 0))
        return ERRPARA;
    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_WRONLY);
    if (fd < 0) {
        DEBUGLOG_GPIO("%s:Failed to open gpio direction for writing!\n",__func__);
        return ERROPEN;
    }
    if (write(fd, &dir_str[dir == IN ? 0 : 3], dir == IN ? 2 : 3) < 0) {
        DEBUGLOG_GPIO("%s:Failed to read direction!\n",__func__);
        close(fd);
        return ERRWRITE;
    }
    close(fd);
    return ERROK;
}
int AebellGetGpioValue(int pin)
{
    char path[DIRECTION_MAX];
    char value_str[3]={0};
    int fd;
    if(pin < 0)
        return ERRPARA;
    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_RDONLY);
    if (fd < 0) {
        DEBUGLOG_GPIO("%s:Failed to open gpio value for reading!\n",__func__);
        return ERROPEN;
    }

    if (read(fd, value_str, 3) < 0) {
        DEBUGLOG_GPIO("%s:Failed to read value!\n",__func__);
        close(fd);
        return ERRREAD;
    }
    close(fd);
    return (atoi(value_str));
}

int AebellSetGpioValue(int pin, int value)
{
    static const char values_str[] = "01";
    char path[DIRECTION_MAX];
    int fd;
    if((value != 0 && value != 1) || (pin < 0))
        return ERRPARA;
    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_WRONLY);
    if (fd < 0) {
        DEBUGLOG_GPIO("%s:Failed to open gpio value for writing!\n",__func__);
        return ERROPEN;
    }

    if (write(fd, &values_str[value == LOW ? 0 : 1], 1) < 0) {
        DEBUGLOG_GPIO("%s:Failed to write value!\n",__func__);
        close(fd);
        return ERRWRITE;
    }
    close(fd);
    return ERROK;
}
#if 0
//初始化gpio口
jint GpioInit(JNIEnv* env, jobject thiz, jint pin)
{
    return AebellGpioInit(pin);
}
//销毁gpio口
jint GpioDestory(JNIEnv* env, jobject thiz, jint pin)
{
    return AebellGpioDestory(pin);
}
//获取gpio口方向
jint GetGpioDirection(JNIEnv* env, jobject thiz, jint pin)
{
    return AebellGetGpioDirection(pin);
}
//设置gpio口方向
jint SetGpioDirection(JNIEnv* env, jobject thiz, jint pin,jint dir)
{
    return AebellSetGpioDirection(pin,dir);
}
//获取gpio口的值
jint GetGpioValue(JNIEnv* env, jobject thiz, jint pin)
{
    return AebellGetGpioValue(pin);
}
//设置gpio口的值
jint SetGpioValue(JNIEnv* env, jobject thiz, jint pin,jint value)
{
    return AebellSetGpioValue(pin,value);
}


#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))

static const char *GpioClassPathName = "com/aebell/jni/controlgpio/ControlGpio";

static JNINativeMethod GpioNativeMethods[] = {
		{"GpioInit",			"(I)I",			(void*)GpioInit},
        {"GpioDestory",			"(I)I",			(void*)GpioDestory},
        {"GetGpioDirection",	"(I)I",			(void*)GetGpioDirection},
        {"SetGpioDirection",	"(II)I",		(void*)SetGpioDirection},
        {"GetGpioValue",		"(I)I",			(void*)GetGpioValue},
        {"SetGpioValue",		"(II)I",		(void*)SetGpioValue},
};

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

#else
/* test demo */
int main(void)
{
    int pin = -1;
    int direction = -1;
    int value = -1;
    int res = -1;
    while(1){
        //输入引脚编号
        DEBUGLOG_GPIO("1.Please input Gpio num(0-288) :");
        scanf("%d",&pin);
        fflush(stdin);
        //引脚导出状态判断和配置
        res = AebellGpioInit(pin);
        if(res < 0) continue;
        //输入引脚设置的状态
        DEBUGLOG_GPIO("2.Please input Gpio direction(0:in 1:out):");
        scanf("%d",&direction);
        fflush(stdin);
        if(direction == 1)//输出
        {
            if(AebellGetGpioDirection(pin) == 0)//如果引脚为输入
            {
                res = AebellSetGpioDirection(pin,direction);
                if(res < 0) continue;
            }
            //输入引脚输出引脚的状态配置
            DEBUGLOG_GPIO("3.Please input Gpio state(0:low level or 1:high level):");
            scanf("%d",&value);
            fflush(stdin);
            res = AebellSetGpioValue(pin, value);
            if(res < 0) continue;
            DEBUGLOG_GPIO("4.Set pin:%d state:%d\n",pin,value);
        }
        else//输入
        {
            if(AebellGetGpioDirection(pin) == 1)//如果引脚为输出
            {
                res = AebellSetGpioDirection(pin,direction);
                if(res < 0) continue;
            }
            res = AebellGetGpioValue(pin);
            if(res < 0) continue;
            DEBUGLOG_GPIO("3.Get pin:%d state:%d\n",pin,res);
        }
    }
    return 0;
}
#endif

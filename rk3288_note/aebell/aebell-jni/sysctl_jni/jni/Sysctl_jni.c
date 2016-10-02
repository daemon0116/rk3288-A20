#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

#include "log.h"
#include "ControlGpio.h"

/*******************************************************************************
标题：GetGpioNumber
功能：通过Gpio口的名字获取Gpio引脚编号
格式：
输入：Gpio口名字
输出：Gpio口引脚编号
返回值:无
异常：暂没有发现
*******************************************************************************/
jint GetGpioNumber(JNIEnv* env,jobject thiz,jstring gpioName)
{
    int ret = 0;
    char *name =(char *)(*env)->GetStringUTFChars(env,gpioName, NULL);
    ret = AeGetGpioNumber(name);
    (*env)->ReleaseStringUTFChars(env,gpioName, name);
    return ret;
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
    return AeOpenGpioDev();
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
    return AeCloseGpioDev(fd);
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
    return AeRequestGpio(fd,num);
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
    return AeReleaseGpio(fd,num);
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
    return AeSetGpioInput(fd,num);
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
jint SetGpioOutput(JNIEnv* env,jobject thiz,jint fd, jint num,jint value)
{
    return AeSetGpioOutput(fd,num,value);
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
    return AeGetGpioState(fd,num);
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
    return AeSetGpioState(fd,num,state);
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
	return AeOpenKeyDev();
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
	if(read(fd,&t,sizeof(t))==sizeof(t))
		if(t.type == EV_KEY && (t.value == 0 || t.value == 1))
		{
			keystate.code = t.code;
			keystate.value = t.value;
			//mapping keystate of c to keyobj
			clazz = (*env)->GetObjectClass(env,keyobj);
			if(0 == clazz)
			{
				LOGE("GetObjectClass is failed!");
				return -1;
			}
			fid = (*env)->GetFieldID(env,clazz,"code","I");
			(*env)->SetIntField(env,keyobj,fid,keystate.code);
			
			fid = (*env)->GetFieldID(env,clazz,"value","I");
			(*env)->SetIntField(env,keyobj,fid,keystate.value);
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
	return AeCloseKeyDev(fd);
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
	return AeControlMicInput(state);
}
/*******************************************************************************
标题：ControlSystemWatchDog
功能：控制系统看门狗
格式：
输入：fd：文件描述符
输出：无
返回值:0：正常 其他：异常
异常：暂没有发现
*******************************************************************************/
jint ControlSystemWatchDog(JNIEnv* env,jobject thiz,jint fd,jint state)
{
	return AeControlSystemWatchDog(fd,state);
}
#else
//注意该部分的实现暂时没有在文件系统中开启对gpio口的操作权限，如果需要进行操作的话，需要修改文件系统的权限方式
jint GpioInit(JNIEnv* env,jobject thiz,jint pin)
{
    return AeGpioInit(pin);
}
jint GpioDestory(JNIEnv* env,jobject thiz,jint pin)
{
    return AeGpioDestory(pin);
}
jint GetGpioDirection(JNIEnv* env,jobject thiz,jint pin);
{
    return AeGetGpioDirection(pin);
}
jint SetGpioDirection(JNIEnv* env,jobject thiz,jint pin, jint dir)
{
    return AeSetGpioDirection(pin,dir);
}
jint GetGpioValue(JNIEnv* env,jobject thiz,jint pin)
{
    return AeGetGpioValue(pin);
}
jint SetGpioValue(JNIEnv* env,jobject thiz,jint pin, jint value)
{
    return AeSetGpioValue(pin,value);
}
#endif
/*******************************************************************************
标题：jniGetLogServerIP
功能：上层调用打印日志设置
格式：
输入：jip  : 远程打印服务器ip
      port : 远程打印端口号
	  flag : 1：开启远程打印; 0: 不开启远程打印，即用otg打印
输出：
返回值:无
异常：暂没有发现
*******************************************************************************/
jint jniGetLogServerIP(JNIEnv* env, jobject thiz, jstring jip, int port, int flag)
{
    char *ip;

    ip = (char*)(*env)->GetStringUTFChars(env, jip, 0);
    InitLog(ip, port, flag);
	(*env)->ReleaseStringUTFChars(env, jip, ip);

    return 0;
}

/*******************************************************************************
jniLogFromJava
功能：java 调用打印日志功能
格式：
输入：str   : 要打印的内容
      level : 打印等级
输出：
返回值:无
异常：暂没有发现
*******************************************************************************/
jint jniLogFromJava(JNIEnv* env, jobject thiz, jstring str, int level)
{
    char *nStr;

    nStr = (char*)(*env)->GetStringUTFChars(env, str, 0);
    switch(level)
	{
		case LOG_VERBOSE :
		    LOGV(nStr);
			break;
		case LOG_DEBUG :
			LOGD(nStr);
			break;
		case LOG_INFO :
			LOGI(nStr);
			break;
		case LOG_WARNING :
			LOGW(nStr);
			break;
		case LOG_ERROR :
			LOGE(nStr);
			break;
		case LOG_FATAL :
			LOGF(nStr);
			break;
		//case LOG_SILENT :
		//	break;
		default :
		    break;
	}
	(*env)->ReleaseStringUTFChars(env, str, nStr);

    return 0;
}

#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
static const char *GpioClassPathName = "com/aebell/app/nvis/platform/sysctljni/ControlGpio";
static const char *LogClassPathName = "com/aebell/app/nvis/platform/sysctljni/JniLog";

#ifdef FIRST_MOTHED_CONTROL_GPIO
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
	{"ReadKeyStatus",		"(ILcom/aebell/app/nvis/platform/sysctljni/KeyObj;)I",(void*)ReadKeyStatus},
	{"CloseKeyDev",			"(I)I",					(void*)CloseKeyDev},
	//mic select
	{"ControlMicInput",		"(I)I",					(void*)ControlMicInput},
	//control watchdog
	{"ControlSystemWatchDog","(II)I",				(void*)ControlSystemWatchDog},
};
#else
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
static JNINativeMethod LogNativeMethods[] = {
    {"get_logserver_ip",	"(Ljava/lang/String;II)I",                 (void *)jniGetLogServerIP},
	{"log_from_java",		"(Ljava/lang/String;I)I",                  (void *)jniLogFromJava},
};

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

	jclass cls1 = (*env)->FindClass(env,GpioClassPathName);
	//Register methods with env->RegisterNatives.
	(*env)->RegisterNatives(env,cls1, GpioNativeMethods, NELEM(GpioNativeMethods));
    
	jclass cls2 = (*env)->FindClass(env,LogClassPathName);
	//Register methods with env->RegisterNatives.
	(*env)->RegisterNatives(env,cls2, LogNativeMethods, NELEM(LogNativeMethods));
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

	jclass cls1 = (*env)->FindClass(env,GpioClassPathName);
	if(cls1 != NULL){
		(*env)->UnregisterNatives(env,cls1);
	}
    
	jclass cls2 = (*env)->FindClass(env,LogClassPathName);
	if(cls2 != NULL){
		(*env)->UnregisterNatives(env,cls2);
	}
}





#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>

#include "log.h"
#include "ControlGpio.h"

/*******************************************************************************
���⣺GetGpioNumber
���ܣ�ͨ��Gpio�ڵ����ֻ�ȡGpio���ű��
��ʽ��
���룺Gpio������
�����Gpio�����ű��
����ֵ:��
�쳣����û�з���
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
���⣺OpenGpioDev
���ܣ����ַ��豸��ȡ�ļ�������
��ʽ��
���룺��
�������
����ֵ:�ļ�������
�쳣����û�з���
*******************************************************************************/
jint OpenGpioDev(JNIEnv* env,jobject thiz)
{
    return AeOpenGpioDev();
}
/*******************************************************************************
���⣺OpenGpioDev
���ܣ��ر��ַ��豸�Լ��ļ�������
��ʽ��
���룺�ļ�������
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
jint CloseGpioDev(JNIEnv* env,jobject thiz,jint fd)
{
    return AeCloseGpioDev(fd);
}
/*******************************************************************************
���⣺RequestGpio
���ܣ�����Gpio��
��ʽ��
���룺fd���ļ������� num:Gpio�����ű��
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
jint RequestGpio(JNIEnv* env,jobject thiz,jint fd, jint num)
{
    return AeRequestGpio(fd,num);
}
/*******************************************************************************
���⣺ReleaseGpio
���ܣ��ͷ�Gpio��
��ʽ��
���룺fd���ļ������� num:Gpio�����ű��
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
jint ReleaseGpio(JNIEnv* env,jobject thiz,jint fd, jint num)
{
    return AeReleaseGpio(fd,num);
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
jint SetGpioInput(JNIEnv* env,jobject thiz,jint fd, jint num)
{
    return AeSetGpioInput(fd,num);
}
/*******************************************************************************
���⣺SetGpioOutput
���ܣ�����Gpio��Ϊ���
��ʽ��
���룺fd���ļ������� num:Gpio�����ű��
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
jint SetGpioOutput(JNIEnv* env,jobject thiz,jint fd, jint num,jint value)
{
    return AeSetGpioOutput(fd,num,value);
}
/*******************************************************************************
���⣺GetGpioState
���ܣ���ȡGpio�ڵ�ֵ
��ʽ��
���룺fd���ļ������� num:Gpio�����ű��
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
jint GetGpioState(JNIEnv* env,jobject thiz,jint fd, jint num)
{
    return AeGetGpioState(fd,num);
}
/*******************************************************************************
���⣺SetGpioState
���ܣ�����Gpio��Ϊ���������ֵ
��ʽ��
���룺fd���ļ������� num:Gpio�����ű�� state:����ֵ
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
jint SetGpioState(JNIEnv* env,jobject thiz,jint fd, jint num, jint state)
{
    return AeSetGpioState(fd,num,state);
}
/*******************************************************************************
���⣺OpenKeyDev
���ܣ��򿪻�ȡ�����豸�Ľڵ�
��ʽ��
���룺��
�������
����ֵ:>0������ �������쳣
�쳣����û�з���
*******************************************************************************/
jint OpenKeyDev(JNIEnv* env,jobject thiz)
{
	return AeOpenKeyDev();
}
/*******************************************************************************
���⣺ReadKeyStatus
���ܣ���ȡ����ֵ��״̬
��ʽ��
���룺fd���ļ������� keyobj:����ֵ��״̬�����ṹ��
�������
����ֵ:0������ �������쳣
�쳣����û�з���
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
���⣺CloseKeyDev
���ܣ��رջ�ȡ�����豸�Ľڵ�
��ʽ��
���룺fd���ļ�������
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
jint CloseKeyDev(JNIEnv* env,jobject thiz,jint fd)
{
	return AeCloseKeyDev(fd);
}
/*******************************************************************************
���⣺ControlMicInput
���ܣ�����micͷ����ѡ��
��ʽ��
���룺fd���ļ�������
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
jint ControlMicInput(JNIEnv* env,jobject thiz,jint state)
{
	return AeControlMicInput(state);
}
/*******************************************************************************
���⣺ControlSystemWatchDog
���ܣ�����ϵͳ���Ź�
��ʽ��
���룺fd���ļ�������
�������
����ֵ:0������ �������쳣
�쳣����û�з���
*******************************************************************************/
jint ControlSystemWatchDog(JNIEnv* env,jobject thiz,jint fd,jint state)
{
	return AeControlSystemWatchDog(fd,state);
}
#else
//ע��ò��ֵ�ʵ����ʱû�����ļ�ϵͳ�п�����gpio�ڵĲ���Ȩ�ޣ������Ҫ���в����Ļ�����Ҫ�޸��ļ�ϵͳ��Ȩ�޷�ʽ
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
���⣺jniGetLogServerIP
���ܣ��ϲ���ô�ӡ��־����
��ʽ��
���룺jip  : Զ�̴�ӡ������ip
      port : Զ�̴�ӡ�˿ں�
	  flag : 1������Զ�̴�ӡ; 0: ������Զ�̴�ӡ������otg��ӡ
�����
����ֵ:��
�쳣����û�з���
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
���ܣ�java ���ô�ӡ��־����
��ʽ��
���룺str   : Ҫ��ӡ������
      level : ��ӡ�ȼ�
�����
����ֵ:��
�쳣����û�з���
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
���⣺JNI_OnLoad
���ܣ�JNI��ļ���
��ʽ��
���룺
�����
����ֵ:
�쳣����û�з���
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
���⣺JNI_OnUnLoad
���ܣ�JNI��ļ���
��ʽ��
���룺
�����
����ֵ:
�쳣����û�з���
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





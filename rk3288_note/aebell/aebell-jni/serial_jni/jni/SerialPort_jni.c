
#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "log.h"

#include "Finger.h"
#include "SerialPort.h"

#include <pthread.h>



#if 1
jint serial_open(JNIEnv* env, jobject thiz, jstring filename, jint eBaudRate, jint eDataBits, jint eStopBits, jint eParityCheck)
{
	int fd;

    char* storage_path = (char*)(*env)->GetStringUTFChars(env, filename, 0);
    
    fd = SerialOpen(storage_path, eBaudRate, eDataBits, eStopBits, eParityCheck);
    (*env)->ReleaseStringUTFChars(env,filename,storage_path);
    if(fd <= 0){
        return -1;
    }

	return fd;
}

jint serial_close(JNIEnv *env, jobject thiz, jint fd)
{
	close(fd);
    
    return 0;
}


jint serial_select(JNIEnv *env, jobject thiz, jint fd, jint timeout)
{
    int ret;
    int maxfd;
    fd_set read_fds;
    struct timeval TimeoutVal;


    maxfd = fd+1;
    
    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);

    if(timeout >= 0){
        TimeoutVal.tv_sec  = timeout/1000000;
        TimeoutVal.tv_usec = timeout%1000000;
        ret = select(maxfd, &read_fds, NULL, NULL, &TimeoutVal);
    }
    else{
        ret = select(maxfd, &read_fds, NULL, NULL, NULL);
    }
    
    if(ret <= 0)
    {
        return 0;
    }
    if(FD_ISSET(fd, &read_fds))
    {
        return 1;
    }
	return 0;
}

jint serial_write(JNIEnv *env, jobject thiz, jint fd, jbyteArray jin, jint jsize)
{
	int nRet;
    
	jbyte* jin_array = (*env)->GetByteArrayElements(env,jin,0);
    
    nRet = write(fd, jin_array, jsize);

	(*env)->ReleaseByteArrayElements(env,jin,jin_array,0);

	return nRet;
}

jint serial_read(JNIEnv *env, jobject thiz, jint fd, jbyteArray jout, jint jsize)
{
	int nRet;
    
    jbyte* jout_array = (*env)->GetByteArrayElements(env,jout,0);

    nRet = read(fd, jout_array, jsize);

    (*env)->ReleaseByteArrayElements(env,jout,jout_array,0);

    return nRet;
}

#endif


#if 1
jint finger_open(JNIEnv* env, jobject thiz, jstring filename, jint eBaudRate, jint eDataBits, jint eStopBits, jint eParityCheck)//´ò¿ªÖ¸ÎÆÒÇ
{
    jint ret;
    char* storage_path = (char*)(*env)->GetStringUTFChars(env, filename, 0);
    
    ret = FingerOpen(storage_path, eBaudRate, eDataBits, eStopBits, eParityCheck);
    (*env)->ReleaseStringUTFChars(env,filename,storage_path);
    return ret;
}


jint finger_close(JNIEnv* env, jobject thiz)
{
    return FingerClose();
}

jint finger_add(JNIEnv* env, jobject thiz, jint nId, jbyteArray pData)
{
    jint ret;
        
    jbyte* jdata_array = (*env)->GetByteArrayElements(env,pData,0);
    
    ret = FingerAdd(nId, jdata_array);
    (*env)->ReleaseByteArrayElements(env,pData,jdata_array,0);
    return ret;
}

jint finger_delete(JNIEnv* env, jobject thiz, jint nId)
{    
    return FingerDelete(nId);
}

jint finger_clean(JNIEnv* env, jobject thiz)
{    
    return FingerClean();
}

jint finger_check(JNIEnv* env, jobject thiz, jint *nId)
{    
    return FingerCheck(nId);
}

jint finger_set_timeout(JNIEnv* env, jobject thiz, jint timeoutVal)
{    
    return FingerSetTimeout(timeoutVal);
}

#endif

# define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))



//static const char *FingerClassPathName = "com/example/administrator/myapplication/SerialPort";
//static const char *FingerClassPathName = "com/aebell/app/ipvi/platform/brocast/SerialPort";
static const char *FingerClassPathName = "com/aebell/finger/jni/SerialPort";


static JNINativeMethod FingerNativeMethods[] = {
    {"serial_open",         "(Ljava/lang/String;IIII)I",                        (void*)serial_open},
    {"serial_close",        "(I)I",                                             (void*)serial_close},
    {"serial_select",       "(II)I",                                            (void*)serial_select},
    {"serial_write",         "(I[BI)I",                                         (void*)serial_write},
    {"serial_read",          "(I[BI)I",                                         (void*)serial_read},
    #if 1
    {"finger_open",         "(Ljava/lang/String;IIII)I", (void*)finger_open},
    {"finger_close",        "()I",              (void*)finger_close},
    {"finger_add",          "(I[B)I",           (void*)finger_add},
    {"finger_delete",       "(I)I",             (void*)finger_delete},
    {"finger_clean",        "()I",              (void*)finger_clean},
    {"finger_check",        "([I)I",            (void*)finger_check},
    {"finger_set_timeout",  "(I)I",             (void*)finger_set_timeout},
    #endif
};

jint JNI_OnLoad(JavaVM* pVm, void* reserved) {
	JNIEnv* env;
	if ((*pVm)->GetEnv(pVm,(void **)&env, JNI_VERSION_1_6) != JNI_OK) {
		 return -1;
	}

	jclass cls = (*env)->FindClass(env,FingerClassPathName);
	//Register methods with env->RegisterNatives.
	(*env)->RegisterNatives(env,cls, FingerNativeMethods, NELEM(FingerNativeMethods));
	return JNI_VERSION_1_6;
}
void JNI_OnUnload(JavaVM* pVm, void* reserved){

	JNIEnv* env;
	if ((*pVm)->GetEnv(pVm, (void **)&env, JNI_VERSION_1_6) != JNI_OK) {
		 return;
	}

	jclass cls = (*env)->FindClass(env,FingerClassPathName);
	if(cls != NULL){
		(*env)->UnregisterNatives(env,cls);
	}
}

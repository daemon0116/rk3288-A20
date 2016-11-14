
#include<stdlib.h>
#include<pthread.h>
#include<jni.h>
#include "MyJni.h"

#include "log.h"

#include "AudioPlayer.h"
#include "NetWork.h"
#include "main.h"

//0未初始化，1已初始化，-1正在反初始化中
BOOL jniStartFlag = 0;

JavaVM *m_vm;
jobject mobj;

jmethodID PlayCallMid;
jmethodID NetCmdCallMid;
jmethodID NetBcCallMid;
jmethodID SetNetworkCallMid;



jint jniPlay(JNIEnv* env, jobject thiz, jbyteArray array, jint length, jstring fileList, jint times, jint defVol)
{
    int ret;
    uint8 *buf;
    char* paths;

    if(jniStartFlag != 1){
        return -1;
    }
    
    buf = (uint8 *)(*env)->GetByteArrayElements(env, array, NULL);
    paths = (char*)(*env)->GetStringUTFChars(env, fileList, 0);

    if(0)
    {
        int i;
        char buff[4*1024];
        for(i=0; i<length&&i<sizeof(buff)/3; i++){
            sprintf(buff+i*3, "%02X ", buf[i]);
        }
        LOGV("devList length=%d", length);
        LOGV("play devList=%s", buff);
    }

    ret = AllocPlayTask(PLAY_FILE, paths, buf, length, times, defVol);

    (*env)->ReleaseStringUTFChars(env, fileList, paths);
    (*env)->ReleaseByteArrayElements(env, array, buf, 0);
    
    return ret;
}

jint jniRecord(JNIEnv* env, jobject thiz, jbyteArray array, jint length, jint defVol, jint flag)
{
    int ret;
    uint8 *buf;
    char* paths;

    if(jniStartFlag != 1){
        return -1;
    }

    buf = (uint8 *)(*env)->GetByteArrayElements(env, array, NULL);

    ret = AllocPlayTask(flag==1?PLAY_RECORD_MIC:PLAY_RECORD_LINE, " ", buf, length, 1, defVol);

    (*env)->ReleaseByteArrayElements(env, array, buf, 0);
    
    return ret;
}


jint jniPlayCtrl(JNIEnv* env, jobject thiz, jint id, jint state)
{
    jint ret;

    if(jniStartFlag != 1){
        return -1;
    }

    ret = HandPlayTaskStatus(id, state);

    return ret;
}

jint jniSetVol(JNIEnv* env, jobject thiz, jint id, jint vol)
{
    jint ret;

    if(jniStartFlag != 1){
        return -1;
    }

    ret = HandSetVol(id, vol);
    return ret;   
}

jint jniSetMute(JNIEnv* env, jobject thiz, jint isMute)
{
    jint ret;

    if(jniStartFlag != 1){
        return -1;
    }

    ret = HandSetMute(isMute);
    return ret;   
}

jint jniTellVoipStatus(JNIEnv* env, jobject thiz, int event, int state, jstring SipNum)
{
    char* str;
    
    if(jniStartFlag != 1){
        return -1;
    }

    LOGD("event=%d, state=%d", event, state);

    uint8 status = pSysInfo->VoipStatus;
    
    if(event == CALL_IN || event == CALL_OUT){
        if(state){
            status |= (event==CALL_OUT ? 1:2);
        }
        else{
            status &= ~(0x03);
        }
    }
    else if(event == MEETING_IN || event == MEETING_OUT){
        if(state){
            status |= ((event==MEETING_OUT ? 1:2)<<2);
        }
        else{
            status &= ~(0x03<<2);
        }
    }
    else if(event == MONITOR_IN || event == MONITOR_OUT){
        if(state){
            status |= ((event==MONITOR_OUT ? 1:2)<<4);
        }
        else{
            status &= ~(0x03<<4);
        }
    }
    else{
        return -1;
    }

    pSysInfo->VoipStatus = status;
    if(state){
        str = (char*)(*env)->GetStringUTFChars(env, SipNum, 0);
        strncpy(pSysInfo->SipNum, str, sizeof(pSysInfo->SipNum));
        (*env)->ReleaseStringUTFChars(env, SipNum, str);
    }
    
    return 0;
}

jint jniNetSend(JNIEnv* env, jobject thiz, jstring jdata, jstring jip, jint to)
{
    char* ip;
    char* pdata;

    if(jniStartFlag != 1){
        return -1;
    }

    ip = (char*)(*env)->GetStringUTFChars(env, jip, 0);
    pdata = (char*)(*env)->GetStringUTFChars(env, jdata, 0);

    //LinkAddData(&SendLink, str, strlen(str)+1);

    //LOGD("jniNetSend net-->%s,=%s", ip, pdata);

    if(to == 1){
        SendToSvr(pdata, SEND_NO_WAIT);
    }
    else{
        SendToDev(ip, pdata, SEND_NO_WAIT);
    }

    (*env)->ReleaseStringUTFChars(env, jip, ip);
    (*env)->ReleaseStringUTFChars(env, jdata, pdata);
    
    return 0;
}

jint jniGetDevIP(JNIEnv* env, jobject thiz, jint nums, jbyteArray ipList)
{
    int i;
    uint8 *buf;
    pIP_ADDR pDevIP;

    if(jniStartFlag != 1){
        return -1;
    }

    if(nums <= 0){
        return -1;
    }
    
    pDevIP = (pIP_ADDR)_malloc(nums*sizeof(IP_ADDR));
    if(!pDevIP){
        return -1;
    }

    buf = (uint8 *)(*env)->GetByteArrayElements(env, ipList, NULL);
    for(i=0; i<nums; i++){
        memcpy(&pDevIP[i].ip, buf+i*sizeof(pDevIP[0].ip), sizeof(pDevIP[0].ip));
        //将点分格式ip转换存一份网络格式，不用在发送时再转换，提高效率。
        pDevIP[i].addr = inet_addr(pDevIP[i].ip);
        pDevIP[i].isMC = 0;
        if(i<10)LOGV("--------------->%s", pDevIP[i].ip);
    }
    (*env)->ReleaseByteArrayElements(env, ipList, buf, 0);

    pSysInfo->DevNums = nums;
    if(!pSysInfo->pDevIP){
        pSysInfo->pDevIP = pDevIP;
    }
    else{
        buf = (uint8 *)pSysInfo->pDevIP;
        pSysInfo->pDevIP = pDevIP;
        if(!buf){
            _free(buf);
        }
    }

    return 0;
}

jint jniGetLocalInfo(JNIEnv* env, jobject thiz, jstring jstr)
{
    int ret;
    char *str;

    if(jniStartFlag != 1){
        return -1;
    }

    str = (char*)(*env)->GetStringUTFChars(env, jstr, 0);
    LOGV("jniGetLocalInfo:%s", str);
    ret = GetLocalInfo(str);
    (*env)->ReleaseStringUTFChars(env, jstr, str);

    return ret;
}

jint jniGetLogServerIP(JNIEnv* env, jobject thiz, jstring jip, int port, int flag)
{
    char *ip;

    if(jniStartFlag != 1){
        return -1;
    }

    ip = (char*)(*env)->GetStringUTFChars(env, jip, 0);
    memcpy(pSysInfo->LogServerIP, ip, sizeof(pSysInfo->LogServerIP));
    pSysInfo->LogServerPort = port;
    pSysInfo->LogFlag = flag;
    (*env)->ReleaseStringUTFChars(env, jip, ip);

    InitLog(pSysInfo->LogServerIP, pSysInfo->LogServerPort, pSysInfo->LogFlag);

    return 0;
}

JNIEnv* getJNIEnv(int* needsDetach)
{
    JNIEnv *env = NULL;
    if ((*m_vm)->GetEnv(m_vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK){
        int status = (*m_vm)->AttachCurrentThread(m_vm, &env, 0);
        if (status < 0){
            return NULL;
        }
        if(needsDetach){
            *needsDetach = 1;
        }
    }
    return env;
}

void PlayCallJava(int id, int event, const char *file)
{
    int needsDetach;
    
    JNIEnv *env = getJNIEnv(&needsDetach);

    jstring jstr = (*env)->NewStringUTF(env, file);
    (*env)->CallVoidMethod(env, mobj, PlayCallMid, id, event, jstr);
    (*env)->DeleteLocalRef(env, jstr);
}

int NetCmdCallJava(const char *data)
{
    int needsDetach;

    //LOGV("-->%s",  data);
    
    JNIEnv *env = getJNIEnv(&needsDetach);

    jstring jstr = (*env)->NewStringUTF(env, data);
    (*env)->CallVoidMethod(env, mobj, NetCmdCallMid, jstr);
    (*env)->DeleteLocalRef(env, jstr);   
}

int SetNetworkCallJava(const char *data)
{
    int ret;
    int needsDetach;

    JNIEnv *env = getJNIEnv(&needsDetach);

    jstring jstr = (*env)->NewStringUTF(env, data);
    ret = (*env)->CallIntMethod(env, mobj, SetNetworkCallMid, jstr);
    (*env)->DeleteLocalRef(env, jstr);

    LOGV("%s", data);

    return ret;
}

int NetBcCallJava(int id, int SrcNum, int state, const char *name)
{
    int needsDetach;
  
    JNIEnv *env = getJNIEnv(&needsDetach);

    jstring jstr = (*env)->NewStringUTF(env, name);
    (*env)->CallVoidMethod(env, mobj, NetBcCallMid, id, SrcNum, state, jstr);
    (*env)->DeleteLocalRef(env, jstr);
}

static void jniRegCallBack(JNIEnv *env, jobject object)
{
    jclass objclass;

    objclass=(*env)->GetObjectClass(env, object);
    mobj=(*env)->NewGlobalRef(env, object);
    PlayCallMid = (*env)->GetMethodID(env, objclass,    "play_callback",     "(IILjava/lang/String;)V");
    NetCmdCallMid  = (*env)->GetMethodID(env, objclass, "net_recv_callback", "(Ljava/lang/String;)V");
    NetBcCallMid  = (*env)->GetMethodID(env, objclass,  "net_bc_callback",   "(IIILjava/lang/String;)V");
    SetNetworkCallMid  = (*env)->GetMethodID(env, objclass,  "set_network_callback", "(Ljava/lang/String;)I");
}


jint jniStart(JNIEnv* env, jobject thiz, jint DevNum)
{
    jint ret;

    if(jniStartFlag == 1){
        return 0;
    }
    if(jniStartFlag == -1){
        return -1;
    }
    
    LOGD("{BC_jni_start}");
    ret = InitApp(DevNum);
    if(ret == 0){
        jniStartFlag = 1;
    }
    return ret;
}

jint jniEnd(JNIEnv* env, jobject thiz, jint mask)
{
    if(jniStartFlag != 1){
        return jniStartFlag; //0未初始化，1已初始化，-1正在反初始化中
    }
    jniStartFlag = -1;
    DelInitApp(mask);
    jniStartFlag = 0;

    LOGD("{BC_jni_end}");
    return 0;
}

//JNI下线程退出要调用这个函数
void jThreadDetach(void)
{
    (*m_vm)->DetachCurrentThread(m_vm);
}

#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))



static const JNINativeMethod gMethods[] = {
	{"reg_callback",			"()V",                                      (void *)jniRegCallBack},
    {"jni_start",		        "(I)I",			                            (void *)jniStart},
    {"jni_end",		            "(I)I",			                            (void *)jniEnd},
	{"bc_file_start",			"([BILjava/lang/String;II)I",		        (void *)jniPlay},
    {"bc_mic_start",			"([BIII)I",			                        (void *)jniRecord},
    {"play_ctrl",		        "(II)I",			                        (void *)jniPlayCtrl},
    {"bc_set_vol",		        "(II)I",			                        (void *)jniSetVol},
    {"bc_set_mute",		        "(I)I",			                            (void *)jniSetMute},
    {"tell_voip_status",		"(IILjava/lang/String;)I",			        (void *)jniTellVoipStatus},
    {"net_send",		        "(Ljava/lang/String;Ljava/lang/String;I)I", (void *)jniNetSend},  
    {"get_dev_ip",		        "(I[B)I",                                   (void *)jniGetDevIP},
    {"get_local_info",		    "(Ljava/lang/String;)I",                    (void *)jniGetLocalInfo},
    {"get_logserver_ip",		"(Ljava/lang/String;II)I",                  (void *)jniGetLogServerIP},
};  


static const char *CLASS_NAME = "com/aebell/app/nvis/platform/bcjni/BcJNI";

jint JNI_OnLoad(JavaVM* pVm, void* reserved) {
	JNIEnv* env;

    m_vm = pVm;
    
	if ((*pVm)->GetEnv(pVm,(void **)&env, JNI_VERSION_1_6) != JNI_OK) {
		 return -1;
	}

	jclass cls = (*env)->FindClass(env, CLASS_NAME);
	
	(*env)->RegisterNatives(env,cls, gMethods, NELEM(gMethods));
	return JNI_VERSION_1_6;
}
void JNI_OnUnload(JavaVM* pVm, void* reserved){

	JNIEnv* env;
	if ((*pVm)->GetEnv(pVm, (void **)&env, JNI_VERSION_1_6) != JNI_OK) {
		 return;
	}

	jclass cls = (*env)->FindClass(env, CLASS_NAME);
	if(cls != NULL){
		(*env)->UnregisterNatives(env, cls);
	}
}


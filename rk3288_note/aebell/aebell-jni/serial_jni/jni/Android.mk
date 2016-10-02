LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := SerialPort
LOCAL_LDLIBS += -llog
LOCAL_SRC_FILES := Command.c Finger.c SerialPort.c SerialPort_jni.c
include $(BUILD_SHARED_LIBRARY)
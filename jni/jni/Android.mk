LOCAL_PATH := $(call my-dir)

#ffmpeg libs start
include $(CLEAR_VARS)
LOCAL_MODULE := avcodec
LOCAL_SRC_FILES := ffmpeglib/lib/libavcodec.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avformat
LOCAL_SRC_FILES := ffmpeglib/lib/libavformat.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avutil
LOCAL_SRC_FILES := ffmpeglib/lib/libavutil.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := swresample
LOCAL_SRC_FILES := ffmpeglib/lib/libswresample.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := swscale
LOCAL_SRC_FILES := ffmpeglib/lib/libswscale.a
include $(PREBUILT_STATIC_LIBRARY)
#ffmpeg libs end


#osip libs start
include $(CLEAR_VARS)
LOCAL_MODULE := osip2
LOCAL_SRC_FILES := osiplib/lib/libosip2.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := osipparser2
LOCAL_SRC_FILES := osiplib/lib/libosipparser2.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := eXosip2
LOCAL_SRC_FILES := osiplib/lib/libeXosip2.a
include $(PREBUILT_STATIC_LIBRARY)
#osip libs end

include $(CLEAR_VARS)
LOCAL_MODULE := log4cpp
LOCAL_SRC_FILES := log4lib/lib/liblog4cpp.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_CPP_EXTENSION := .cpp
include $(CLEAR_VARS)
LOCAL_SRC_FILES := wavfile.c AudioPlayer.c FFmpegApi.c OpenSlApi.c ringbuffer.c linkbuffer.c g711.c\
				   UdpApp.c threadpool.c NetWork.c main.c jni.c sip.c osip.c SysConfig.c log.c common.c
LOCAL_LDLIBS += -llog -lm -lz -landroid -lOpenSLES
LOCAL_MODULE := bc
LOCAL_C_INCLUDES += $(LOCAL_PATH)/ffmpeglib/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/osiplib/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/log4lib/include

#avformat需放在最前面
LOCAL_STATIC_LIBRARIES :=avformat avcodec swresample swscale avutil eXosip2 osip2 osipparser2

include $(BUILD_SHARED_LIBRARY)
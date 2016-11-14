//
// Created by Jonesx on 2016/3/19.
//
#ifndef FFMPEG_API_H
#define FFMPEG_API_H

#include <stdio.h>

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/samplefmt.h"

#include "common.h"
#include "OpenSlApi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STREAM_BUF_SIZE     (2*1024)
#define OUTPUT_BUF_SIZE     (6*1024);

typedef struct
{
    int audioStream;
    AVPacket packet;
    AVFrame *aFrame;
    SwrContext *swr;
    AVFormatContext *aFormatCtx;
    AVCodecContext *aCodecCtx;
    size_t outputBufferSize;
    uint8_t *outputBuffer;
    uint8_t *stream_buf;
}FFMPEG,*pFFMPEG;

pFFMPEG FFmpegCreateAudioPlay(pFFMPEG *pFFinfo, const char *file_name, pPCM_PARAM pPCM);
int FFmpegReleaseAudioPlay(pFFMPEG pFFinfo);
int FFmpegAudioDecode(pFFMPEG pFFinfo, void **pcm);
int FFmpegReadFrame(pFFMPEG pFFinfo, void *data, int size);


#ifdef __cplusplus
}
#endif

#endif //AUDIOPLAYER_FFMPEGAUDIOPLAY_H

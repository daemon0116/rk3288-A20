

#include <fcntl.h>

#include "log.h"

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/samplefmt.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include "AudioPlayer.h"
#include "FFmpegApi.h"


//注册av_read_frame的回调函数...

int FFmpegReleaseAudioPlay(pFFMPEG pFFinfo)
{
    if(pFFinfo){
        av_packet_unref(&pFFinfo->packet);
        if(pFFinfo->outputBuffer){
            _free(pFFinfo->outputBuffer);
            pFFinfo->outputBuffer = NULL;
        }
        av_free(pFFinfo->aFrame);
        //if(pFFinfo->stream_buf){
        //    av_free(pFFinfo->stream_buf);//stream_buf在avformat_close_input中释放
        //}
        if(pFFinfo->swr){
            swr_free(&pFFinfo->swr);
        }
        avcodec_close(pFFinfo->aCodecCtx);
        avformat_close_input(&pFFinfo->aFormatCtx);
        _free(pFFinfo);
        pFFinfo = NULL;
    }
    return 0;
}


pFFMPEG FFmpegCreateAudioPlay(pFFMPEG *pFF, const char *file_name, pPCM_PARAM pPCM)
{
    pFFMPEG pFFM;
    AVIOContext * pb = NULL;
    AVInputFormat *piFmt = NULL;

    if(!(pFFM = (pFFMPEG)_malloc(sizeof(FFMPEG)))){
        LOGW("pFF_INFO malloc fail");
        return NULL;
    }
    memset(pFFM, 0, sizeof(FFMPEG));

    if(pFF){
        *pFF = pFFM;
    }
    
    av_register_all();

    pFFM->aFormatCtx = avformat_alloc_context();
    pFFM->aFormatCtx->probesize = STREAM_BUF_SIZE;//STREAM_BUF_SIZE;//这个用来控制接收到多少数据才进行码流解析！
    pFFM->aFormatCtx->max_analyze_duration = 2 * AV_TIME_BASE;

	if(!file_name){ //file_name为空表示音频数据不是由ff直接从文件读取
		pFFM->stream_buf = av_mallocz(STREAM_BUF_SIZE);
        //注册av_read_frame取数据的回调函数
	    pb = avio_alloc_context(pFFM->stream_buf, STREAM_BUF_SIZE, 0, NULL, GetStreamData, NULL, NULL);
	    if (!pb) {
	        LOGV("avio alloc failed!");
	        goto err;
	    }
        #if 1
	    //探测流格式
	    if (av_probe_input_buffer(pb, &piFmt, "", NULL, 0, STREAM_BUF_SIZE) < 0) {
	        LOGV("probe failed!");
	        goto err;
	    } else {
	        LOGV("probe success format: %s[%s]", piFmt->name, piFmt->long_name);
	    }
        #endif
	    pFFM->aFormatCtx->pb = pb;
	}

    // Open audio file
    if (avformat_open_input(&pFFM->aFormatCtx, file_name?file_name:"", piFmt, NULL) != 0) {
        LOGW("Couldn't open file:%s\n", file_name);
        goto err; // Couldn't open file
    }

    // Retrieve stream information
    if (avformat_find_stream_info(pFFM->aFormatCtx, NULL) < 0) {
        LOGW("Couldn't find stream information.");
        goto err;
    }

    // Find the first audio stream
    int i;
    pFFM->audioStream = -1;
    for (i = 0; i < pFFM->aFormatCtx->nb_streams; i++) {
        if (pFFM->aFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO &&
            pFFM->audioStream < 0) {
            pFFM->audioStream = i;
        }
    }
    if (pFFM->audioStream == -1) {
        LOGW("Couldn't find audio stream!");
        goto err;
    }

    // Get a pointer to the codec context for the video stream
    pFFM->aCodecCtx = pFFM->aFormatCtx->streams[pFFM->audioStream]->codec;

    // Find the decoder for the audio stream
    AVCodec *aCodec = avcodec_find_decoder(pFFM->aCodecCtx->codec_id);
    if (!aCodec) {
        LOGW("Unsupported codec!\n");
        goto err;
    }

    if (avcodec_open2(pFFM->aCodecCtx, aCodec, NULL) < 0) {
        LOGW("Could not open codec.");
        goto err; // Could not open codec
    }

    pFFM->aFrame = av_frame_alloc();
    if(!pFFM->aFrame){
        goto err;
    }

    pFFM->swr = swr_alloc();
    if(!pFFM->swr){
        goto err;
    }
    av_opt_set_int(pFFM->swr, "in_channel_layout",  pFFM->aCodecCtx->channel_layout, 0);
    av_opt_set_int(pFFM->swr, "out_channel_layout", pFFM->aCodecCtx->channel_layout,  0);
    av_opt_set_int(pFFM->swr, "in_sample_rate",     pFFM->aCodecCtx->sample_rate, 0);
    av_opt_set_int(pFFM->swr, "out_sample_rate",    pFFM->aCodecCtx->sample_rate, 0);
    av_opt_set_sample_fmt(pFFM->swr, "in_sample_fmt",  pFFM->aCodecCtx->sample_fmt, 0);
    av_opt_set_sample_fmt(pFFM->swr, "out_sample_fmt", AV_SAMPLE_FMT_S16,  0);
    swr_init(pFFM->swr);

    pFFM->outputBufferSize = OUTPUT_BUF_SIZE;
    pFFM->outputBuffer = (uint8_t *) _malloc(pFFM->outputBufferSize);

    av_init_packet(&pFFM->packet);

    LOGD("rate=%d, channels=%d, sample_fmt=%d", pFFM->aCodecCtx->sample_rate, pFFM->aCodecCtx->channels, pFFM->aCodecCtx->sample_fmt);
    if(!pFFM->aCodecCtx->sample_rate || !pFFM->aCodecCtx->channels){
        goto err;
    }
    
    if(pPCM){
        pPCM->rate = pFFM->aCodecCtx->sample_rate;
        pPCM->channels = pFFM->aCodecCtx->channels;
        pPCM->bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
    }
    
    return pFFM;

    err:
    FFmpegReleaseAudioPlay(pFFM);
    if(pFF){
        *pFF = NULL;
    }
    
    return NULL;
}


int FFmpegAudioDecode(pFFMPEG pFFinfo, void **pcm)
{
    while (av_read_frame(pFFinfo->aFormatCtx, &pFFinfo->packet) >= 0) 
    {
        int frameFinished = 0;
        
        // Is this a packet from the audio stream?
        if (pFFinfo->packet.stream_index == pFFinfo->audioStream) {
            avcodec_decode_audio4(pFFinfo->aCodecCtx, pFFinfo->aFrame, &frameFinished, &pFFinfo->packet);

            if (frameFinished) {
                int data_size = av_samples_get_buffer_size(
                        pFFinfo->aFrame->linesize, pFFinfo->aCodecCtx->channels,
                        pFFinfo->aFrame->nb_samples, pFFinfo->aCodecCtx->sample_fmt, 1);

                if (data_size > pFFinfo->outputBufferSize) {
                    LOGD("old size=%d, new size=%d", pFFinfo->outputBufferSize, data_size);
                    uint8_t *p = (uint8_t *) realloc(pFFinfo->outputBuffer, pFFinfo->outputBufferSize);
                    if(!p){
                        av_free_packet(&pFFinfo->packet);
                        continue;
                    }
                    pFFinfo->outputBufferSize = data_size;
                }

                swr_convert(pFFinfo->swr, &pFFinfo->outputBuffer, pFFinfo->aFrame->nb_samples,
                           (uint8_t const **) (pFFinfo->aFrame->extended_data), pFFinfo->aFrame->nb_samples);

                *pcm = pFFinfo->outputBuffer;
                av_free_packet(&pFFinfo->packet);
                return data_size;
            }
        }
        av_free_packet(&pFFinfo->packet);
    }
    return 0;
}


int FFmpegReadFrame(pFFMPEG pFFinfo, void *data, int size) 
{
    int len;
    
    while (av_read_frame(pFFinfo->aFormatCtx, &pFFinfo->packet) >= 0) 
    {        
        // Is this a packet from the audio stream?
        if (pFFinfo->packet.stream_index == pFFinfo->audioStream){
            if(data){
                len = pFFinfo->packet.size>size ? size:pFFinfo->packet.size;
                memcpy(data, pFFinfo->packet.data, len);
            }
            av_free_packet(&pFFinfo->packet);
            return len;
        }
        av_free_packet(&pFFinfo->packet);
    }
    return 0;
}


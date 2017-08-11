//
// Created by Saqrag Borgn on 05/07/2017.
//
#include "Compress.h"
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
#include "libavformat/avformat.h"
#include "../Log.h"
#include "Progress.h"


static volatile int mIsCancel = 0;

static char *getRotate(AVStream *inStream) {
    AVDictionaryEntry *tag = NULL;
    tag = av_dict_get(inStream->metadata, "rotate", tag, 0);
    if (tag == NULL) {
        return "0";
    } else {
        int angle = atoi(tag->value);
        angle %= 360;
        if (angle == 90) {
            return "90";
        } else if (angle == 180) {
            return "180";
        } else if (angle == 270) {
            return "270";
        } else {
            return "0";
        }
    }
}

static void setProgress(AVPacket *outPacket, AVFormatContext *outFormatContext, int *outVideoIndex,
                        AVFormatContext *inFormatContext, char *inFile) {
    long pts = outPacket->pts;
    long num = outFormatContext->streams[*outVideoIndex]->time_base.num;
    long den = outFormatContext->streams[*outVideoIndex]->time_base.den;
    long nowTime = (long long) pts * 1000 * num / den;
    long duration = inFormatContext->duration / 1000;
    float percent = (float) (nowTime * 100 / (double) duration);
    updateProgress(inFile, percent);
}

static int initInput(
        const char *inFile,
        AVFormatContext **inFormatContext,
        AVCodecContext **inVideoCodecContext,
        AVCodecContext **inAudioCodecContext,
        int *inVideoIndex,
        int *inAudioIndex
) {

    int ret;

    //1. 获取AVFormatContext
    *inFormatContext = avformat_alloc_context();
    ret = avformat_open_input(inFormatContext, inFile, NULL, NULL);
    if (ret != 0) {
        LOGE("open input file: %s error!!", inFile);
        return -1;
    }

    //2. 查找AVStream信息，并填充到AVFormatContext
    ret = avformat_find_stream_info(*inFormatContext, NULL);
    if (ret < 0) {
        LOGE("find input AVStream error");
        return -2;
    }

    //3. 获取 StreamIndex
    for (int i = 0; i < (*inFormatContext)->nb_streams; ++i) {
        enum AVMediaType type = (*inFormatContext)->streams[i]->codecpar->codec_type;
        switch (type) {
            case AVMEDIA_TYPE_VIDEO:
                *inVideoIndex = i;
                *inVideoCodecContext = (*inFormatContext)->streams[i]->codec;
                (*inVideoCodecContext)->thread_count = 0;
                AVCodec *videoCodec = avcodec_find_decoder(
                        (*inFormatContext)->streams[i]->codecpar->codec_id);
                avcodec_open2(*inVideoCodecContext, videoCodec, NULL);
                break;
            case AVMEDIA_TYPE_AUDIO:
                *inAudioIndex = i;
                *inAudioCodecContext = (*inFormatContext)->streams[i]->codec;
                AVCodec *audioCodec = avcodec_find_decoder(
                        (*inFormatContext)->streams[i]->codecpar->codec_id);
                avcodec_open2(*inAudioCodecContext, audioCodec, NULL);
                break;
            default:
                break;
        }
    }

    if (*inVideoIndex == -1) {
        LOGD("not found video Stream");
        return -3;
    }
    if (*inAudioIndex == -1) {
        LOGD("not found audio Stream");
        return -4;
    }
    return 0;
}

static int initOutput(
        const char *outFile,
        AVCodecContext *inVideoCodecContext,
        AVCodecContext *inAudioCodecContext,
        AVFormatContext **outFormatContext,
        AVCodecContext **outVideoCodecContext,
        AVCodecContext **outAudioCodecContext,
        int *outVideoIndex,
        int *outAudioIndex,
        long videoBitRate,
        long audioBitRate,
        int width,
        int height,
        char *rotate,
        int threadCount
) {

    int ret;

    //1. 创建AVFormat
    ret = avformat_alloc_output_context2(outFormatContext, NULL, NULL, outFile);
    if (ret < 0) {
        LOGE("alloc output format context error!");
        return -100;
    }

    //2. 创建AVIOContext
    ret = avio_open(&(*outFormatContext)->pb, outFile, AVIO_FLAG_READ_WRITE);
    if (ret < 0) {
        LOGE("open output AVIOContext error");
        return -101;
    }

    int audioThreadCount = threadCount / 2;
    int videoThreadCount = threadCount - audioThreadCount;
    if (audioThreadCount == 0) {
        audioThreadCount = 2;
        videoThreadCount = 3;
    }

    AVStream *outStream;
    for (int i = 0; i < 2; ++i) {
        //3. 创建video AVStream
        outStream = avformat_new_stream(*outFormatContext, NULL);
        outStream->id = (*outFormatContext)->nb_streams - 1;

        if (i == 0) {
            *outVideoIndex = outStream->id;

            //4. 配置AVCodecContext
            *outVideoCodecContext = outStream->codec;
            (*outVideoCodecContext)->flags = AV_CODEC_FLAG_GLOBAL_HEADER;
            (*outVideoCodecContext)->bit_rate = videoBitRate;
            (*outVideoCodecContext)->gop_size = 250;
//            (*outVideoCodecContext)->thread_count = videoThreadCount;
            (*outVideoCodecContext)->thread_count = 0;
            (*outVideoCodecContext)->time_base.num = inVideoCodecContext->time_base.num;
            (*outVideoCodecContext)->time_base.den = inVideoCodecContext->time_base.den;
            (*outVideoCodecContext)->max_b_frames = 0;
            (*outVideoCodecContext)->codec_id = AV_CODEC_ID_H264;
            (*outVideoCodecContext)->codec_type = AVMEDIA_TYPE_VIDEO;
            (*outVideoCodecContext)->pix_fmt = AV_PIX_FMT_YUV420P;
            (*outVideoCodecContext)->width = width;
            (*outVideoCodecContext)->height = height;

            //H264
            (*outVideoCodecContext)->me_range = 16;
            (*outVideoCodecContext)->max_qdiff = 4;
            (*outVideoCodecContext)->qcompress = 0.6;
            (*outVideoCodecContext)->qmin = 10;
            (*outVideoCodecContext)->qmax = 51;

            av_dict_set(&(*outFormatContext)->streams[*outVideoIndex]->metadata, "rotate", rotate,
                        0);

            // Set Option
            AVDictionary *param = 0;
            //H.264
            if ((*outVideoCodecContext)->codec_id == AV_CODEC_ID_H264) {
                av_opt_set((*outVideoCodecContext)->priv_data, "preset", "ultrafast", 0);
                av_dict_set(&param, "profile", "baseline", 0);
            }

            //Show some Information
            av_dump_format(*outFormatContext, 0, outFile, 1);

            //5. 配置AVCodec
            AVCodec *outCodec = NULL;
            outCodec = avcodec_find_encoder((*outVideoCodecContext)->codec_id);
            if (outCodec == NULL) {
                LOGE("find AVCodec error!");
                return -103;
            }
            if (outCodec->capabilities & AV_CODEC_CAP_DELAY) {
                LOGE("out codec capabilities true");
            }
            ret = avcodec_open2(*outVideoCodecContext, outCodec, &param);
            if (ret != 0) {
                LOGE("open codec error");
                return -104;
            }
        } else {
            *outAudioIndex = outStream->id;

            //1. find encoder
            AVCodec *outCodec = NULL;
            outCodec = avcodec_find_encoder(inAudioCodecContext->codec_id);
            if (outCodec == NULL) {
                LOGE("find audio AVCodec error!");
                return -105;
            }

            //2. setting encoder context
            *outAudioCodecContext = outStream->codec;
            (*outAudioCodecContext)->flags = AV_CODEC_FLAG_GLOBAL_HEADER;
            (*outAudioCodecContext)->codec_id = inAudioCodecContext->codec_id;
            (*outAudioCodecContext)->codec_type = inAudioCodecContext->codec_type;
            (*outAudioCodecContext)->frame_size = inAudioCodecContext->frame_size;
            (*outAudioCodecContext)->thread_count = 0;
//            (*outAudioCodecContext)->thread_count = audioThreadCount;
            (*outAudioCodecContext)->channel_layout = inAudioCodecContext->channel_layout;
            if (outCodec->channel_layouts) {
                (*outAudioCodecContext)->channel_layout = outCodec->channel_layouts[0];
                for (int a = 0; outCodec->channel_layouts[a]; a++) {
                    if (outCodec->channel_layouts[a] == inAudioCodecContext->channel_layout) {
                        (*outAudioCodecContext)->channel_layout = inAudioCodecContext->channel_layout;
                    }
                }
            }
            (*outAudioCodecContext)->channels = av_get_channel_layout_nb_channels(
                    (*outAudioCodecContext)->channel_layout
            );
            (*outAudioCodecContext)->sample_fmt = inAudioCodecContext->sample_fmt;
            if (outCodec->sample_fmts) {
                (*outAudioCodecContext)->sample_fmt = outCodec->sample_fmts[0];
                for (int b = 0; outCodec->sample_fmts[b]; b++) {
                    if (outCodec->sample_fmts[b] == inAudioCodecContext->sample_fmt) {
                        (*outAudioCodecContext)->sample_fmt = inAudioCodecContext->sample_fmt;
                    }
                }
            }
            LOGD("channel layout: %ld", (*outAudioCodecContext)->channel_layout);
            LOGD("channels :%d", (*outAudioCodecContext)->channels);
            LOGD("sample_fmts: %d", (*outAudioCodecContext)->sample_fmt);
            (*outAudioCodecContext)->sample_rate = inAudioCodecContext->sample_rate;
            (*outAudioCodecContext)->bit_rate = audioBitRate;

//          Show some Information
            av_dump_format(*outFormatContext, 0, outFile, 1);

            ret = avcodec_open2(*outAudioCodecContext, outCodec, NULL);
            if (ret != 0) {
                LOGE("open audio codec error, CODE: %d", ret);
                return -106;
            }
        }
    }
    return 0;
}

static int initAudioFrame(
        AVCodecContext *inAudioCodecContext,
        AVCodecContext *outAudioCodecContext,
        AVFrame **outAudioFrame,
        SwrContext **swrContext
) {

    int ret;

    *outAudioFrame = av_frame_alloc();
    if (*outAudioFrame == NULL) {
        LOGE("alloc out audio frame error");
        return -500;
    }

    *swrContext = swr_alloc_set_opts(
            NULL,
            outAudioCodecContext->channel_layout,
            outAudioCodecContext->sample_fmt,
            outAudioCodecContext->sample_rate,
            inAudioCodecContext->channel_layout,
            inAudioCodecContext->sample_fmt,
            inAudioCodecContext->sample_rate,
            0,
            NULL
    );

    if (!*swrContext) {
        LOGE("create mSwrContext error");
        return -502;
    }

    ret = swr_init(*swrContext);
    if (ret < 0) {
        LOGE("init SwrContext error, CODE: %d", ret);
        return -503;
    }

    return 0;
}

static int transCodeVideo(
        AVFormatContext *inFormatContext,
        AVFormatContext *outFormatContext,
        AVCodecContext *inVideoCodecContext,
        AVCodecContext *outVideoCodecContext,
        AVPacket *inPacket,
        AVPacket *outPacket,
        AVFrame *inFrame,
        AVFrame *outVideoFrame,
        char *inFile,
        struct SwsContext *swsContext,
        int *inVideoIndex,
        int *outVideoIndex
) {

    int ret;

    uint8_t *out_buffer = (unsigned char *) av_malloc((size_t) av_image_get_buffer_size(
            AV_PIX_FMT_YUV420P,
            outVideoCodecContext->width,
            outVideoCodecContext->height,
            1
    ));
    av_image_fill_arrays(
            outVideoFrame->data,
            outVideoFrame->linesize,
            out_buffer,
            AV_PIX_FMT_YUV420P,
            outVideoCodecContext->width,
            outVideoCodecContext->height,
            1
    );

    LOGD("------------------------------start video trans code-------------------------------------");

    LOGD(
            ">>>>in stream time base num: %d den: %d<<<<",
            inFormatContext->streams[*inVideoIndex]->time_base.num,
            inFormatContext->streams[*inVideoIndex]->time_base.den
    );

    LOGD(">>>>in packet pts: %lld dts: %lld<<<<", inPacket->pts, inPacket->dts);

    inPacket->pts = av_rescale_q(
            inPacket->pts,
            inFormatContext->streams[*inVideoIndex]->time_base,
            inVideoCodecContext->time_base
    );
    inPacket->dts = inPacket->pts;

    LOGD("1111111111111111111111111111111111");

    //1. 解码
    ret = avcodec_send_packet(inVideoCodecContext, inPacket);
    if (ret != 0) {
        LOGE("send decode packet error");
        return -200;
    }

    LOGD("22222222222222222222222222222222222");

    ret = avcodec_receive_frame(inVideoCodecContext, inFrame);
    if (ret != 0) {
        LOGE("receive decode frame error");
        return -201;
    }

    LOGD(
            ">>>>in codec context time base num: %d den: %d<<<<",
            inVideoCodecContext->time_base.num,
            inVideoCodecContext->time_base.den
    );

    LOGD(">>>>in frame pts: %lld<<<<", inFrame->pts);
    sws_scale(
            swsContext,
            (const uint8_t *const *) inFrame->data,
            inFrame->linesize, 0,
            inFrame->height,
            outVideoFrame->data,
            outVideoFrame->linesize
    );
    outVideoFrame->format = inFrame->format;
    outVideoFrame->width = outVideoCodecContext->width;
    outVideoFrame->height = outVideoCodecContext->height;
    outVideoFrame->pts = av_rescale_q(inFrame->pts, inVideoCodecContext->time_base,
                                      outVideoCodecContext->time_base);

    LOGD(
            "<<<<out codec context time base num: %d den: %d>>>>",
            outVideoCodecContext->time_base.num,
            outVideoCodecContext->time_base.den
    );

    LOGD("<<<<out frame pts: %lld>>>>", outVideoFrame->pts);


    ret = avcodec_send_frame(outVideoCodecContext, outVideoFrame);
    if (ret != 0) {
        LOGE("send encode frame error, CODE: %d", ret);
        return -203;
    }

    LOGD("33333333333333333333333");

    outPacket->data = NULL;
    outPacket->size = 0;

    ret = avcodec_receive_packet(outVideoCodecContext, outPacket);
    if (ret != 0) {
        LOGE("receive encode packet error, CODE: %d", ret);
        return -203;
    }

    LOGD("444444444444444444444444444");

    outPacket->pts = av_rescale_q(
            outPacket->pts,
            outVideoCodecContext->time_base,
            outFormatContext->streams[*outVideoIndex]->time_base
    );
    outPacket->dts = outPacket->pts;
    outPacket->stream_index = *outVideoIndex;

    LOGD(
            "<<<<out stream time base num: %d den: %d>>>>",
            outFormatContext->streams[*outVideoIndex]->time_base.num,
            outFormatContext->streams[*outVideoIndex]->time_base.den
    );

    LOGD("<<<<out packet pts: %lld dts: %lld>>>>", outPacket->pts, outPacket->dts);

    setProgress(outPacket, outFormatContext, outVideoIndex, inFormatContext, inFile);

    //3. 写入 AVFormatContext
    ret = av_interleaved_write_frame(outFormatContext, outPacket);

    if (ret < 0) {
        LOGE("write frame error, CODE: %d", ret);
        return -204;
    }

    LOGD("------------------------------end video trans code-------------------------------------");
    av_free(out_buffer);
    return 0;
}

static int transCodeAudio(
        AVFormatContext *inFormatContext,
        AVFormatContext *outFormatContext,
        AVCodecContext *inAudioCodecContext,
        AVCodecContext *outAudioCodecContext,
        AVPacket *inPacket,
        AVPacket *outPacket,
        AVFrame *inFrame,
        AVFrame *outAudioFrame,
        SwrContext *swrContext,
        int *inAudioIndex,
        int *outAudioIndex
) {

    int ret;

    LOGD("------------------------------start audio trans code-------------------------------------");

    LOGD(
            ">>>>in stream time base num: %d den: %d<<<<",
            inFormatContext->streams[*inAudioIndex]->time_base.num,
            inFormatContext->streams[*inAudioIndex]->time_base.den
    );

    LOGD(">>>>in packet pts: %lld dts: %lld<<<<", inPacket->pts, inPacket->dts);

    inPacket->pts = av_rescale_q(
            inPacket->pts,
            inFormatContext->streams[*inAudioIndex]->time_base,
            inAudioCodecContext->time_base
    );
    inPacket->dts = inPacket->pts;

    //1. 解码
    ret = avcodec_send_packet(inAudioCodecContext, inPacket);
    if (ret != 0) {
        LOGE("send decode packet error");
        return -200;
    }

    ret = avcodec_receive_frame(inAudioCodecContext, inFrame);
    if (ret != 0) {
        LOGE("receive decode frame error");
        return -201;
    }

    LOGD(
            ">>>>in codec context time base num: %d den: %d<<<<",
            inAudioCodecContext->time_base.num,
            inAudioCodecContext->time_base.den
    );

    LOGD(">>>>in frame pts: %lld<<<<", inFrame->pts);

    int tempOutAudioSampleCount = (int) av_rescale_rnd(
            swr_get_delay(swrContext, inAudioCodecContext->sample_rate) +
            inFrame->nb_samples,
            outAudioCodecContext->sample_rate,
            inAudioCodecContext->sample_rate,
            AV_ROUND_UP
    );

    int channelCount = av_get_channel_layout_nb_channels(outAudioCodecContext->channel_layout);
    ret = av_samples_alloc(
            outAudioFrame->data,
            outAudioFrame->linesize,
            channelCount,
            tempOutAudioSampleCount,
            outAudioCodecContext->sample_fmt,
            0
    );
    if (ret < 0) {
        LOGD("alloc out audio frame sample error, CODE: %d", ret);
        return -202;
    }

    ret = swr_convert(
            swrContext,
            outAudioFrame->data,
            outAudioFrame->nb_samples,
            (const uint8_t **) inFrame->data,
            inFrame->nb_samples
    );
    if (ret < 0) {
        LOGD("swr_convert error: %d", ret);
        return -203;
    }

    outAudioFrame->pts = av_rescale_q(
            inFrame->pts,
            inAudioCodecContext->time_base,
            outAudioCodecContext->time_base
    );
    outAudioFrame->nb_samples = tempOutAudioSampleCount;
    outAudioFrame->channel_layout = outAudioCodecContext->channel_layout;
    outAudioFrame->format = outAudioCodecContext->sample_fmt;
    outAudioFrame->sample_rate = outAudioCodecContext->sample_rate;

    ret = avcodec_send_frame(outAudioCodecContext, outAudioFrame);
    if (ret != 0) {
        LOGE("send encode frame error, CODE: %d", ret);
        return -203;
    }

    outPacket->data = NULL;
    outPacket->size = 0;

    ret = avcodec_receive_packet(outAudioCodecContext, outPacket);
    if (ret != 0) {
        LOGE("receive encode packet error, CODE: %d", ret);
        return -203;
    }

    av_freep(outAudioFrame->data);

    outPacket->pts = av_rescale_q(
            outPacket->pts,
            outAudioCodecContext->time_base,
            outFormatContext->streams[*outAudioIndex]->time_base
    );
    outPacket->dts = outPacket->pts;
    outPacket->stream_index = *outAudioIndex;

    LOGD(
            "<<<<out stream time base num: %d den: %d>>>>",
            outFormatContext->streams[*outAudioIndex]->time_base.num,
            outFormatContext->streams[*outAudioIndex]->time_base.den
    );

    LOGD("<<<<out packet pts: %lld dts: %lld>>>>", outPacket->pts, outPacket->dts);

//3. 写入 AVFormatContext
    ret = av_interleaved_write_frame(outFormatContext, outPacket);

    if (ret < 0) {
        LOGE("write frame error, CODE: %d", ret);
        return -204;
    }

    LOGD("------------------------------end audio trans code-------------------------------------");
    return 0;
}

static int flushEncoder(
        AVFormatContext *outFormatContext,
        AVCodecContext *outCodecContext,
        AVPacket *outPacket,
        int streamIndex,
        AVFormatContext *inFormatContext,
        char *inFile
) {
    int ret;
    if (!(outCodecContext->codec->capabilities & CODEC_CAP_DELAY)) {
        return 0;
    }

    ret = avcodec_send_frame(outCodecContext, NULL);
    if (ret != 0) {
        LOGD("send frame error, CODE: %d", ret);
        return 0;
    }

    while (1) {
        outPacket->data = NULL;
        outPacket->size = 0;
        av_init_packet(outPacket);


        ret = avcodec_receive_packet(outCodecContext, outPacket);
        if (ret != 0) {
            LOGD("receive packet error, CODE: %d", ret);
            break;
        }

        LOGD("------------------------------start %d mux----------------------", streamIndex);
        LOGD(
                "<<<<out stream time base num: %d den: %d>>>>",
                outFormatContext->streams[streamIndex]->time_base.num,
                outFormatContext->streams[streamIndex]->time_base.den
        );
        LOGD("<<<<out packet pts: %lld dts: %lld>>>>", outPacket->pts, outPacket->dts);

        outPacket->pts = av_rescale_q(
                outPacket->pts,
                outCodecContext->time_base,
                outFormatContext->streams[streamIndex]->time_base
        );
        outPacket->dts = outPacket->pts;
        outPacket->stream_index = streamIndex;


        LOGD(
                "<<<<out stream time base num: %d den: %d>>>>",
                outFormatContext->streams[streamIndex]->time_base.num,
                outFormatContext->streams[streamIndex]->time_base.den
        );

        if (inFile != NULL) {
            setProgress(outPacket, outFormatContext, &streamIndex, inFormatContext, inFile);
        }

        LOGD("<<<<out packet pts: %lld dts: %lld>>>>", outPacket->pts, outPacket->dts);
        ret = av_interleaved_write_frame(outFormatContext, outPacket);
        LOGD("------------------------------end %d mux----------------------", streamIndex);

        if (ret != 0) {
            LOGD("write frame error");
            break;
        }

        LOGD("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", outPacket->size);
    }
    return 0;
}

int compress(
        const char *inFilename,
        const char *outFilename,
        long videoBitRate,
        long audioBitRate,
        int width,
        int height,
        int threadCount
) {

    LOGW("inFile:%s,outFile:%s,videoBitrate:%ld,audioBitrate:%ld,width:%d,height:%d,threadCount:%d",
         inFilename, outFilename, videoBitRate, audioBitRate, width, height, threadCount);

    int ret;
    AVFormatContext *inFormatContext = NULL, *outFormatContext = NULL;
    AVCodecContext *inVideoCodecContext, *outVideoCodecContext = NULL, *inAudioCodecContext, *outAudioCodecContext = NULL;
    AVPacket *inPacket = NULL, *outPacket = NULL;
    AVFrame *inFrame = NULL, *outVideoFrame = NULL, *outAudioFrame = NULL;
    int inVideoIndex = -1, inAudioIndex = -1, outVideoIndex = -1, outAudioIndex = -1;
    struct SwsContext *swsContext = NULL;
    struct SwrContext *swrContext = NULL;

    mIsCancel = 1;

//1. 注册
    av_register_all();

//2. 初始化input
    ret = initInput(
            inFilename,
            &inFormatContext,
            &inVideoCodecContext,
            &inAudioCodecContext,
            &inVideoIndex,
            &inAudioIndex
    );
    if (ret != 0) {
        LOGE("init input File error!!");
        goto end;
    }

    LOGE("rotate: %s", getRotate(inFormatContext->streams[inVideoIndex]));

//3. 初始化output
    ret = initOutput(
            outFilename,
            inVideoCodecContext,
            inAudioCodecContext,
            &outFormatContext,
            &outVideoCodecContext,
            &outAudioCodecContext,
            &outVideoIndex,
            &outAudioIndex,
            videoBitRate,
            audioBitRate,
            width,
            height,
            getRotate(inFormatContext->streams[inVideoIndex]),
            threadCount
    );
    if (ret != 0) {
        LOGE("init output file error");
        goto end;
    }

//4. AVPacket 申请内存
    inPacket = av_packet_alloc();
    outPacket = av_packet_alloc();


//5. AVFrame 申请内存
    inFrame = av_frame_alloc();
    outVideoFrame = av_frame_alloc();

//6. 写头
    avformat_write_header(outFormatContext, NULL);

    swsContext = sws_getContext(
            inVideoCodecContext->width,
            inVideoCodecContext->height,
            AV_PIX_FMT_YUV420P,
            outVideoCodecContext->width,
            outVideoCodecContext->height,
            AV_PIX_FMT_YUV420P,
            SWS_FAST_BILINEAR,
            NULL,
            NULL,
            NULL
    );
    if (swsContext == NULL) {
        LOGE("getSwsContext error");
        goto end;
    }

    initAudioFrame(inAudioCodecContext, outAudioCodecContext, &outAudioFrame, &swrContext);

    while (av_read_frame(inFormatContext, inPacket) == 0) {
        if (mIsCancel == 0) break;

        if (inPacket->stream_index == inVideoIndex) {
            transCodeVideo(
                    inFormatContext,
                    outFormatContext,
                    inVideoCodecContext,
                    outVideoCodecContext,
                    inPacket,
                    outPacket,
                    inFrame,
                    outVideoFrame,
                    (char *) inFilename,
                    swsContext,
                    &inVideoIndex,
                    &outVideoIndex);
        } else if (inPacket->stream_index == inAudioIndex) {
            transCodeAudio(
                    inFormatContext,
                    outFormatContext,
                    inAudioCodecContext,
                    outAudioCodecContext,
                    inPacket,
                    outPacket,
                    inFrame,
                    outAudioFrame,
                    swrContext,
                    &inAudioIndex,
                    &outAudioIndex);
        }
        av_packet_unref(inPacket);
        av_packet_unref(outPacket);
    }

    if (mIsCancel != 0) {
        flushEncoder(outFormatContext, outVideoCodecContext, outPacket,
                     outVideoIndex, inFormatContext, (char *) inFilename);
        flushEncoder(outFormatContext, outAudioCodecContext, outPacket,
                     outAudioIndex, NULL, NULL);
        ret = av_write_trailer(outFormatContext);
        if (ret != 0) {
            LOGE("write trailer error");
            goto end;
        }
    } else {
        ret = -100;
        goto end;
    }

    ret = 0;

    end:
    if (inFrame != NULL) {
        av_frame_free(&inFrame);
    }
    if (outVideoFrame != NULL) {
        av_frame_free(&outVideoFrame);
    }
    if (outAudioFrame != NULL) {
        av_frame_free(&outAudioFrame);
    }
    if (inPacket != NULL) {
        av_packet_free(&inPacket);
    }
    if (outPacket != NULL) {
        av_packet_free(&outPacket);
    }
    sws_freeContext(swsContext);
    if (swrContext != NULL) {
        swr_free(&swrContext);
    }
    avcodec_close(inVideoCodecContext);
    avcodec_close(outVideoCodecContext);
    avcodec_close(inAudioCodecContext);
    avcodec_close(outAudioCodecContext);
    if (inFormatContext != NULL) {
        avformat_close_input(&inFormatContext);
        avformat_free_context(inFormatContext);
    }
    if (outFormatContext != NULL) {
        avformat_close_input(&outFormatContext);
        avformat_free_context(outFormatContext);
    }
    return ret;
}

void cancelCompress() {
    LOGE("before cancel: %d", mIsCancel);
    mIsCancel = 0;
    LOGE("after cancel: %d", mIsCancel);
}

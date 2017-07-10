//
// Created by Saqrag Borgn on 05/07/2017.
//
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include "Compress.h"
#include "libavformat/avformat.h"
#include "../Log.h"

int decodeVideo();

int decodeAudio(AVPacket *packet);

int initInput(const char *inFle);

int initOutput(const char *outFile);

int print_ABC(AVCodecContext *p);

int flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index);

int ret = -1;
AVFormatContext *mInFormatContext, *mOutFormatContext;
AVCodecContext *mInVideoCodecContext, *mOutVideoCodecContext;
AVPacket *mInPacket = NULL, *mOutPacket = NULL;
AVFrame *mInFrame, *mOutFrame;
int mVideoIndex = -1, mAudioIndex = -1;
AVStream *outStream = NULL;
int frameNum = -1;


int compress(const char *in_filename, const char *out_filename) {

    //1. 注册
    av_register_all();

    //2. 初始化input
    ret = initInput(in_filename);
    if (ret != 0) {
        LOGE("init input File error!!");
        goto end;
    }

    //3. 初始化output
    ret = initOutput(out_filename);
    if (ret != 0) {
        LOGE("init output file error");
        goto end;
    }

    //4. AVPacket 申请内存
    mInPacket = av_malloc(sizeof(AVPacket));
    mOutPacket = av_malloc(sizeof(AVPacket));

    //5. AVFrame 申请内存
    mInFrame = av_frame_alloc();
    mOutFrame = av_frame_alloc();
    int picture_size = avpicture_get_size(mOutVideoCodecContext->pix_fmt,
                                          mOutVideoCodecContext->width,
                                          mOutVideoCodecContext->height);
    uint8_t *picture_buf = (uint8_t *) av_malloc(picture_size);
    avpicture_fill((AVPicture *) mOutFrame, picture_buf, mOutVideoCodecContext->pix_fmt,
                   mOutVideoCodecContext->width, mOutVideoCodecContext->height);

    //6. 写头
    avformat_write_header(mOutFormatContext, NULL);

    while (av_read_frame(mInFormatContext, mInPacket) == 0) {
        if (mInPacket->stream_index == mVideoIndex) {
            ret = decodeVideo();
        } else if (mInPacket->stream_index == mAudioIndex) {

        }
    }

    flush_encoder(mOutFormatContext, (unsigned int) outStream->index);

    ret = av_write_trailer(mOutFormatContext);
    if (ret != 0) {
        LOGE("write trailer error");
        goto end;
    }

    av_frame_free(&mInFrame);
    avcodec_close(mInVideoCodecContext);
    avformat_close_input(&mInFormatContext);
    av_frame_free(&mOutFrame);
    avcodec_close(mOutVideoCodecContext);
    avformat_close_input(&mOutFormatContext);
    return 0;

    end:
    avformat_free_context(mInFormatContext);
    avformat_free_context(mInFormatContext);
    return -10;
}

int initInput(const char *inFile) {

    //1. 获取AVFormatContext
    mInFormatContext = avformat_alloc_context();
    ret = avformat_open_input(&mInFormatContext, inFile, NULL, NULL);
    if (ret != 0) {
        LOGE("open input file: %s error!!", inFile);
        return -1;
    }

    //2. 查找AVStream信息，并填充到AVFormatContext
    ret = avformat_find_stream_info(mInFormatContext, NULL);
    if (ret < 0) {
        LOGE("find input AVStream error");
        return -2;
    }

    //3. 获取 StreamIndex
    for (int i = 0; i < mInFormatContext->nb_streams; ++i) {
        enum AVMediaType type = mInFormatContext->streams[i]->codecpar->codec_type;
        switch (type) {
            case AVMEDIA_TYPE_VIDEO:
                mVideoIndex = i;
                mInVideoCodecContext = mInFormatContext->streams[mVideoIndex]->codec;
                AVCodec *pCodec = avcodec_find_decoder(
                        mInFormatContext->streams[mVideoIndex]->codecpar->codec_id);
                avcodec_open2(mInVideoCodecContext, pCodec, NULL);
                LOGD("in pix fmt: %d", mInVideoCodecContext->pix_fmt);
                LOGD("AV_PIX_FMT_YUV420P: %d", AV_PIX_FMT_YUV420P);
                break;
            case AVMEDIA_TYPE_AUDIO:
                mAudioIndex = i;
                break;
            default:
                break;
        }
    }

    if (mVideoIndex == -1) {
        LOGD("not found video Stream");
        return -3;
    }
    if (mAudioIndex == -1) {
        LOGD("not found audio Stream");
        return -4;
    }
    return 0;
}

int initOutput(const char *outFile) {
    //1. 创建AVFormat
    ret = avformat_alloc_output_context2(&mOutFormatContext, NULL, NULL, outFile);
    if (ret < 0) {
        LOGE("alloc output format context error!");
        return -100;
    }

    //2. 创建AVIOContext
    ret = avio_open(&mOutFormatContext->pb, outFile, AVIO_FLAG_READ_WRITE);
    if (ret < 0) {
        LOGE("open output AVIOContext error");
        return -101;
    }

    //3. 创建video AVStream
    outStream = avformat_new_stream(mOutFormatContext, NULL);
    if (outStream == NULL) {
        LOGE("create out Stream error");
        return -102;
    }
    outStream->pts.num = mInVideoCodecContext->time_base.num;
    outStream->pts.den = mInVideoCodecContext->time_base.den;

    //4. 配置AVCodecContext
    mOutVideoCodecContext = outStream->codec;
//    avcodec_copy_context(mOutVideoCodecContext, mInVideoCodecContext);
    mOutVideoCodecContext->bit_rate = 60;
    mOutVideoCodecContext->gop_size = 250;
    mOutVideoCodecContext->thread_count = 16;
    mOutVideoCodecContext->time_base.num = 1;
    mOutVideoCodecContext->time_base.den = 18;
    mOutVideoCodecContext->qmin = 10;
    mOutVideoCodecContext->qmax = 51;
    mOutVideoCodecContext->max_b_frames = 3;


    //pCodecCtx->codec_id =AV_CODEC_ID_HEVC;
    mOutVideoCodecContext->codec_id = AV_CODEC_ID_H264;
    mOutVideoCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    mOutVideoCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    mOutVideoCodecContext->width = mInVideoCodecContext->width;
    mOutVideoCodecContext->height = mInVideoCodecContext->height;

    LOGD("m num: %d", mInVideoCodecContext->time_base.num);
    LOGD("m den: %d", mInVideoCodecContext->time_base.den);
    LOGD("in pix_format: %d", mInVideoCodecContext->pix_fmt);
    LOGD("out pix_format: %d", mOutVideoCodecContext->pix_fmt);
    LOGD("AV_PIX_FMT_YUV420P: %d", AV_PIX_FMT_YUV420P);

    mOutVideoCodecContext->bit_rate = 400000;
    mOutVideoCodecContext->gop_size = 250;
    mOutVideoCodecContext->thread_count = 16;

    mOutVideoCodecContext->max_b_frames = 3;

    mOutVideoCodecContext->time_base.num = mInVideoCodecContext->time_base.num;
    mOutVideoCodecContext->time_base.den = mInVideoCodecContext->time_base.den;

//    pCodecCtx->me_pre_cmp = 1;
    //H264
    //pCodecCtx->me_range = 16;
    //pCodecCtx->max_qdiff = 4;
    //pCodecCtx->qcompress = 0.6;
    mOutVideoCodecContext->qmin = 10;
    mOutVideoCodecContext->qmax = 51;

    //Optional Param
    mOutVideoCodecContext->max_b_frames = 3;
    LOGD("AV_CODEC_ID_H264:%d", AV_CODEC_ID_H264);
    LOGD("out pix format: %d", mOutVideoCodecContext->codec_id);
    LOGD("int codec id :%d", mInVideoCodecContext->codec_id);

    // Set Option
    AVDictionary *param = 0;
    //H.264
    if (mOutVideoCodecContext->codec_id == AV_CODEC_ID_H264) {
        av_opt_set(mOutVideoCodecContext->priv_data, "preset", "superfast", 0);
//        av_dict_set(&param, "tune", "animation", 0);
        av_dict_set(&param, "profile", "baseline", 0);
    }

    //Show some Information
    av_dump_format(mOutFormatContext, 0, outFile, 1);

    //5. 配置AVCodec
    AVCodec *outCodec = NULL;
    outCodec = avcodec_find_encoder(mOutVideoCodecContext->codec_id);
    if (outCodec == NULL) {
        LOGE("find AVCodec error!");
        return -103;
    }
    ret = avcodec_open2(mOutVideoCodecContext, outCodec, &param);
    if (ret != 0) {
        LOGE("open codec error");
        return -104;
    }
    return 0;
}

int decodeVideo() {
    LOGD("解码1");
    //1. 解码
    ret = avcodec_send_packet(mInVideoCodecContext, mInPacket);
    if (ret != 0) {
        LOGE("send decode packet error");
        return -200;
    }
    LOGD("解码2");
    ret = avcodec_receive_frame(mInVideoCodecContext, mInFrame);
    if (ret != 0) {
        LOGE("receive decode frame error");
        return -201;
    }

    LOGD("inFrame pts: %lld", mInFrame->pts);
    LOGD("inFrame Data: %s", mInFrame->data[1]);

    mOutFrame->data[0] = mInFrame->data[0];
    mOutFrame->data[1] = mInFrame->data[1];
    mOutFrame->data[2] = mInFrame->data[2];
    frameNum++;
    mOutFrame->pts = frameNum;

    ret = avcodec_send_frame(mOutVideoCodecContext, mOutFrame);
    if (ret != 0) {
        LOGE("send encode frame error, CODE: %d", ret);
        return -203;
    }

    LOGD("encode  .............");

    ret = avcodec_receive_packet(mOutVideoCodecContext, mOutPacket);
    if (ret != 0) {
        LOGE("receive encode packet error, CODE: %d", ret);
        return -203;
    }

    LOGD("encode: %s", mOutPacket->data);

    mOutPacket->stream_index = outStream->index;
    LOGD("outStream index: %d", mOutPacket->stream_index);

    LOGD("编码3");
    //3. 写入 AVFormatContext
    ret = av_write_frame(mOutFormatContext, mOutPacket);
    if (ret < 0) {
        LOGE("write frame error, CODE: %d", ret);
        return -204;
    }
    av_free_packet(mInPacket);
    av_free_packet(mOutPacket);
    LOGD("编码4");
    return 0;
}

int decodeAudio(AVPacket *packet) {

}

int print_ABC(AVCodecContext *p) {
    int i;
    char *p_char = (char *) p;

    for (i = 0; i < sizeof(AVCodecContext); i++, p_char++) {
        LOGD("%02x  ", *p_char);
    }
    return 0;
}

int flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index) {
    int ret;
    int got_frame;
    AVPacket enc_pkt;
    if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities &
          CODEC_CAP_DELAY))
        return 0;
    while (1) {
        enc_pkt.data = NULL;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);
        ret = avcodec_encode_video2(fmt_ctx->streams[stream_index]->codec, &enc_pkt,
                                    NULL, &got_frame);
        av_frame_free(NULL);
        if (ret < 0)
            break;
        if (!got_frame) {
            ret = 0;
            break;
        }
        printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);
        /* mux encoded frame */
        ret = av_write_frame(fmt_ctx, &enc_pkt);
        if (ret < 0)
            break;
    }
    return ret;
}

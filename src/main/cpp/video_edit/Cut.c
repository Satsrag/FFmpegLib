//
// Created by Saqrag Borgn on 05/07/2017.
//

#include "Cut.h"
#include "../Log.h"
#include "libavformat/avformat.h"

/**
 * 根据起始时间裁剪一段视频
 * @param starttime  开始时间
 * @param endtime   结束时间
 * @param in_filename   原视频的路径(包含文件名)
 * @param out_filename  裁剪结果的存放路径(包含文件名)
 * @return  如果成功则返回0，否则返回其他
 */
int cut_video(double starttime, double endtime, const char *in_filename, const char *out_filename) {
    LOGI("开始执行裁剪");
    AVOutputFormat *outputFormat = NULL;
    AVFormatContext *inFormatContext = NULL, *outFormatContext = NULL;
    AVPacket packet;
    int ret;
    int stream_index = 0;
    int *stream_mapping = NULL;
    int stream_mapping_size = 0;

    //1、注册所有组件
    LOGI("1、注册所有组件");
    av_register_all();

    //2、开始读取输入视频文件
    LOGI("2、开始读取输入视频文件");
    if ((ret = avformat_open_input(&inFormatContext, in_filename, 0, 0)) < 0) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Could not open input file %s", in_filename);
        return ret;
    }

    //3、获取视频流媒体信息
    LOGI("3、获取视频流媒体信息");
    if ((ret = avformat_find_stream_info(inFormatContext, 0)) < 0) {
        LOGE("\tFailed to retrieve input stream information");
        return ret;
    }

    __android_log_print(ANDROID_LOG_INFO, TAG,
                        "bit_rate:%lld\n"
                                "duration:%lld\n"
                                "nb_streams:%d\n"
                                "fps_prob_size:%d\n"
                                "format_probesize:%d\n"
                                "max_picture_buffer:%d\n"
                                "max_chunk_size:%d\n"
                                "format_name:%s",
                        inFormatContext->bit_rate,
                        inFormatContext->duration,
                        inFormatContext->nb_streams,
                        inFormatContext->fps_probe_size,
                        inFormatContext->format_probesize,
                        inFormatContext->max_picture_buffer,
                        inFormatContext->max_chunk_size,
                        inFormatContext->iformat->name);

    //4、创建输出的AVFormatContext对象
    LOGI("4、创建输出的AVFormatContext对象");
    avformat_alloc_output_context2(&outFormatContext, NULL, NULL, out_filename);
    if (!outFormatContext) {
        LOGE("\tCould not create output context\n");
        ret = AVERROR_UNKNOWN;
        return ret;
    }

    //设置stream_mapping
    LOGI("5、设置stream_mapping");
    stream_mapping_size = inFormatContext->nb_streams;
    stream_mapping = (int *) av_mallocz_array((size_t) stream_mapping_size,
                                              sizeof(*stream_mapping));
    if (!stream_mapping) {
        ret = AVERROR(ENOMEM);
        LOGE("Error while set stream_mapping");
        return ret;
    }

    outputFormat = outFormatContext->oformat;
    //6、根据输入流设置相应的输出流参数
    LOGI("6、根据输入流设置相应的输出流参数");
    for (int i = 0; i < inFormatContext->nb_streams; i++) {
        AVStream *in_stream = inFormatContext->streams[i];
        AVStream *out_stream = avformat_new_stream(outFormatContext, NULL);
        AVCodecParameters *in_codecpar = in_stream->codecpar;
        if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
            stream_mapping[i] = -1;
            continue;
        }
        stream_mapping[i] = stream_index++;

        if (!out_stream) {
            LOGE("\tFailed to create output stream");
            ret = AVERROR_UNKNOWN;
            return ret;
        }

        if ((ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar)) < 0) {
            LOGE("\tFailed to copy codec parameters");
            return ret;
        }

        out_stream->codecpar->codec_tag = 0;

    }


    //7、检查输出文件是否正确配置完成
    LOGI("7、检查输出文件是否正确配置完成");
    if (!(outputFormat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&outFormatContext->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            LOGE("\tCould not open output file %s", out_filename);
            return ret;
        }
    }

    //8、写入Stream头部信息
    LOGI("8、写入Stream头部信息");
    ret = avformat_write_header(outFormatContext, NULL);
    if (ret < 0) {
        LOGE("\tError occurred while write header");
        return ret;
    }



    //9、设置dts和pts变量的内存
    LOGI("9、设置dts和pts变量的内存");
    int64_t *dts_start_from = (int64_t *) malloc(sizeof(int64_t) * inFormatContext->nb_streams);
    memset(dts_start_from, 0, sizeof(int64_t) * inFormatContext->nb_streams);
    int64_t *pts_start_from = (int64_t *) malloc(sizeof(int64_t) * inFormatContext->nb_streams);
    memset(pts_start_from, 0, sizeof(int64_t) * inFormatContext->nb_streams);

    //10、定位当前位置到裁剪的起始位置 from_seconds
    LOGI("10、定位当前位置到裁剪的起始位置:%lld, stream: %d, ", (int64_t) (starttime * AV_TIME_BASE),
         stream_index);

    ret = av_seek_frame(inFormatContext, -1, (int64_t) (starttime * AV_TIME_BASE),
                        AVSEEK_FLAG_BACKWARD);
    if (ret < 0) {
        LOGE("\tError seek to the start");
        return ret;
    }

    //11、开始写入视频信息
    LOGI("11、开始写入视频信息");
    int k = 0;
    while (1) {
        k++;
        LOGD("<1> -----------------------------< Loop：%d >-------------------------------", k);
        AVStream *in_stream, *out_stream;

        LOGD("<2> Read frame");
        ret = av_read_frame(inFormatContext, &packet);
        if (ret < 0) {
            break;
        }


        LOGI("\tPacket stream_index：%d", packet.stream_index);
        in_stream = inFormatContext->streams[packet.stream_index];
        if (packet.stream_index >= stream_mapping_size ||
            stream_mapping[packet.stream_index] < 0) {
            LOGE("reach end");
            av_packet_unref(&packet);
            continue;
        }

        //convert coding
//        avcodec_send_frame(in_stream->codec, frame);
//        AVCodecContext *outCodecContext = avcodec_alloc_context3(in_stream->codec->codec);
//        avcodec_parameters_to_context(outCodecContext, in_stream->codecpar);
//        outCodecContext->bit_rate = 40000;
//        avcodec_receive_packet(outCodecContext, &mInPacket);

        packet.stream_index = stream_mapping[packet.stream_index];

        out_stream = outFormatContext->streams[packet.stream_index];

        av_dict_copy(&out_stream->metadata, in_stream->metadata, AV_DICT_IGNORE_SUFFIX);

        LOGI("\tin_steam bit_rate:%lld", in_stream->codecpar->bit_rate);
        LOGI("\tin_steam bits_codec_sample:%d", in_stream->codecpar->bits_per_coded_sample);
        LOGI("\tin_steam bits_per_raw_sample:%d", in_stream->codecpar->bits_per_raw_sample);


//        log_packet(inFormatContext, &mInPacket, "in");

        //av_q2d转换AVRational(包含分子分母的结构)类型为double,此过程有损
        LOGI("\tin_stream time_base: %d/%d", in_stream->time_base.num, in_stream->time_base.den);
        if (av_q2d(in_stream->time_base) * packet.pts > endtime) {
            LOGI("Reach End");
            av_packet_unref(&packet);
            break;
        }

        if (dts_start_from[packet.stream_index] == 0) {
            dts_start_from[packet.stream_index] = packet.dts;
            LOGI("\tdts_start_from: %lld", dts_start_from[packet.stream_index]);
        }

        if (pts_start_from[packet.stream_index] == 0) {
            pts_start_from[packet.stream_index] = packet.pts;
            LOGI("\tpts_start_from: %lld", pts_start_from[packet.stream_index]);
        }

        //判断dts和pts的关系，如果 dts < pts 那么当调用 av_write_frame() 时会导致 Invalid Argument
        if (dts_start_from[packet.stream_index] < pts_start_from[packet.stream_index]) {
            LOGW("pts is smaller than dts, resting pts to equal dts");
            pts_start_from[packet.stream_index] = dts_start_from[packet.stream_index];
        }

        LOGD("<3> Packet timestamp");
        packet.pts = av_rescale_q_rnd(packet.pts - pts_start_from[packet.stream_index],
                                      in_stream->time_base, out_stream->time_base,
                                      AV_ROUND_INF);
        packet.dts = av_rescale_q_rnd(packet.dts - dts_start_from[packet.stream_index],
                                      in_stream->time_base, out_stream->time_base,
                                      AV_ROUND_ZERO);
        if (packet.pts < 0) {
            packet.pts = 0;
        }
        if (packet.dts < 0) {
            packet.dts = 0;
        }
        LOGI("PTS:%lld\tDTS:%lld", packet.dts, packet.dts);
        packet.duration = av_rescale_q((int64_t) packet.duration, in_stream->time_base,
                                       out_stream->time_base);
        packet.pos = -1;

        //将当前Packet写入输出文件
        LOGD("<4> Write Packet");
        LOGI("\tAVFormatContext State:%d", outFormatContext != NULL);
        LOGI("\tPacket State:%d", &packet != NULL);
        if ((ret = av_interleaved_write_frame(outFormatContext, &packet)) < 0) {
            LOGE("\tError write mInPacket\n");
            return ret;
        }
        //重置Packet对象
        LOGD("<5> Unref Packet");
        av_packet_unref(&packet);
    }
    free(dts_start_from);
    free(pts_start_from);

    //12、写入stream尾部信息
    LOGD("12、写入stream尾部信息");
    av_write_trailer(outFormatContext);

    //13、收尾：回收内存，关闭流...
    LOGD("13、收尾：回收内存，关闭流...");
    avformat_close_input(&inFormatContext);

    if (outFormatContext && !(outputFormat->flags & AVFMT_NOFILE)) {
        avio_closep(&outFormatContext->pb);
    }
    avformat_free_context(outFormatContext);

    return 0;
}


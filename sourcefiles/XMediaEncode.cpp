//
// Created by lcx on 12/1/22.
//
#include <iostream>
#include "XMediaEncode.h"

extern "C" {
#include"libavcodec/avcodec.h"
#include"libavutil/avutil.h"
#include"libswscale/swscale.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
}

using namespace std;


#if defined WIN32 || defined _WIN32
#include <windows.h>
#endif

#if defined __linux__

#include "unistd.h"

#endif

//获取CPU数量
static int XGetCpuNum() {
#if defined WIN32 || defined _WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);

    return (int)sysinfo.dwNumberOfProcessors;
#elif defined __linux__
    return (int) sysconf(_SC_NPROCESSORS_ONLN);
#elif defined __APPLE__
    int numCPU = 0;
    int mib[4];
    size_t len = sizeof(numCPU);
    // set the mib for hw.ncpu
    mib[0] = CTL_HW;
    mib[1] = HW_AVAILCPU;  // alternatively, try HW_NCPU;
                           // get the number of CPUs from the system
    sysctl(mib, 2, &numCPU, &len, NULL, 0);
    if (numCPU < 1)
    {
        mib[1] = HW_NCPU;
        sysctl(mib, 2, &numCPU, &len, NULL, 0);

        if (numCPU < 1)
            numCPU = 1;
    }
    return (int)numCPU;
#else
    return 1;
#endif
}  //小技巧 拿来主义  opencv 源码



class CXMediaEncode : public XMediaEncode {
public:
    void close() {
        if (srcpix) {
            sws_freeContext(srcpix);
            srcpix = nullptr;  //防止多次施放内存
        }
        if (yuv) {
            av_frame_free(&yuv);
        }
        if (asc) {
            swr_free(&asc);  //自动置空
        }
        if (pcm) {
            av_frame_free(&yuv);
        }

        if (cctx) {
            avcodec_free_context(&cctx);
        }
        fpts = 0;

        av_packet_unref(&vpkt);
        av_packet_unref(&apkt);
        apts = 0;

    };


    bool InitScale() override {

        srcpix = sws_getCachedContext(srcpix, SRCWIDTH, SRCHEIGHT, AV_PIX_FMT_RGB24,
                                      SRCWIDTH, SRCHEIGHT, AV_PIX_FMT_YUV420P,
                                      SWS_BICUBIC, nullptr, 0, 0);
        if (!srcpix) {
            cout << "创建swscontext时出错！" << endl;
            return false;
        }

        yuv = av_frame_alloc();
        yuv->format = AV_PIX_FMT_YUV420P;
        yuv->width = SRCWIDTH;
        yuv->height = SRCHEIGHT;
        yuv->pts = 0;
        int ret = av_frame_get_buffer(yuv, 32);  //按照32位对齐
        if (ret != 0) {
            char buffer[1024] = {0};
            av_strerror(ret, buffer, sizeof(buffer) - 1);
            cout << buffer << endl;
            return false;
        }
        return true;
    }

    bool InitResample() {
//    AAc 编码必须是float类型的所以需要进行重采样

// 1、 初始化重采样上下文
        asc = swr_alloc_set_opts(asc,
                                 av_get_default_channel_layout(channels),
                                 (AVSampleFormat) outSamplefmt,
                                 sampleRate, av_get_default_channel_layout(channels),
                                 (AVSampleFormat) inSamplefmt,
                                 sampleRate, 0, 0);
        if (!asc) {
            cout << "swresample上下文分配出错！" << endl;
            return false;
        }
        int ret = swr_init(asc);
        if (ret != 0) {
            char err[1024] = {0};
            av_strerror(ret, err, sizeof(err) - 1);
            cout << err << endl;
            return -1;
        }
        cout << "初始化重采样上下文成功！" << endl;
        //音频重采样空间分配
        pcm = av_frame_alloc();
        pcm->format = outSamplefmt;
        pcm->channels = channels;
        pcm->channel_layout = av_get_default_channel_layout(channels);
        pcm->nb_samples = nbsample;
        ret = av_frame_get_buffer(pcm, 0);//给pcm分配空间大小自动计算
        if (ret != 0) {
            char err[1024] = {0};
            av_strerror(ret, err, sizeof(err) - 1);
            cout << err << endl;
            return false;
        }
        return true;
    }


    XData Resample(XData d) {
        XData r;
        const uint8_t *indata[AV_NUM_DATA_POINTERS] = {(uint8_t *) d.data};

        int len = swr_convert(asc, this->pcm->data,
                              this->pcm->nb_samples, indata,
                              this->pcm->nb_samples);
        if (len < 0) {
            return r;
        }
        pcm->pts = d.pts;
        r.data = (char *)pcm;
        r.size = pcm->nb_samples*pcm->channels*2;
        r.pts = d.pts;
        return r;

    }


    XData rgb_to_yuv(XData d) override {
        XData r;
        r.pts = d.pts;
        uint8_t *indata[AV_NUM_DATA_POINTERS] = {0};  //AV_NUM_DATA_POINTERS = 8
        indata[0] = (uint8_t *) d.data;
        int insize[AV_NUM_DATA_POINTERS] = {0};  //AV_NUM_DATA_POINTERS = 8
        insize[0] = SRCWIDTH * inPixSize;
        int h = sws_scale(srcpix, indata, insize,
                          0, SRCHEIGHT, yuv->data, yuv->linesize);
        if (h <= 0) {
            return r;  //跳过异常帧率
        }
        yuv->pts = d.pts;
        r.data = (char *)yuv;
        int *p = yuv->linesize;  //8位数组
        while((*p)){
            r.size += (*p)*OUTHeight;
            p++;
        }

        return r;
    }

    bool InitVideoCodec() override {
        //找到编码器 同时创建编码器上下文
        cctx = CreateCodec(AV_CODEC_ID_H264);
        if (!cctx) {
            cout << "CreateCodec产生错误！" << endl;
            return false;
        }
        //参数配置
        cctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        cctx->codec_id = cctx->codec->id;
        cctx->thread_count = XGetCpuNum();

        cctx->bit_rate = 50 * 1024 * 8; // 压缩以后每秒的视频大小（bit）
        cctx->width = SRCWIDTH;
        cctx->height = SRCHEIGHT;
//        cctx->time_base = {1, fps};
        cctx->framerate = {fps, 1};

        cctx->gop_size = 40; //多少帧一个关键帧
        cctx->max_b_frames = 0; //不要关键帧 由于 B帧的解码顺序特殊 故为了方便设为0
        cctx->pix_fmt = AV_PIX_FMT_YUV420P;
        cctx->time_base = {1,1000000};
        //打开编码器
        return OpenCodec(&cctx);
    }


    bool InitAudioCodec() override {
//    初始化音频编码器
        ac = CreateCodec(AV_CODEC_ID_AAC);

        //参数配置
        ac->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        ac->codec_id = ac->codec->id;
        ac->thread_count = XGetCpuNum();
        ac->bit_rate = 40000;
        ac->sample_rate = sampleRate;
        ac->sample_fmt = AV_SAMPLE_FMT_FLTP;
        ac->channels = channels;
        ac->time_base = {1,1000000};
        ac->channel_layout = av_get_default_channel_layout(channels);

        return OpenCodec(&ac);

    }


    XData EncodecVideo(XData frame) override {
        av_packet_unref(&vpkt);  //外部不需要清理
        XData r;
        if(frame.size<=0 || !frame.data) return r;
        AVFrame *p = (AVFrame *)frame.data;

        //正式开始编码
        int ret = avcodec_send_frame(cctx, p);

        if (ret != 0) {
            return r;//次出现错误跳过
        }
        ret = avcodec_receive_packet(cctx, &vpkt);
        if (ret != 0 || vpkt.size<=0) {
            return r;//次出现错误跳过
        }
        r.data = (char *)&vpkt;
        r.size = vpkt.size;
        r.pts = frame.pts;
        cout << vpkt.size << endl;

        return r;

    }

    long long lasta = -1;
    XData EncodecAudio(XData frame) override {
        XData r;
        if(frame.size<=0 || !frame.data) return r;
        AVFrame *p = (AVFrame *)frame.data;
        if(lasta == p->pts){
            p->pts += 1200; //防止pts相同 由于系统时间的误差
        }
        lasta = p->pts;
        int ret = avcodec_send_frame(ac, p);
        if (ret != 0) {
            return r;//次出现错误跳过
        }

        av_packet_unref(&apkt);
        ret = avcodec_receive_packet(ac, &apkt);
        if (ret != 0) {
            return r;//次出现错误跳过
        }
        r.data = (char *)&apkt;
        r.size = apkt.size;
        r.pts = frame.pts;
        cout << apkt.size << endl;
        return r;
    }

private:
    AVCodecContext *CreateCodec(AVCodecID id) {
        //找到编码器
        AVCodec *codec = avcodec_find_encoder(id);
        if (!codec) {
            cout << "找不到对应的编码器" << endl;
            return nullptr;
        }
        cout << "找到对应的编码器" << endl;
        //创建编码器上下文
        AVCodecContext *c = avcodec_alloc_context3(codec); //如果这里指定下面打开编码器可以不指定
        if (!c) {
            cout << "创建上下文出错！" << endl;
            return nullptr;
        }
        cout << "创建上下文成功！" << endl;

        return c;
    }

    bool OpenCodec(AVCodecContext **c) {
        int ret = avcodec_open2(*c, nullptr, nullptr);
        if (ret != 0) {
            char err[1024] = {0};
            av_strerror(ret, err, sizeof(err) - 1);
            cout << err << endl;
            avcodec_free_context(c);
            return false;
        }
        cout << "编码器打开成功！" << endl;
        return true;
    }

    SwsContext *srcpix = nullptr;  //像素格式初始化上下文C++11特性
    AVFrame *yuv = nullptr;  //输出的yuv

    AVPacket vpkt = {nullptr};  //视频包
    AVPacket apkt = {nullptr}; //音频包
    int fpts = 0;
    int apts = 0;
    SwrContext *asc = nullptr;
    AVFrame *pcm = nullptr;


};


XMediaEncode *XMediaEncode::Get(unsigned char index) {
    static CXMediaEncode cxMediaEncode[255];
    return &cxMediaEncode[index];
}


XMediaEncode::XMediaEncode() {

}

XMediaEncode::~XMediaEncode() {

}


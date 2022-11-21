#include "XData.h"

//
// Created by lcx on 12/1/22.
//
 //值的学习
struct AVCodecContext;

enum XAVSampleFormat {
    XAV_SAMPLE_FMT_NONE = -1,
    XAV_SAMPLE_FMT_U8,          ///< unsigned 8 bits
    XAV_SAMPLE_FMT_S16,         ///< signed 16 bits
    XAV_SAMPLE_FMT_S32,         ///< signed 32 bits
    XAV_SAMPLE_FMT_FLT,         ///< float
    XAV_SAMPLE_FMT_DBL,         ///< double

    XAV_SAMPLE_FMT_U8P,         ///< unsigned 8 bits, planar
    XAV_SAMPLE_FMT_S16P,        ///< signed 16 bits, planar
    XAV_SAMPLE_FMT_S32P,        ///< signed 32 bits, planar
    XAV_SAMPLE_FMT_FLTP,        ///< float, planar
    XAV_SAMPLE_FMT_DBLP,        ///< double, planar
    XAV_SAMPLE_FMT_S64,         ///< signed 64 bits
    XAV_SAMPLE_FMT_S64P,        ///< signed 64 bits, planar

    XAV_SAMPLE_FMT_NB           ///< Number of sample formats. DO NOT USE if linking dynamically
};

#ifndef RTMP_XMEDIAENCODE_H
#define RTMP_XMEDIAENCODE_H
class XMediaEncode{
public:
    AVCodecContext *cctx = nullptr;
    AVCodecContext *ac = nullptr;
    //输入参数
    int SRCWIDTH = 640;
    int SRCHEIGHT = 480;
    int inPixSize = 3;
    //输出参数
    int OUTWIDTH = 640;
    int OUTHeight = 480;
    int bitrate = 4000000;  //压缩以后每秒的视频大小（bit）
    int fps = 25;
//音频输入
    int sampleRate = 44100;
    int channels = 2;
    int sampleByte = 2;
    XAVSampleFormat inSamplefmt = XAV_SAMPLE_FMT_S16;
    //输出
    XAVSampleFormat outSamplefmt = XAV_SAMPLE_FMT_FLTP;
    int nbsample = 1024;

    //工厂生产方法
    static XMediaEncode* Get(unsigned char index = 0);

    //
    virtual void close() = 0;
    //初始化下像素格式转换的上下文
    virtual bool InitScale() = 0;

    //初始化重采样上下文
    virtual bool InitResample() = 0;

    //重采样
    virtual XData Resample(XData data) = 0;

    //RGB to YUV
    virtual XData rgb_to_yuv(XData data) = 0;

    //编码器初始化
    virtual bool InitVideoCodec() = 0;

    //音频编码器初始化
    virtual bool InitAudioCodec() = 0;

    //视频编码 //返回值无需调用者清理
    virtual XData EncodecVideo(XData frame) = 0;

    //音频编码 //返回值无需调用者清理
    virtual XData EncodecAudio(XData frame) = 0;

    virtual ~XMediaEncode();
protected:
    XMediaEncode();
};



#endif //RTMP_XMEDIAENCODE_H

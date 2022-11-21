//
// Created by lcx on 12/1/22.
//

#ifndef RTMP_XRTMP_H
#define RTMP_XRTMP_H
#include "XData.h"
struct AVCodecContext;
struct AVPacket;
class Xrtmp{
public:

    virtual  void close() = 0;
    //工厂生产方法
    static Xrtmp* Get(unsigned char index = 0);

    //初始化封装器上下文
    virtual ~Xrtmp();

    virtual bool Init(const char* url) = 0;
    //添加视频或者音频流
    virtual int addStream(const AVCodecContext *avCodecContext) = 0;

    //打开rtmp网络IO 发送封装头
    virtual bool sendHeader() = 0;

    //rtmp推流
    virtual bool SendFrame(XData pkt,int streamindex= 0 ) = 0;
protected:
    Xrtmp();
};
#endif //RTMP_XRTMP_H

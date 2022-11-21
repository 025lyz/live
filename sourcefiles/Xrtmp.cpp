//
// Created by lcx on 12/1/22.
//
#include "Xrtmp.h"
#include<iostream>
#include<string>
extern "C"{
#include "libavformat/avformat.h"

}
using namespace std;
class CXrtmp:public Xrtmp{
public:

    void close(){
        if(ifctx){
            avformat_close_input(&ifctx);
            videos = nullptr;
            audios = nullptr;
        }
        videocodectx = nullptr;
        audiocodectx = nullptr;

        url = "";

    }

    bool Init(const char* url){
        int ret = avformat_alloc_output_context2(&ifctx, nullptr,"flv", url);
        this->url = url;
        if (ret != 0) {
            char buffer[1024] = {0};
            av_strerror(ret, buffer, sizeof(buffer) - 1);
            cout<<buffer<<endl;
            return false;
        }
        return true;
    }

    int addStream(const AVCodecContext *avCodecContext) override{
        if(!avCodecContext)return false;

        AVStream *stream = avformat_new_stream(ifctx, nullptr);
        if(!stream){
            cout<<"创建流错误"<<endl;
            return -1;
        }
        stream->codecpar->codec_tag = 0;
        //从编码器上下文赋值参数.
        avcodec_parameters_from_context(stream->codecpar,avCodecContext);
        av_dump_format(ifctx,0,url.c_str(),1);
        if(avCodecContext->codec_type == AVMEDIA_TYPE_VIDEO){
            videocodectx = avCodecContext;
            videos = stream;
        }
        if(avCodecContext->codec_type == AVMEDIA_TYPE_AUDIO){
            audiocodectx = avCodecContext;
            audios = stream;
        }
        return stream->index;
    }

    bool sendHeader() override{
        //打开输出io
        int ret = avio_open(&ifctx->pb,url.c_str(),AVIO_FLAG_WRITE);
        //pb 关闭记得置为nullptr
        if(ret!=0){
            char buffer[1024] = {0};
            av_strerror(ret, buffer, sizeof(buffer) - 1);
            cout<<buffer<<endl;
            return false;
        }
        //写入封住头 注意会更改time_base
        ret = avformat_write_header(ifctx,nullptr);
        if(ret!=0){
            char buffer[1024] = {0};
            av_strerror(ret, buffer, sizeof(buffer) - 1);
            cout<<buffer<<endl;
            return false;
        }
        return true;

    }
    bool SendFrame(XData d,int streamindex){

        if(!d.data || d.size<=0) return false;
        AVPacket *pkt = (AVPacket *)d.data;
        pkt->stream_index = streamindex;

        AVRational stime;
        AVRational dtime;
        //判断音视频
        if(videos && videocodectx && pkt->stream_index==videos->index){
            stime = videocodectx->time_base;
            dtime = videos->time_base;
        }
        else if(audios && audiocodectx && pkt->stream_index==audios->index){
            stime = audiocodectx->time_base;
            dtime = audios->time_base;
        }
        else{
            return false;
        }
        pkt->pts = av_rescale_q(pkt->pts,stime,dtime);
        pkt->dts = av_rescale_q(pkt->dts,stime,dtime);
        pkt->duration = av_rescale_q(pkt->duration,stime,dtime);

        //开始推流
        int ret = av_interleaved_write_frame(ifctx,pkt);
        if(ret==0){
            return true;
        }
        return false;
    }


private:
    AVFormatContext *ifctx = nullptr;
    string url;
    //视频编码器流
    const AVCodecContext *videocodectx = nullptr;

    const AVCodecContext *audiocodectx = nullptr;

    AVStream *videos = nullptr;

    AVStream *audios = nullptr;


};
Xrtmp* Xrtmp::Get(unsigned char index){
    static CXrtmp cXrtmp[255];
    return &cXrtmp[index];
}
Xrtmp::Xrtmp() {}
Xrtmp::~Xrtmp() noexcept {}

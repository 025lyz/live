//
// Created by lcx on 19/1/22.
//

#include "XController.h"
#include "XVideoCaptur.h"
#include "XAudioRecord.h"
#include "XMediaEncode.h"
#include "Xrtmp.h"

#include <iostream>

using namespace std;

bool XController::Start() {
    //设置磨皮参数
    XVideoCaptur *xv = XVideoCaptur::Get(0);
    XVideoCaptur::Get()->AddFilter(XFilter::Get());
    if (camIndex >= 0) {
        if(!xv->Init(0)) {
            cout<<"相机打开失败！"<<endl;
            return false;
        }

    }
    else if (!inUrl.empty()) {
        if (!xv->Init(inUrl.c_str())) {
            err = "打开";
            err += inUrl;
            err += "相机失败！";
            cerr << err << endl;
            return false;
        }
    } else {
        err = "请设置相机参数";
        cerr << "请设置相机参数" << endl;
        return false;
    }
    cout << "相机打开成功！" << endl;
    //
    XAudioRecord *ar= XAudioRecord::Get();
    ar->sampleRate = 44100;
    ar->channels = 2;
    ar->sampleByte = 2;
    ar->nbSamples = 1024;
    if (!ar->Init()) {
        err = "录音设备打开失败！";
        cerr << err << endl;
        return false;
    }
    cout<<"录音设备打开成功！"<<endl;
    xv->Start();
    XMediaEncode *xe = XMediaEncode::Get();
    xe->SRCWIDTH = xv->width;
    xe->SRCHEIGHT = xv->height;
    xe->OUTHeight = xv->height;
    xe->OUTWIDTH = xv->width;
    if (!xe->InitScale()) {
        err = "视频像素格式转换打开失败！";
        cerr << err << endl;
        return false;
    }
    cout << "视频像素格式转换打开成功！";

    XMediaEncode::Get()->channels = XAudioRecord::Get()->channels;
    XMediaEncode::Get()->nbsample = XAudioRecord::Get()->nbSamples;
    XMediaEncode::Get()->sampleRate = XAudioRecord::Get()->sampleRate;
    xe->outSamplefmt = XAVSampleFormat::XAV_SAMPLE_FMT_FLTP;
    xe->inSamplefmt = XAVSampleFormat::XAV_SAMPLE_FMT_S16;
    if (!xe->InitResample()) {
        err = "音频重采样上下文初始化失败！";
        cerr << err << endl;
        return false;
    }
    cout << "音频重采样上下文初始化成功！" << endl;


    ar->Start();

    //初始化音频编码器
    if (!xe->InitAudioCodec()) {
        err = "音频编码器初始化失败！";
        cerr << err << endl;
        return false;
    }
    cout << "音频编码器初始化成功！";

    //初始化视频编码器
    if (!xe->InitVideoCodec()) {
        err = "视频编码器初始化失败！";
        cerr << err << endl;
        return false;
    }
    cout << "视频编码器初始化成功！";

    Xrtmp *xr = Xrtmp::Get(0);
    if (!xr->Init(outUrl.c_str())) {
        err = "创建输出封装器上下文失败！";
        cerr << err << endl;
        return false;
    }
    cout << "创建输出封装器上下文成功！" << endl;

    //添加视频流
    //添加音频流
    //初始化视频流

    vindex = xr->addStream(xe->cctx);
    if(vindex<0) {
        err = "添加视频流失败!";
        cerr << err << endl;
        return false;
    }

    //初始化音频流
    aindex = xr->addStream(xe->ac);
    if(aindex<0){
        err = "添加音频流失败!";
        cerr << err << endl;
        return false;
    }

    cout<<"添加音视频流成功!"<<endl;

    if(!xr->sendHeader()){
        err = "写入头信息失败！";
        cerr << err << endl;
        return false;
    }

    cout<<"写入头信息成功！"<<endl;

    //清除已经录制的内容
    XAudioRecord::Get()->clear();
    XVideoCaptur::Get()->clear();
    XDataThread::Start();
    return true;
}



bool XController::Set(std::string key, double value) {
    XFilter::Get()->Set(key, value);
    return false;
}


void XController::Stop() {
    XDataThread::Stop();
    XVideoCaptur::Get()->Stop();
    XAudioRecord::Get()->Stop();
    XMediaEncode::Get()->close();
    Xrtmp::Get()->close();
    camIndex = -1;
    inUrl = "";
}

void XController::run() {
    long long begintime  = GetCurTime();  //放在线程之前保证pts是正数
    while(!isExit){
        //一次读取一帧音频
        XData adata = XAudioRecord::Get()->Pop();
        XData vdata = XVideoCaptur::Get()->Pop();
        if(adata.size<=0 && vdata.size<=0)
        {
            msleep(1);
            continue;
        }
        //音频处理
        if(adata.size>0){
            adata.pts = adata.pts - begintime;

            XData pcm = XMediaEncode::Get()->Resample(adata);
            adata.Drop();

            //        pts运算
            //  nb_sample/sample_rate  = 一帧音频的秒数sec
            // timebase  pts = sec * timebase.den
            //封装推流
            XData pkt = XMediaEncode::Get()->EncodecAudio(pcm);
            if(pkt.size>0){
                if(Xrtmp::Get()->SendFrame(pkt,aindex))
                    cout<<"#"<<flush;
            }
        }

        if(vdata.size>0){
            vdata.pts = vdata.pts - begintime;
            XData yuv = XMediaEncode::Get()->rgb_to_yuv(vdata);
            vdata.Drop();
            XData pkt = XMediaEncode::Get()->EncodecVideo(yuv);
            if(pkt.size>0){
                if(Xrtmp::Get()->SendFrame(pkt,vindex))
                    cout<<"*"<<flush;
            }
        }
    }
}

XController::~XController() {
};
XController::XController() {};


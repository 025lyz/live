//#include <QtCore/QCoreApplication>
#include<QtWidgets/QApplication>
#include <QAudioInput>
#include <iostream>
#include "Xrtmp.h"
#include <QThread>
#include <QtGui/QWindow>
#include "XMediaEncode.h"
#include "XAudioRecord.h"
#include "XVideoCaptur.h"
#include "XController.h"

using namespace std;

#if defined WIN32 || defined _WIN32
#include <windows.h>
#endif

#if defined __linux__

#include "unistd.h"
#include "xrtmpstreamer.h"

#endif

//系统内定义的宏 获取CPU数量
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

#define outurl "rtmp://122.112.143.244/lcx"

int main(int argc, char *argv[]) {


    QApplication a(argc,argv);
    XRtmpStreamer w;
    w.show();
    return a.exec();

    int sampleRate = 44100;
    int channels = 2;
    int sampleByte = 2;
    int nbsample = 1024;
    XVideoCaptur *xv = XVideoCaptur::Get(0);
    if(!xv->Init(0)) {
        cout<<"相机打开失败！"<<endl;
        return -1;
    }
    cout<<"相机打开成功！"<<endl;
    //视频格式转换上下文


    ///1 qt音频开始录制
    XAudioRecord *ar= XAudioRecord::Get();
    ar->sampleRate = sampleRate;
    ar->channels = channels;
    ar->sampleByte = sampleByte;
    ar->nbSamples = nbsample;

    if(!ar->Init()){
        cout<<"XAudioRecord初始化失败！"<<endl;
        return -1;
    }
    xv->Start();


    XMediaEncode *xe = XMediaEncode::Get();
    //视频初始化输出结构
    xe->SRCWIDTH = xv->width;
    xe->SRCHEIGHT = xv->height;
    xe->OUTHeight = xv->height;
    xe->OUTWIDTH = xv->width;
    //初始化转换
    if(!xe->InitScale()){
        return -1;
    }
    cout<<"初始化视频像素转换上下文成功！"<<endl;

//   AAc 编码必须是float类型的所以需要进行重采样
    xe->channels = channels;
    xe->nbsample = 1024;
    xe->sampleRate = sampleRate;
    xe->outSamplefmt = XAVSampleFormat::XAV_SAMPLE_FMT_FLTP;
    xe->inSamplefmt = XAVSampleFormat::XAV_SAMPLE_FMT_S16;
    if (!xe->InitResample()) {
        cout << "初始化重采样错误" << endl;
        return -1;
    }
    ar->Start();
    /**
     * 1.找到编码器
     * 2.分配句柄
     * 3.参数配置
     * 4.打开codec
     * */
    if (!xe->InitAudioCodec()) {
        cout << "编码器打开出错！" << endl;
        return -1;
    }

    if (!xe->InitVideoCodec()) {
        cout << "编码器打开出错！" << endl;
        return -1;
    }

    Xrtmp *xr = Xrtmp::Get(0);
    //创建封装器上下文
    if(!xr->Init(outurl)) return -1;

    //初始化视频流
    int vindex = 0;
    vindex = xr->addStream(xe->cctx);
    if(vindex<0)return -1;

    //初始化音频流
    int aindex = 0;
    aindex = xr->addStream(xe->ac);
    if(aindex<0)return -1;

    //打开网络IO
    if(!xr->sendHeader()) return -1;

    //一次读取一帧音频的字节数
    long long begintime  = GetCurTime();  //放在线程之前保证pts是正数

    for (;;) {


       //一次读取一帧音频
       XData adata = ar->Pop();
       XData vdata = xv->Pop();
       if(adata.size<=0 && vdata.size<=0)
       {
           QThread::msleep(1);
           continue;
       }
       //音频处理
       if(adata.size>0){
           adata.pts = adata.pts - begintime;

           XData pcm = xe->Resample(adata);
           adata.Drop();

           //        pts运算
           //  nb_sample/sample_rate  = 一帧音频的秒数sec
           // timebase  pts = sec * timebase.den
           //封装推流
           XData pkt = xe->EncodecAudio(pcm);
           if(pkt.size>0){
               if(xr->SendFrame(pkt,aindex))
                   cout<<"#"<<flush;
           }
       }

        if(vdata.size>0){
            vdata.pts = vdata.pts - begintime;
            XData yuv = xe->rgb_to_yuv(vdata);
            vdata.Drop();
            XData pkt = xe->EncodecVideo(yuv);
            if(pkt.size>0){
                if(xr->SendFrame(pkt,vindex))
                    cout<<"*"<<flush;
            }
        }

    }
    getchar();
    return a.exec();
}


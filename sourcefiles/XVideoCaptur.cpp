//
// Created by lcx on 17/1/22.
//
#include"XVideoCaptur.h"
#include<iostream>
#include<opencv2/highgui.hpp>

using namespace cv;
using namespace std;

class CXVideoCaptur:public XVideoCaptur{
    void run() override{
        Mat frame;
        while(!isExit){
            if(!camra.read( frame)){
                msleep(1);
                continue;
            }
            if(frame.empty()){
                msleep(1);
                continue;
            }
            //确保数据是连续的
            fmutex.lock();
            for(int i = 0;i<filters.size();i++){
                Mat des;
                filters[i]->Filter(&frame,&des);
                frame = des;
            }
            fmutex.unlock();
            XData data((char *)frame.data,frame.cols*frame.rows*frame.elemSize(),GetCurTime());
            Push(data);
        }
    }
    //打开本地摄像机
    bool Init(int camIndex) override{
        camra.open(0);
        if (!camra.isOpened()) {
            cout<< "camra open faild!" << endl;
        }
        cout<< "camra open success" << endl;
        //获取像素信息
        width = camra.get(3);
        height = camra.get(4);
        fps = camra.get(CAP_PROP_FPS);
        if(fps==0) fps = 25;
        return true;
    }

    //打开网络摄像机
    bool Init(const char *inurl){
        camra.open(inurl);
        if (!camra.isOpened()) {
            cout<< "camra open faild!" << endl;
        }
        cout<< "camra open success" << endl;

        //获取像素信息
        width = camra.get(3);
        height = camra.get(4);
        fps = camra.get(CAP_PROP_FPS);
        if(fps==0) fps = 25;
        return true;
    }

    void Stop(){
        XDataThread::Stop();
        if(camra.isOpened()){
            camra.release();
        }
    }

private:
    VideoCapture camra;
};

XVideoCaptur *XVideoCaptur::Get(unsigned char index) {
    static CXVideoCaptur xc[255];
    return &xc[index];
}
XVideoCaptur::XVideoCaptur()  = default;
XVideoCaptur::~XVideoCaptur()  = default;

//
// Created by lcx on 17/1/22.
//
#include"XAudioRecord.h"
#include <QAudioInput>
#include <iostream>
#include <list>

using namespace std;

class CXAudioRecord:public XAudioRecord{
public:
    void run() override{
        cout<<"进入音频录制线程！"<<endl;
        int readsize = nbSamples * channels * sampleByte;
        char *buf = new char[readsize];
        while(!isExit){
            //读取音频
            int offset = input->bytesReady();  //产生阻塞
            if (offset < readsize) {
                QThread::msleep(1);
                io->read(buf, offset);
                continue;
            }
            int size = 0;
            while (size != readsize) {
                int len = io->read(buf + size, readsize - size);
                if (len < 0)break;
                size += len ;
            }

            if (size != readsize) continue;
            long long pts = GetCurTime();
            //已经读取到一帧音频

            Push(XData(buf,size,pts));
        }
        delete []buf;
    }

    bool Init() override{
        Stop();
        ///1 qt音频开始录制
        QAudioFormat fmt;
        fmt.setSampleRate(sampleRate);
        fmt.setChannelCount(channels);
        fmt.setSampleSize(sampleByte * 8);
        fmt.setCodec("audio/pcm");
        fmt.setByteOrder(QAudioFormat::LittleEndian);
        fmt.setSampleType(QAudioFormat::UnSignedInt);
        QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
        if (!info.isFormatSupported(fmt)) {
            cout << "Audio format not support!" << endl;
            fmt = info.nearestFormat(fmt);
        }
        input = new QAudioInput(fmt);
        //开始录制音频
        io = input->start();
        if(!io){
            return false;
        }
        return true;

    }
    void Stop(){
        XDataThread::Stop();
        if(input)
            input->stop();
        if(io)
            io->close();
        input = nullptr;
        io = nullptr;
    }

    QAudioInput *input = nullptr;
    QIODevice *io = nullptr;

};
XAudioRecord* XAudioRecord::Get(XAUDIOTYPE type,unsigned char index){
    static CXAudioRecord record[255];
    return &record[index];
}
XAudioRecord::XAudioRecord() = default;
XAudioRecord::~XAudioRecord() noexcept = default;
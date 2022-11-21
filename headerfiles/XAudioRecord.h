//
// Created by lcx on 17/1/22.
//

#ifndef PCMRTP_XAUDIORECORD_H
#define PCMRTP_XAUDIORECORD_H
#include "XDataThread.h"
enum XAUDIOTYPE{
    X_AUDIO_QT
};



class XAudioRecord:public XDataThread{
public:
    int sampleRate = 44100;
    int channels = 2;
    int sampleByte = 2;
    int nbSamples = 1024;


    //开始录制
    virtual bool Init() = 0;

    virtual void Stop() = 0;

    static XAudioRecord* Get(XAUDIOTYPE type = X_AUDIO_QT,unsigned char index = 0);


    virtual ~XAudioRecord();
protected:
    XAudioRecord();

};
#endif //PCMRTP_XAUDIORECORD_H

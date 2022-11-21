//
// Created by lcx on 17/1/22.
//
#include <cstring>
#include"XData.h"
extern "C"{
#include "libavutil/time.h"
}
void XData::Drop(){
    if(data)
        delete data;
    char* data = nullptr;
    int size = 0;
}

long long GetCurTime(){
    return av_gettime();
}
XData::XData(char *data, int size,long long pts) {
    this->data = new char[size];
    std::memcpy(this->data,data,size);
    this->size = size;
    this->pts = pts;
}
XData::XData() = default;
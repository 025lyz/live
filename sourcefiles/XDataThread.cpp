//
// Created by lcx on 17/1/22.
//


#include "XDataThread.h"
#include "QThread"
XData XDataThread::Pop() {
    mutex.lock();
    if(datas.empty()){
        mutex.unlock();
        return XData();
    }
    XData data = datas.front();
    datas.pop_front();
    mutex.unlock();
    return data;
}

void XDataThread::Push(XData data) {
    mutex.lock();
    if(datas.size()>MaxList){ //size会遍历可以修改数据结构优化性能。
        datas.front().Drop();
        datas.pop_front();
    }
    datas.push_back(data);
    mutex.unlock();
}

bool XDataThread::Start() {
    isExit = false;
    QThread::start();
    return true;
}

void XDataThread::Stop(){

    isExit = true;
    wait(); //保证线程安全
}

void XDataThread::clear() {
    mutex.lock();
    while(!datas.empty()){
        datas.front().Drop();
        datas.pop_front();
    }
    mutex.unlock();
}

XDataThread::XDataThread() =default;
XDataThread::~XDataThread() noexcept = default;

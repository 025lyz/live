//
// Created by lcx on 17/1/22.
//

#ifndef PCMRTP_XDATATHREAD_H
#define PCMRTP_XDATATHREAD_H

#include "QThread"
#include "XData.h"
#include "QMutex"
class XDataThread:public QThread {
public:
    //（缓冲列表大小）列表最大值
    int MaxList = 100;
    //在列表结尾插入

    virtual void Push(XData data) ;

    //读取列表中的数据  需要调用 XData.Drop()函数进行空间清理。
    virtual XData Pop() ;

    //启动线程
    virtual bool Start();
    //退出线程 并等待线程退出
    virtual void Stop();

    //
    virtual void clear();

    XDataThread();
    virtual ~XDataThread();
protected:
    //存放交互数据
    std::list<XData> datas;

    //交互数据列表大小
    int dataCount = 0;

    //互斥访问
    QMutex mutex;
    //处理线程退出
    bool isExit  = false;
};


#endif //PCMRTP_XDATATHREAD_H

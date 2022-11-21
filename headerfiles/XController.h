//
// Created by lcx on 19/1/22.
//

#ifndef PCMRTP_XCONTROLLER_H
#define PCMRTP_XCONTROLLER_H
#include "XDataThread.h"

class XController :public XDataThread{ //启动线程等不开放给用户
public:
    std::string err;
    std::string outUrl;
    int camIndex = -1;
    std::string inUrl;
    static XController *Get(){
        static XController xc;
        return &xc;
    }
    //设定磨皮等级
    virtual bool Set(std::string key,double value);

    virtual bool Start();

    virtual void Stop();

    void run();

    virtual ~XController();
protected:
    int vindex = 0;

    int aindex = 1;
    XController() ;
};


#endif //PCMRTP_XCONTROLLER_H

//
// Created by lcx on 17/1/22.
//

#ifndef PCMRTP_XVIDEOCAPTUR_H
#define PCMRTP_XVIDEOCAPTUR_H
#include "XData.h"
#include "XDataThread.h"
#include "XFilter.h"
#include <vector>

//头文件当中尽量不要引入命名空间
class XVideoCaptur:public XDataThread{
public:
    int width = 0;
    int height = 0;
    int fps = 0;

    //支持多路视频
    static XVideoCaptur *Get(unsigned char index = 0);

    virtual bool Init(int CamIndex = 0) = 0;

    virtual bool Init(const char *inurl) = 0;
    virtual void Stop() = 0;

    void AddFilter(XFilter *f){
        fmutex.lock();
        filters.push_back(f);
        fmutex.unlock();
    }

    virtual ~XVideoCaptur();
protected:
    QMutex fmutex;
    std::vector<XFilter*> filters;
    XVideoCaptur();
};
#endif //PCMRTP_XVIDEOCAPTUR_H

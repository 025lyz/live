//
// Created by lcx on 17/1/22.
//

#ifndef PCMRTP_XDATA_H
#define PCMRTP_XDATA_H
class XData{

public:
    long long pts = 0;
    char* data = nullptr;
    int size = 0;
    void Drop();
    XData();
    //创建空间并复制data内容
    XData(char *data,int size,long long pts = 0);
};
long long GetCurTime();
#endif //PCMRTP_XDATA_H

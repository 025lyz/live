//
// Created by lcx on 19/1/22.
//

#ifndef PCMRTP_XFILTER_H
#define PCMRTP_XFILTER_H
namespace cv{
    class Mat;
}

enum XFilterType{
    XBILATERAL //双边滤波
};

#include <string>
#include <map>
class XFilter {
public:
    static XFilter *Get(XFilterType type = XBILATERAL);

    virtual bool Filter(cv::Mat *src,cv::Mat *des) = 0;

    virtual bool Set(std::string ,double value);
    virtual ~XFilter();
protected:
    std::map<std::string,double> paras;
    XFilter();
};


#endif //PCMRTP_XFILTER_H

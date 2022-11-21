//
// Created by lcx on 19/1/22.
//

#ifndef PCMRTP_XBILATERALFILTER_H
#define PCMRTP_XBILATERALFILTER_H

#include "XFilter.h"
class XBilateralFilter :public XFilter{
public:
    XBilateralFilter();
    bool Filter(cv::Mat *src,cv::Mat *des);
    virtual ~XBilateralFilter();

};


#endif //PCMRTP_XBILATERALFILTER_H

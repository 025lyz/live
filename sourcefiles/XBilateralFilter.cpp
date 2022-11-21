//
// Created by lcx on 19/1/22.
//

#include "XBilateralFilter.h"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
using namespace cv;
bool XBilateralFilter::Filter(cv::Mat *src, cv::Mat *des) {
    double d = paras["d"];
    bilateralFilter(*src,*des,d,d*2,d/2);
    return true;
}

XBilateralFilter::XBilateralFilter() {
    paras.insert(std::make_pair("d",5)); //默认值 5
}
XBilateralFilter::~XBilateralFilter() = default;


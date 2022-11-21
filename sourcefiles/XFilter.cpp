//
// Created by lcx on 19/1/22.
//
#include <iostream>
#include "XFilter.h"
#include "XBilateralFilter.h"

XFilter* XFilter::Get(XFilterType type){
    static XBilateralFilter xbf;
    switch (type) {
        case XBILATERAL:
            return &xbf;
            break;
        default:
            break;
    }
    return 0;
}
XFilter::XFilter()  = default;
XFilter::~XFilter() noexcept = default;


bool XFilter::Set(std::string key, double value) {
    if(paras.find(key) == paras.end()){
        std::cout<<"para" << key <<"is not support!"<<std::endl;
        return false;
    }
    paras[key] = value;
    return true;
}


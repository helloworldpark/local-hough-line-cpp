//
//  FasterHough.cpp
//  faster-hough-line-cpp
//
//  Created by 박성현 on 31/07/2019.
//  Copyright © 2019 Sean Park. All rights reserved.
//

#include "LineFinder.hpp"


using namespace fh;

LineFinder::LineFinder(cv::Mat* rawImage, LineParams params) {
    this->params = params;
    
    preprocess(rawImage);
}

LineFinder::~LineFinder() {
    if (_worksheet != nullptr) {
        delete _worksheet;
    }
    
    if (_result != nullptr) {
        delete _result;
    }
}

cv::Mat* LineFinder::runStandardHough() {
    return nullptr;
}

cv::Mat* LineFinder::runFasterHough() {
    return nullptr;
}

void LineFinder::preprocess(cv::Mat* rawImage) {
    
}

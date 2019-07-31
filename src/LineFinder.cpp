//
//  FasterHough.cpp
//  faster-hough-line-cpp
//
//  Created by 박성현 on 31/07/2019.
//  Copyright © 2019 Sean Park. All rights reserved.
//

#include "LineFinder.hpp"
#include "Helper.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>


using namespace fh;

LineFinder::LineFinder(cv::Mat* rawImage, LineParams params) {
    this->params = params;
    
    preprocess(rawImage);
}

LineFinder::~LineFinder() {
    if (_worksheet != nullptr) {
        _worksheet->release();
    }
    
    if (_result != nullptr) {
        _result->release();
    }
}

cv::Mat* LineFinder::runStandardHough() {
    return nullptr;
}

cv::Mat* LineFinder::runFasterHough() {
    return nullptr;
}

void LineFinder::preprocess(cv::Mat* rawImage) {
    cv::Size size = getProcessingSize(*rawImage, params.worksheetLength);
    int channel = rawImage->dims;
    
    // Resize
    cv::Mat* resized = new cv::Mat(size, rawImage->type());
    cv::resize(*rawImage, *resized, size);
    // Bilateral
    cv::Mat* bilateral = new cv::Mat(resized->size(), resized->type());
    cv::bilateralFilter(*resized, *bilateral, 0, params.bilateralColorS, params.bilateralSpaceS);
    resized->release();
    // Grayscale
    
    cv::Mat* grayImage = new cv::Mat(size, CV_8UC1);
    if (channel == 3) {
        cv::cvtColor(*bilateral, *grayImage, cv::COLOR_BGR2GRAY);
    } else {
        cv::cvtColor(*bilateral, *grayImage, cv::COLOR_BGRA2GRAY);
    }
    bilateral->release();
    
    // Canny edge
    cv::Mat* edgeImage = new cv::Mat(size, resized->type());
    cv::Canny(*grayImage, *edgeImage, params.cannyThreshold1, params.cannyThreshold2, params.cannyAperture, params.cannyUseL2Gradient);
    
    _worksheet = edgeImage;
    
    cv::namedWindow("Preprocess");
    cv::imshow("Preprocess", *_worksheet);
    cv::waitKey(0);
}

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
#include <vector>


using namespace fh;

LineFinder::LineFinder(cv::Mat* rawImage, LineParams params) {
    this->params = params;
    
    preprocess(rawImage);
}

LineFinder::~LineFinder() {
    releaseImage(&_worksheet);
    releaseImage(&_result);
}

// https://docs.opencv.org/4.1.0/d5/df9/samples_2cpp_2tutorial_code_2ImgTrans_2houghlines_8cpp-example.html#a8
cv::Mat& LineFinder::runStandardHough() {
    releaseImage(&_result);
    
    std::vector<cv::Vec3f> lines;
    cv::HoughLines(*_worksheet,
                   lines,
                   params.houghResolutionRho,
                   CV_PI / params.houghResolutionTheta,
                   params.houghThreshold());
    
    _result = new cv::Mat(_worksheet->size(), CV_8UC3);
    cv::cvtColor(*_worksheet, *_result, cv::COLOR_GRAY2BGR);

    for (auto line: lines) {
        drawHoughLine(*_result, line);
    }
    
    return *_result;
}

cv::Mat& LineFinder::runFasterHough() {
    releaseImage(&_result);
    
    return *_result;
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
}

cv::Vec3d LineFinder::convertFriendly(cv::Vec3d& line) {
    return cv::Vec3d(line[0], 90.0 - line[0] * (CV_PI / 180.0), line[2]);
}

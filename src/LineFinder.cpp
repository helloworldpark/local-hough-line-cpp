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
    
    // Prepare cos, sin
    std::vector<cv::Vec3d> trigs(params.houghResolutionTheta);
    for (int t = 0; t < params.houghResolutionTheta; t++) {
        double theta = ((double)t) / ((double)params.houghResolutionTheta);
        theta *= CV_PI;
        trigs.push_back(cv::Vec3d(theta, cos(theta), sin(theta)));
    }
    
    // Prepare rhos
    int H = (int)hypot(_worksheet->size[0], _worksheet->size[1]);
    std::vector<double> rhos;
    int rhoCount = floor(((double)H) / ((double)params.houghResolutionRho));
    for (int r = -rhoCount; r <= rhoCount; r++) {
        rhos.push_back(r * ((double)params.houghResolutionRho));
    }
    
    std::vector<cv::Vec3d> lines;
    // Iterate for theta
    for (auto theta: trigs) {
        // Iterate for rho
        for (auto rho: rhos) {
            // Check if this rho and theta is meaningful
            if (!isFindingMeaningful(rho, theta)) {
                continue;
            }
            
            cv::Vec3d line;
            // If success to find a line, append it
            bool didFind = didFindLine(_worksheet, rho, theta, line);
            if (didFind) {
                lines.push_back(line);
            }
        }
    }
    
    // Plot
    _result = new cv::Mat(_worksheet->size(), CV_8UC3);
    cv::cvtColor(*_worksheet, *_result, cv::COLOR_GRAY2BGR);
    
    for (auto line: lines) {
        drawHoughLine(*_result, line);
    }
    
    return *_result;
}

bool LineFinder::isFindingMeaningful(double rho, cv::Vec3d& theta) {
    return false;
}

bool LineFinder::didFindLine(cv::Mat* image, double rho, cv::Vec3d& theta, cv::Vec3d& line) {
    return false;
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

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
#include <chrono>
#include <iostream>
#include <fast_math.hpp>


using namespace fh;

typedef cv::Vec3f Line; // rho, theta, votes
typedef cv::Vec3f Angle; // theta, cos, sin

const float pi2 = CV_PI * 0.5f;

LineFinder::LineFinder(cv::Mat* rawImage, LineParams params) {
    this->params = params;
    cv::Size size = getProcessingSize(*rawImage, params.worksheetLength);
    this->_diagonalLength = hypot(size.width, size.height);
    this->_diagonalAngle = atan2(rawImage->size[1], rawImage->size[0]);
    
    preprocess(rawImage);
}

LineFinder::~LineFinder() {
    releaseImage(&_worksheet);
    releaseImage(&_result);
}

// https://docs.opencv.org/4.1.0/d5/df9/samples_2cpp_2tutorial_code_2ImgTrans_2houghlines_8cpp-example.html#a8
cv::Mat& LineFinder::runStandardHough() {
    releaseImage(&_result);
    
    auto start = std::chrono::system_clock::now();
    
    std::vector<cv::Vec3f> lines;
    cv::HoughLines(*_worksheet,
                   lines,
                   params.houghResolutionRho,
                   CV_PI / params.houghResolutionTheta,
                   params.houghThreshold());
    
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Standard Hough: " << diff.count() << std::endl;
    
    
    _result = new cv::Mat(_worksheet->size(), CV_8UC3);
    cv::cvtColor(*_worksheet, *_result, cv::COLOR_GRAY2BGR);

    for (auto line: lines) {
        drawHoughLine(*_result, line);
    }
    
    return *_result;
}

cv::Mat& LineFinder::runFasterHough() {
    releaseImage(&_result);
    
    auto start = std::chrono::system_clock::now();
    
    // Prepare cos, sin
    std::vector<Angle> trigs(params.houghResolutionTheta);
    for (int t = 0; t < params.houghResolutionTheta; t++) {
        float theta = ((float)t) / ((float)params.houghResolutionTheta);
        theta *= CV_PI;
        trigs[t] = Angle(theta, cos(theta), sin(theta));
    }
    
    // Prepare rhos
    int H = (int)diagonalLength();
    int rhoCount = floor(H / ((float)params.houghResolutionRho));
    std::vector<float> rhos;
    for (int r = -rhoCount; r <= rhoCount; r++) {
        rhos.push_back(r * ((float)params.houghResolutionRho));
    }
    
    float diagonalAngle = atan2(_worksheet->rows, _worksheet->cols);
    cv::Size imgSize = cv::Size(_worksheet->cols, _worksheet->rows);
    
    std::vector<Line> lines;
    // Iterate for theta
    double ttFind = 0.0;
    
    int threshold = params.houghThreshold();
    for (auto theta: trigs) {
        // Iterate for rho
        for (auto rho: rhos) {
            // Check if this rho and theta is meaningful
            bool isMeaningful = isFindingMeaningful(imgSize, rho, theta, diagonalAngle);
            if (!isMeaningful) {
                continue;
            }
            
            Line line;
            // If success to find a line, append it
            auto didFindStart = std::chrono::system_clock::now();
            bool didFind = didFindLine(_worksheet, rho, theta, line, threshold);
            auto didFindEnd = std::chrono::system_clock::now();
            std::chrono::duration<double> diff = didFindEnd - didFindStart;
            ttFind += diff.count();
            if (didFind) {
                lines.push_back(line);
            }
        }
    }
    
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Faster Hough: " << diff.count() << std::endl;
    std::cout << " -didFindLine: " << ttFind << std::endl;
    
    // Plot
    _result = new cv::Mat(_worksheet->size(), CV_8UC3);
    cv::cvtColor(*_worksheet, *_result, cv::COLOR_GRAY2BGR);
    
    for (auto line: lines) {
        drawHoughLine(*_result, line);
    }
    
    return *_result;
}



bool LineFinder::isFindingMeaningful(cv::Size& imageSize, float rho, cv::Vec3f& theta, float diagonalAngle) {
    float t = theta[0];
    float tcos = theta[1];
    float tsin = theta[2];
    
    // Assuming that 0 <= t < PI
    if (rho >= 0) {
        if (t < diagonalAngle) {
            return rho < imageSize.width / tcos;
        }
        return rho < imageSize.height * tsin;
    }
    
    if (t < pi2) {
        return false;
    } else if (t < (pi2 + diagonalAngle)) {
        return rho >= -imageSize.height * tsin;
    }
    return rho >= imageSize.width * tcos;
}

bool LineFinder::didFindLine(cv::Mat* image, float rho, Angle& theta, Line& line, int& threshold) {
    double tcos = theta[1];
    double tsin = theta[2];
    
    float multiplier = MAX(image->rows, image->cols);
    cv::Point2f center(rho * tcos, rho * tsin);
    cv::Point2f direction(multiplier * tcos, multiplier * tsin);
    
    cv::Point pt0(cvRound(center.x - multiplier * tsin), cvRound(center.y + multiplier * tcos));
    cv::Point pt1(cvRound(center.x + multiplier * tsin), cvRound(center.y - multiplier * tcos));
    
//    std::cout << "Rho: " << rho << ", Theta: " << (int)(theta[0] * (1800.0 / CV_PI)) << ", pt0(" << pt0.x << ", " << pt0.y << "), pt1(" << pt1.x << ", " << pt1.y << ")" << std::endl;
    
    line[0] = rho;
    line[1] = theta[0];
    line[2] = 0;

    cv::LineIterator iterator(*image, pt0, pt1);
    
    if (iterator.count <= threshold) {
        return false;
    }
    
    for (int i = 0; i < iterator.count; i++, ++iterator) {
        cv::Point pos = iterator.pos();
        bool isPointLine = (*iterator.ptr != 0) || isLine(image, pos);

        if (isPointLine) {
            // Accumulate votes
            line[2] += 1;
        } else {
            if (line[2] <= threshold) {
                // If votes are lower than threshold
                // Discard
                line[2] = 0;
            } else {
                // Else
                // Found a line, finish
                break;
            }
        }
    }
    return line[2] > threshold;
}



bool LineFinder::isLine(cv::Mat* image, cv::Point& p) {
    if (p.x == 0 || p.x == image->cols) {
        return false;
    }
    
    if (p.y == 0 || p.y == image->rows) {
        return false;
    }
    uchar* data = nullptr;
    data = &image->data[(p.y - 1) * image->cols + (p.x - 1)];
    for (int i = 0; i < 3; i++) {
        if (data[i] != 0) {
            return true;
        }
    }
    
    data = &image->data[p.y * image->cols + (p.x - 1)];
    for (int i = 0; i < 3; i++) {
        if (data[i] != 0) {
            return true;
        }
    }
    
    data = &image->data[(p.y + 1) * image->cols + (p.x - 1)];
    for (int i = 0; i < 3; i++) {
        if (data[i] != 0) {
            return true;
        }
    }
    
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
//    cv::bilateralFilter(*resized, *bilateral, 0, params.bilateralColorS, params.bilateralSpaceS);
    cv::GaussianBlur(*resized, *bilateral, cv::Size(7, 7), params.bilateralColorS);
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

cv::Vec3f LineFinder::convertFriendly(cv::Vec3f& line) {
    return cv::Vec3f(line[0], 90.0f - line[0] * (CV_PI / 180.0f), line[2]);
}

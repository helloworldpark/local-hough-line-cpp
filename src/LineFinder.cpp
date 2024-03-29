//
//  FasterHough.cpp
//  faster-hough-line-cpp
//
//  Created by 박성현 on 31/07/2019.
//  Copyright © 2019 Sean Park. All rights reserved.
//

#include <chrono>
#include <iostream>
#include <string>
#include <fast_math.hpp>
#include <opencv2/imgproc.hpp>
#include "LineFinder.hpp"
#include "Helper.hpp"6


using namespace fh;


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
    
    Timer timer("Standard Hough");
    
    std::vector<cv::Vec3f> lines;
    cv::HoughLines(*_worksheet,
                   lines,
                   params.houghResolutionRho,
                   CV_PI / params.houghResolutionTheta,
                   params.houghThreshold());
    
    timer.stop();
    
    _result = new cv::Mat(_worksheet->size(), CV_8UC3);
    cv::cvtColor(*_worksheet, *_result, cv::COLOR_GRAY2BGR);

    for (auto& line: lines) {
        drawHoughLine(*_result, line);
    }
    
    return *_result;
}

cv::Mat& LineFinder::runStandardLocalHough() {
    releaseImage(&_result);
    
    Timer timer("Standard Local Hough");
    
    std::vector<cv::Vec3f> lines;
    cv::HoughLines(*_worksheet,
                   lines,
                   params.houghResolutionRho,
                   CV_PI / params.houghResolutionTheta,
                   params.houghThreshold());
    
    // Filter candidate lines by testing locality
    // Prepare trigonometric function table for faster calculation
    std::vector<Angle> trigs;
    prepareCosSin(trigs);
    
    // Test each line for locality
    int localThreshold = params.houghLocalThreshold();
    std::vector<Line> realLines;
    for (auto& line: lines) {
        Line realLine;
        
        // Get precaculated cos, sin values
        int angleIdx = cvRound(line[1] * params.houghResolutionTheta / CV_PI);
        auto& angle = trigs[angleIdx];
        
        // Locality is defined as such:
        //   If there exists a group of consecutive pixels along the line,
        //   then that 'candidate line' is locally a line.
        //   We decide the consecutivity by thresholding, i.e. over T pixels should be consecutive.
        if (didFindLine(_worksheet, line[0], angle, realLine, localThreshold)) {
            realLines.push_back(realLine);
        }
    }
    
    timer.stop();
    
    
    _result = new cv::Mat(_worksheet->size(), CV_8UC3);
    cv::cvtColor(*_worksheet, *_result, cv::COLOR_GRAY2BGR);
    
    for (auto& line: realLines) {
        drawHoughLine(*_result, line);
    }
    
    return *_result;
}

cv::Mat& LineFinder::runNaiveLocalHough() {
    releaseImage(&_result);
    
    Timer timer("Naive Local Hough");
    
    // Prepare cos, sin
    std::vector<Angle> trigs;
    prepareCosSin(trigs);
    
    // Prepare rhos
    int H = (int)diagonalLength();
    int rhoCount = floor(H / ((float)params.houghResolutionRho));
    std::vector<float> rhos;
    for (int r = -rhoCount; r <= rhoCount; r++) {
        rhos.push_back(r * ((float)params.houghResolutionRho));
    }
    
    float diagonalAngle = atan2(_worksheet->rows, _worksheet->cols);
    cv::Size imgSize = cv::Size(_worksheet->cols, _worksheet->rows);
    int threshold = params.houghLocalThreshold();
    
    std::vector<Line> lines;
    // Iterate for theta
    for (auto& theta: trigs) {
        // Iterate for rho
        for (auto& rho: rhos) {
            // Check if this rho and theta is meaningful
            bool isMeaningful = isFindingMeaningful(imgSize, rho, theta, diagonalAngle);
            if (!isMeaningful) {
                continue;
            }
            
            Line line;
            // If success to find a line, append it
            bool didFind = didFindLine(_worksheet, rho, theta, line, threshold);
            if (didFind) {
                lines.push_back(line);
            }
        }
    }
    
    timer.stop();
    
    // Plot
    _result = new cv::Mat(_worksheet->size(), CV_8UC3);
    cv::cvtColor(*_worksheet, *_result, cv::COLOR_GRAY2BGR);
    
    for (auto& line: lines) {
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
    
    int multiplier = MAX(image->rows, image->cols);
    cv::Point2f center(rho * tcos, rho * tsin);
    
    cv::Point pt0(cvRound(center.x - multiplier * tsin), cvRound(center.y + multiplier * tcos));
    cv::Point pt1(cvRound(center.x + multiplier * tsin), cvRound(center.y - multiplier * tcos));
    
    line[0] = rho;
    line[1] = theta[0];
    line[2] = 0;

    cv::LineIterator iterator(*image, pt0, pt1);
    
    if (iterator.count <= threshold) {
        return false;
    }
    
    int votes = 0;
    for (int i = 0; i < iterator.count; i++, ++iterator) {
        cv::Point pos = iterator.pos();
        bool isPointLine = (*iterator.ptr != 0) || isLine(image, pos);

        if (isPointLine) {
            // Accumulate votes
            ++votes;
        } else {
            // If votes are bigger than threshold
            // Append to line candidate's votes
            // Else
            // Discard
            if (votes > threshold) {
                line[2] += votes;
            }
            votes = 0;
        }
    }
    return line[2] > threshold;
}



bool LineFinder::isLine(cv::Mat* image, cv::Point& p) {
    if (p.x <= 0 || p.x >= image->cols - 1) {
        return false;
    }
    
    if (p.y <= 0 || p.y >= image->rows - 1) {
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
    Timer timer("Preprocess");
    cv::Size size = getProcessingSize(*rawImage, params.worksheetLength);
    int channel = rawImage->dims;
    
    // Resize
    cv::Mat* resized = new cv::Mat(size, rawImage->type());
    cv::resize(*rawImage, *resized, size);
    // Bilateral or Gaussian
    // It's just a matter of choice, I think,
    // but Bilateral Filtering was much better for extracting lines, in average.
    // Instead, it might be(and most of the case, yes) slower than smoothing images using Gaussian Filtering.
    cv::Mat* bilateral = new cv::Mat(resized->size(), resized->type());
    cv::bilateralFilter(*resized, *bilateral, params.bilateralSpaceS, params.bilateralColorS, params.bilateralSpaceS);
//    cv::GaussianBlur(*resized, *bilateral, cv::Size(7, 7), params.bilateralColorS);
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
    timer.stop();
}

cv::Mat& LineFinder::preprocessedImage() {
    return *_worksheet;
}

void LineFinder::prepareCosSin(std::vector<Angle>& table) {
    // Assume houghResolutiuonTheta is a multiple of 180
    
    int idx45 = (params.houghResolutionTheta / 4);
    int idx90 = (params.houghResolutionTheta / 2);
    
    // Calculate cos, sin for [0, 45] (or [0, pi/4]
    for (int i = 0; i <= idx45; i++) {
        float theta = ((float)i) / ((float)params.houghResolutionTheta) * CV_PI;
        table.push_back(Angle(theta, cos(theta), sin(theta)));
    }
    
    // Reuse the results
    // cos(t + pi/4) = sin(t)
    // sin(t + pi/4) = cos(t)
    for (int i = idx45+1; i <= idx90; i++) {
        auto& a = table[idx90 - i];
        float theta = ((float)i) / ((float)params.houghResolutionTheta) * CV_PI;
        table.push_back(Angle(theta, a[2], a[1]));
    }
    
    // Reuse the results
    // cos(t + pi/2) = -sin(t)
    // sin(t + pi/2) = cos(t)
    for (int i = idx90 + 1; i < params.houghResolutionTheta; i++) {
        auto& a = table[i - idx90];
        float theta = ((float)i) / ((float)params.houghResolutionTheta) * CV_PI;
        table.push_back(Angle(theta, -a[2], a[1]));
    }
}

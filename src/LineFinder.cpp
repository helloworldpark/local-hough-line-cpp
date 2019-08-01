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


using namespace fh;

typedef cv::Vec3d Line; // rho, theta, votes
typedef cv::Vec3d Angle; // theta, cos, sin

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
        double theta = ((double)t) / ((double)params.houghResolutionTheta);
        theta *= CV_PI;
        trigs[t] = Angle(theta, cos(theta), sin(theta));
    }
    
    // Prepare rhos
    int H = (int)diagonalLength();
    int rhoCount = floor(((double)H) / ((double)params.houghResolutionRho));
    std::vector<double> rhos;
    for (int r = -rhoCount; r <= rhoCount; r++) {
        rhos.push_back(r * ((double)params.houghResolutionRho));
    }
    
    double diagonalAngle = atan2(_worksheet->rows, _worksheet->cols);
    cv::Size imgSize = _worksheet->size();
    
    std::vector<Line> lines;
    // Iterate for theta
    for (auto theta: trigs) {
        // Iterate for rho
        for (auto rho: rhos) {
            // Check if this rho and theta is meaningful
            if (!isFindingMeaningful(imgSize, rho, theta, diagonalAngle)) {
                continue;
            }
            
            Line line;
            // If success to find a line, append it
            if (didFindLine(_worksheet, rho, theta, line)) {
                lines.push_back(line);
            }
        }
    }
    
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    std::cout << "Faster Hough: " << diff.count() << std::endl;
    
    // Plot
    _result = new cv::Mat(_worksheet->size(), CV_8UC3);
    cv::cvtColor(*_worksheet, *_result, cv::COLOR_GRAY2BGR);
    
    for (auto line: lines) {
        drawHoughLine(*_result, line);
    }
    
    return *_result;
}

bool LineFinder::isFindingMeaningful(cv::Size& imageSize, double rho, cv::Vec3d& theta, double diagonalAngle) {
    double t = theta[0];
    double tcos = theta[1];
    double tsin = theta[2];
    
    // Assuming that 0 <= t < PI
    if (rho > 0) {
        if (t < diagonalAngle) {
            return rho <= imageSize.width / tcos;
        } else if (t < CV_PI * 0.5) {
            return rho <= imageSize.height / tsin;
        }
        return rho <= imageSize.height * tsin;
    }
    
    if (t < CV_PI * 0.5) {
        return false;
    } else if (t < (CV_PI * 0.5 + diagonalAngle)) {
        return rho >= -imageSize.height * tsin;
    }
    
    return rho >= -imageSize.width * tcos;
}

bool LineFinder::didFindLine(cv::Mat* image, double rho, Angle& theta, Line& line) {
    double tcos = theta[1];
    double tsin = theta[2];
    
    cv::Point pt0;
    cv::Point pt1;
    
    if (tcos == 0.0) {
        // It is horizontal
        pt0 = cv::Point(0, (int)round(rho / tsin));
        pt1 = cv::Point(image->cols, (int)round(rho / tsin));
    } else if (tsin == 0.0) {
        // It is vertical
        pt0 = cv::Point((int)round(rho / tcos), 0);
        pt1 = cv::Point((int)round(rho / tcos), image->rows);
    } else {
        // Find x = -tan(theta) * y + rho / cos(theta)
        pt0 = cv::Point(0, (int)round(rho / tsin));
        pt1 = cv::Point((int)round(rho / tcos), 0);
    }
    
    line[0] = rho;
    line[1] = theta[0];
    line[2] = 0;

    cv::LineIterator iterator(*image, pt0, pt1);
    
    int votes = 0;
    for (int i = 0; i < iterator.count; i++, ++iterator) {
        cv::Point pos = iterator.pos();
        bool isPointLine = isLine(image, pos);
        if (isPointLine) {
            // Accumulate accumulate
            votes += 1;
        } else {
            if (votes <= params.houghThreshold()) {
                // If votes are lower than threshold
                // Discard
                votes = 0;
            } else {
                // Else
                // Found a line, increase votes
                line[2] += votes;
            }
        }
    }
    
    return line[2] > 0;
}



bool LineFinder::isLine(cv::Mat* image, cv::Point& p) {
    bool result = false;
    uchar values[Direction::COUNT];
    values[Direction::CENTER] = *(image->ptr(p.y, p.x));
    
    if (p.x == 0) {
        // Left boundary
        values[Direction::W] = values[Direction::CENTER];
        values[Direction::NW] = values[Direction::CENTER];
        values[Direction::SW] = values[Direction::CENTER];
        values[Direction::E] = *(image->ptr(p.y, p.x + 1));
        if (p.y == 0) {
            // Upper boundary
            values[Direction::NE] = values[Direction::CENTER];
            values[Direction::N] = values[Direction::CENTER];
            values[Direction::S] = *(image->ptr(p.y + 1, p.x));
            values[Direction::SE] = *(image->ptr(p.y + 1, p.x + 1));
        } else if (p.y == image->rows - 1) {
            // Lower boundary
            values[Direction::NE] = *(image->ptr(p.y - 1, p.x + 1));
            values[Direction::N] = *(image->ptr(p.y - 1, p.x));
            values[Direction::S] = values[Direction::CENTER];
            values[Direction::SE] = values[Direction::CENTER];
        } else {
            values[Direction::NE] = *(image->ptr(p.y - 1, p.x + 1));
            values[Direction::N] = *(image->ptr(p.y - 1, p.x));
            values[Direction::S] = *(image->ptr(p.y + 1, p.x));
            values[Direction::SE] = *(image->ptr(p.y + 1, p.x + 1));
        }
    } else if (p.x == image->cols - 1) {
        // Right boundary
        values[Direction::E] = values[Direction::CENTER];
        values[Direction::NE] = values[Direction::CENTER];
        values[Direction::SE] = values[Direction::CENTER];
        values[Direction::W] = *(image->ptr(p.y, p.x - 1));
        if (p.y == 0) {
            // Upper boundary
            values[Direction::NW] = values[Direction::CENTER];
            values[Direction::N] = values[Direction::CENTER];
            values[Direction::S] = *(image->ptr(p.y + 1, p.x));
            values[Direction::SW] = *(image->ptr(p.y + 1, p.x - 1));
        } else if (p.y == image->rows - 1) {
            // Lower boundary
            values[Direction::NW] = *(image->ptr(p.y - 1, p.x - 1));
            values[Direction::N] = *(image->ptr(p.y - 1, p.x));
            values[Direction::S] = values[Direction::CENTER];
            values[Direction::SW] = values[Direction::CENTER];
        } else {
            values[Direction::NW] = *(image->ptr(p.y - 1, p.x - 1));
            values[Direction::N] = *(image->ptr(p.y - 1, p.x));
            values[Direction::S] = *(image->ptr(p.y + 1, p.x));
            values[Direction::SW] = *(image->ptr(p.y + 1, p.x - 1));
        }
    } else {
        values[Direction::N] = *(image->ptr(p.y - 1, p.x));
        values[Direction::S] = *(image->ptr(p.y + 1, p.x));
        values[Direction::W] = *(image->ptr(p.y, p.x - 1));
        values[Direction::E] = *(image->ptr(p.y, p.x + 1));
        values[Direction::NW] = *(image->ptr(p.y - 1, p.x - 1));
        values[Direction::NE] = *(image->ptr(p.y - 1, p.x + 1));
        values[Direction::SW] = *(image->ptr(p.y + 1, p.x - 1));
        values[Direction::SE] = *(image->ptr(p.y + 1, p.x + 1));
    }
    
    for (int i = 0; i < Direction::COUNT; i++) {
        if (values[i] == 0xff) {
            return true;
        }
    }
    
    return result;
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

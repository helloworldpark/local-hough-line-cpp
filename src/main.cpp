//
//  main.cpp
//  faster-hough-line-cpp
//
//  Created by 박성현 on 31/07/2019.
//  Copyright © 2019 Sean Park. All rights reserved.
//

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include "LineFinder.hpp"

int main(int argc, const char * argv[]) {
    
    std::string imgName("images/test0.jpg");
    
    cv::Mat image;
    image = cv::imread(imgName, cv::IMREAD_COLOR);
    
    if (image.empty()) {
        std::cout << "Failed to open image" << std::endl;
        return -1;
    }
    
    fh::LineFinder* lineFinder = new fh::LineFinder(&image);
    
    cv::Mat& standardHough = lineFinder->runStandardHough();
    cv::namedWindow("Standard Hough");
    cv::imshow("Standard Hough", standardHough);
    
    cv::Mat& fastHough = lineFinder->runNaiveLocalHough();
    cv::namedWindow("Fast Hough");
    cv::imshow("Fast Hough", fastHough);
    cv::waitKey(0);

    delete lineFinder;
    
    return 0;
}

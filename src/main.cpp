//
//  main.cpp
//  faster-hough-line-cpp
//
//  Created by 박성현 on 31/07/2019.
//  Copyright © 2019 Sean Park. All rights reserved.
//

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include "LineFinder.hpp"
#include "Visualizer.hpp"

int main(int argc, const char * argv[]) {
    
    std::string imgName("images/test0.jpg");
    
    cv::Mat image;
    image = cv::imread(imgName, cv::IMREAD_COLOR);
    
    if (image.empty()) {
        std::cout << "Failed to open image" << std::endl;
        return -1;
    }
    
    fh::show("Original", image);
    fh::LineFinder* lineFinder = new fh::LineFinder(&image);
    
    cv::Mat& standardHough = lineFinder->runStandardHough();
    fh::show("Standard Hough", lineFinder->preprocessedImage(), standardHough);
    
    cv::Mat& standardLocalHough = lineFinder->runStandardLocalHough();
    fh::show("Standard Local Hough", lineFinder->preprocessedImage(), standardLocalHough);
    
    cv::Mat& naiveLocalHough = lineFinder->runNaiveLocalHough();
    fh::show("Naive Local Hough", lineFinder->preprocessedImage(), naiveLocalHough);
    fh::waitKey();

    delete lineFinder;
    
    return 0;
}

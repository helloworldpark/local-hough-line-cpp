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
    
    const std::string imgDir("images/");
    const std::string imgName("test1");
    const std::string imgExt(".jpg");
    
    std::string imgSrcPath = imgDir + imgName + imgExt;
    cv::Mat image = cv::imread(imgSrcPath, cv::IMREAD_COLOR);
    
    if (image.empty()) {
        std::cout << "Failed to open image: " << imgSrcPath << std::endl;
        return -1;
    }
    
    fh::show("Original", image);
    
    fh::LineFinder* lineFinder = new fh::LineFinder(&image);
    
    const std::string imgResultDir("images/results/");
    
    cv::Mat& standardHough = lineFinder->runStandardHough();
    fh::show("Standard Hough(Left-click for results)", lineFinder->preprocessedImage(), standardHough);
    std::string saveStandardHough = imgResultDir + imgName + "_stdHough.png";
    fh::save(saveStandardHough, standardHough);
    
    cv::Mat& standardLocalHough = lineFinder->runStandardLocalHough();
    fh::show("Standard Local Hough(Left-click for results)", lineFinder->preprocessedImage(), standardLocalHough);
    std::string saveStandardLocalHough = imgResultDir + imgName + "_stdLocalHough.png";
    fh::save(saveStandardLocalHough, standardLocalHough);
    
    cv::Mat& naiveLocalHough = lineFinder->runNaiveLocalHough();
    fh::show("Naive Local Hough(Left-click for results)", lineFinder->preprocessedImage(), naiveLocalHough);
    std::string saveNaiveLocalHough = imgResultDir + imgName + "_naiveLocalHough.png";
    fh::save(saveNaiveLocalHough, naiveLocalHough);
    
    fh::waitKey();

    delete lineFinder;
    
    return 0;
}

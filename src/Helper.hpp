//
//  Helper.hpp
//  faster-hough-line-cpp
//
//  Created by 박성현 on 31/07/2019.
//  Copyright © 2019 Sean Park. All rights reserved.
//

#ifndef Helper_hpp
#define Helper_hpp

#include <iostream>
#include <string>
#include <chrono>
#include <opencv2/core.hpp>

namespace fh {
    
    typedef std::chrono::time_point<std::chrono::steady_clock> Time;
    typedef std::chrono::duration<std::chrono::nanoseconds> Nanoseconds;
    
    class Timer {
        constexpr static const double million = 1000000.0;
    public:
        Timer(std::string name) {
            this->name = name;
            start();
        }
        
        void start() {
            timeStart = std::chrono::steady_clock::now();
        }
        
        void stop() {
            timeEnd = std::chrono::steady_clock::now();
            auto rawElapsed = timeEnd - timeStart;
            std::cout << "[Timer] " << name << ": " << rawElapsed.count() / million << "ms" << std::endl;
        }
    private:
        Time timeStart;
        Time timeEnd;
        std::string name;
    };
    
    
    cv::Size getProcessingSize(cv::Mat& image, int minLength);
    
    void releaseImage(cv::Mat** image);
    void drawHoughLine(cv::Mat& image, cv::Vec3f& line);
    void drawHoughLine(cv::Mat& image, cv::Vec3d& line);
}


#endif /* Helper_hpp */

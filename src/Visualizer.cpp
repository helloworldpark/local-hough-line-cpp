//
//  Visualizer.cpp
//  local-hough-line-cpp
//
//  Created by 박성현 on 02/08/2019.
//  Copyright © 2019 Sean Park. All rights reserved.
//

#include "Visualizer.hpp"
#include <opencv2/highgui.hpp>
#include <unordered_map>

namespace fh {
    static std::unordered_map<std::string, std::pair<cv::Mat, cv::Mat>> nameImage;
    
    static void onMouseCallback(int event, int x, int y, int flags, void* userdata) {
        std::string name((char*)userdata);
        
        if (event == cv::EVENT_LBUTTONUP) {
            cv::imshow(name, nameImage[name].first);
        } else if (event == cv::EVENT_LBUTTONDOWN) {
            cv::imshow(name, nameImage[name].second);
        }
    }
    
    void show(std::string name, cv::Mat& image0) {
        cv::namedWindow(name);
        cv::imshow(name, image0);
    }
    
    void show(std::string name, cv::Mat& image0, cv::Mat& image1) {
        cv::Mat image0Copy(image0);
        cv::Mat image1Copy(image1);
        nameImage[name] = std::make_pair(image0Copy, image1Copy);
        cv::namedWindow(name);
        
        cv::setMouseCallback(name, onMouseCallback, (void*)name.c_str());
        cv::imshow(name, image0);
    }
    
    void waitKey() {
        cv::waitKey(0);
    }
}

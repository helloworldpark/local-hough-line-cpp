//
//  Visualizer.cpp
//  local-hough-line-cpp
//
//  Created by 박성현 on 02/08/2019.
//  Copyright © 2019 Sean Park. All rights reserved.
//

#include "Visualizer.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <unordered_map>
#include <vector>
#include <iostream>

namespace fh {
    static std::unordered_map<std::string, std::pair<cv::Mat, cv::Mat>> nameImage;
    
    static void onMouseCallback(int event, int x, int y, int flags, void* userdata) {
        std::string name((char *)userdata);
        if (event == cv::EVENT_LBUTTONUP) {
            cv::imshow(name, (nameImage[name].first));
        } else if (event == cv::EVENT_LBUTTONDOWN) {
            cv::imshow(name, (nameImage[name].second));
        }
    }
    
    void show(std::string name, cv::Mat& image0) {
        cv::namedWindow(name);
        cv::imshow(name, image0);
    }
    
    void show(std::string name, cv::Mat& image0, cv::Mat& image1) {
        image0.addref();
        image1.addref();
        nameImage[name] = std::make_pair(image0, image1);
        cv::namedWindow(name);
        
        char* nameChar = new char[name.length() + 1];
        std::copy(name.c_str(), name.c_str() + strlen(name.c_str()), nameChar);
        cv::setMouseCallback(name, onMouseCallback, (void*)nameChar);
        cv::imshow(name, image0);
    }
    
    void close(std::string name) {
        cv::destroyWindow(name);
        
        std::pair<cv::Mat, cv::Mat> imgs = nameImage[name];
        imgs.first.release();
        imgs.second.release();
        nameImage.erase(name);
    }
    
    void waitKey() {
        cv::waitKey(0);
        for (auto keyValue: nameImage) {
            close(keyValue.first);
        }
    }
    
    // https://docs.opencv.org/4.1.1/d4/da8/group__imgcodecs.html#gabbc7ef1aa2edfaa87772f1202d67e0ce
    bool save(std::string& name, cv::Mat& image, cv::Size size) {
        cv::Mat toUse;
        if (size.height == 0 || size.width == 0) {
            toUse = image;
        } else {
            cv::resize(image, toUse, size);
        }
        
        std::vector<int> compressionParams;
        compressionParams.push_back(cv::IMWRITE_PNG_COMPRESSION);
        compressionParams.push_back(9);
        
        try {
            cv::imwrite(name, toUse);
        } catch (const cv::Exception& e) {
            std::cout << "[Visualizer] Failed to save to " << name << ": " << e.what() << std::endl;
            return false;
        }
        
        std::cout << "[Visualizer] Saved " << name << std::endl;
        return true;
    }
}

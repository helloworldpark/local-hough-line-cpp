//
//  Helper.cpp
//  faster-hough-line-cpp
//
//  Created by 박성현 on 31/07/2019.
//  Copyright © 2019 Sean Park. All rights reserved.
//

#include "Helper.hpp"
#include <opencv2/imgproc.hpp>

namespace fh {
    cv::Size getProcessingSize(cv::Mat& image, int minLength) {
        double h = (double)image.rows;
        double w = (double)image.cols;
        
        if (h > w) {
            if (w > minLength) {
                h = (h / w) * minLength;
                w = minLength;
            }
        } else if (h < w) {
            if (h > minLength) {
                w = (w / h) * minLength;
                h = minLength;
            }
        } else {
            if (w > minLength) {
                h = minLength;
                w = minLength;
            }
        }
        return cv::Size((int)w, (int)h);
    }
    
    void releaseImage(cv::Mat** image) {
        if (image && *image) {
            (*image)->release();
            *image = nullptr;
        }
    }
    
    // https://docs.opencv.org/4.1.0/d5/df9/samples_2cpp_2tutorial_code_2ImgTrans_2houghlines_8cpp-example.html#a8
    void drawHoughLine(cv::Mat& image, cv::Vec3f& line) {
        float rho = line[0], theta = line[1];
        cv::Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cvRound(x0 + 1000*(-b));
        pt1.y = cvRound(y0 + 1000*(a));
        pt2.x = cvRound(x0 - 1000*(-b));
        pt2.y = cvRound(y0 - 1000*(a));
        cv::line(image, pt1, pt2, cv::Scalar(0xff, 0, 0), 1, cv::LINE_AA);
    }
}

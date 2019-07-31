//
//  Helper.cpp
//  faster-hough-line-cpp
//
//  Created by 박성현 on 31/07/2019.
//  Copyright © 2019 Sean Park. All rights reserved.
//

#include "Helper.hpp"

namespace fh {
    cv::Vec2i getProcessingSize(cv::Mat& image, int minLength) {
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
        return cv::Vec2i((int)h, (int)w);
    }
    
}

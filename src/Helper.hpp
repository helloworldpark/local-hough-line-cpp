//
//  Helper.hpp
//  faster-hough-line-cpp
//
//  Created by 박성현 on 31/07/2019.
//  Copyright © 2019 Sean Park. All rights reserved.
//

#ifndef Helper_hpp
#define Helper_hpp

#include <stdio.h>
#include <opencv2/core.hpp>

namespace fh {
    cv::Size getProcessingSize(cv::Mat& image, int minLength);
    
    void releaseImage(cv::Mat** image);
    void drawHoughLine(cv::Mat& image, cv::Vec3f& line);
}


#endif /* Helper_hpp */

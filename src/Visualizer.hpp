//
//  Visualizer.hpp
//  local-hough-line-cpp
//
//  Created by 박성현 on 02/08/2019.
//  Copyright © 2019 Sean Park. All rights reserved.
//

#ifndef Visualizer_hpp
#define Visualizer_hpp

#include <string>
#include <opencv2/core.hpp>

namespace fh {
    void show(std::string name, cv::Mat& image0);
    void show(std::string name, cv::Mat& image0, cv::Mat& image1);
    void waitKey();
}

#endif /* Visualizer_hpp */

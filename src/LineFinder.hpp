//
//  FasterHough.hpp
//  faster-hough-line-cpp
//
//  Created by 박성현 on 31/07/2019.
//  Copyright © 2019 Sean Park. All rights reserved.
//

#ifndef FasterHough_hpp
#define FasterHough_hpp

#include <stdio.h>
#include <string>
#include <opencv2/core.hpp>

namespace fh {
    class LineParams {
    public:
        int worksheetLength = 300;
        
        int bilateralColorS = 160;
        int bilateralSpaceS = 5;
        
        int cannyAperture = 7;
        int cannyThreshold1 = 100;
        int cannyThreshold2 = 200;
        bool cannyUseL2Gradient = true;
        
        int houghResolutionTheta = 360;
        int houghResolutionRho = 1;
        
        int minCountKDE = 10;
        int thetaTolerance = 12;
        bool verbose = false;
        
        inline int houghThreshold() {
            return int(worksheetLength / 3);
        }
    };
    
    
    class LineFinder {
        cv::Mat* _worksheet = nullptr;
        cv::Mat* _result = nullptr;
        
        LineParams params;
        
        void preprocess(cv::Mat* rawImage);
        static cv::Vec3d convertFriendly(cv::Vec3d& line);
        
    public:
        LineFinder(cv::Mat* rawImage, LineParams params = LineParams());
        ~LineFinder();
        
        cv::Mat& runStandardHough();
        cv::Mat& runFasterHough();
    };
}

#endif /* FasterHough_hpp */

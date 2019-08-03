//
//  FasterHough.hpp
//  faster-hough-line-cpp
//
//  Created by 박성현 on 31/07/2019.
//  Copyright © 2019 Sean Park. All rights reserved.
//

#ifndef FasterHough_hpp
#define FasterHough_hpp

#include <string>
#include <vector>
#include <opencv2/core.hpp>

namespace fh {
    typedef cv::Vec3f Line; // rho, theta, votes
    typedef cv::Vec3f Angle; // theta, cos, sin
    
    class LineParams {
    public:
        int worksheetLength = 300;
        
        int bilateralColorS = 200;
        int bilateralSpaceS = 7;
        
        int cannyAperture = 5;
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
        
        inline int houghLocalThreshold() {
            return int(worksheetLength / 4);
        }
    };
    
    
    class LineFinder {
        cv::Mat* _worksheet = nullptr;
        cv::Mat* _result = nullptr;
        
        LineParams params;
        double _diagonalLength = 0.0;
        double _diagonalAngle = 0.0;
        
        void preprocess(cv::Mat* rawImage);
        void prepareCosSin(std::vector<Angle>& table);
        static bool isFindingMeaningful(cv::Size& imageSize, float rho, cv::Vec3f& theta, float diagonalAngle);
        static bool didFindLine(cv::Mat* image, float rho, cv::Vec3f& theta, cv::Vec3f& line, int& threshold);
        static bool isLine(cv::Mat* image, cv::Point& p);
        static cv::Vec3f convertFriendly(cv::Vec3f& line);
        
        inline double diagonalAngle() { return _diagonalAngle; }
        inline double diagonalLength() { return _diagonalLength; }
        
    public:
        LineFinder(cv::Mat* rawImage, LineParams params = LineParams());
        ~LineFinder();
        
        cv::Mat& runStandardHough();
        cv::Mat& runStandardLocalHough();
        cv::Mat& runNaiveLocalHough();
        cv::Mat& preprocessedImage();
    };
}

#endif /* FasterHough_hpp */

//
//  Candidate.hpp
//  faster-hough-line-cpp
//
//  Created by 박성현 on 31/07/2019.
//  Copyright © 2019 Sean Park. All rights reserved.
//

#ifndef Candidate_hpp
#define Candidate_hpp

#include <stdio.h>

namespace fh {
    class Candidate {
        double _theta;
        double _votes;
    public:
        Candidate(double theta, double votes): _theta(theta), _votes(votes) {}
        
        inline double theta() { return _theta; }
        inline double votes() { return _votes; }
        
        static double thetaToUse(double x) {
            if (x < -45.0) {
                return x + 90.0;
            }
            if (x > 45.0) {
                return x - 90;
            }
            return x;
        }
    };
}

#endif /* Candidate_hpp */

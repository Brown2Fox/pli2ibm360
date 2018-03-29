//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_BCR_HPP
#define PROJECT_BCR_HPP

#include "RR.hpp"

class BCR: public RR {
public:
    BCR(): RR(0, '\x07', "BCR  ") {};
};

#endif //PROJECT_BCR_HPP

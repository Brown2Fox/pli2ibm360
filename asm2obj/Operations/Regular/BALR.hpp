//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_BALR_HPP
#define PROJECT_BALR_HPP

#include "RR.hpp"

class BALR: public RR
{
public:
    BALR(): RR(0, '\x05', "BALR ") {};
};

#endif //PROJECT_BALR_HPP

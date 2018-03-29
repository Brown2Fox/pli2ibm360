//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_L_HPP
#define PROJECT_L_HPP

#include "RX.hpp"

class L: public RX {
public:
    L(): RX(0, '\x58', "L     ") {};
};

#endif //PROJECT_L_HPP

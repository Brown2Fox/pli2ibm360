//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_A_HPP
#define PROJECT_A_HPP

#include "RX.hpp"

class A: public RX {
public:
    A(): RX(0, '\x5A', "A     ") {};
};

#endif //PROJECT_A_HPP

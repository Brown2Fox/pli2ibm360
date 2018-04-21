//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_ST_HPP
#define PROJECT_ST_HPP

#include "RX.hpp"

class ST: public RX {
public:
    ST(): RX(0, '\x50', "ST   ") {};
};

#endif //PROJECT_ST_HPP

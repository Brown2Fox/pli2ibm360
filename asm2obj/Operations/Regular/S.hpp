//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_S_HPP
#define PROJECT_S_HPP

#include "RX.hpp"

class S: public RX {
public:
    S(): RX(0, '\x5B', "S     ") {};
};

#endif //PROJECT_S_HPP

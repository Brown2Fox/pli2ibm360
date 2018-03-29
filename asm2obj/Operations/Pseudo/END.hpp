//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_ENDD_HPP
#define PROJECT_ENDD_HPP

#include "Operation.hpp"

class END: public Operation
{

public ctors:
    END(): Operation(1, '\x00', "END  ") {};

public methods:
    int process1(Params& p) override
    {
        printf("END.\n");
        return -1;
    }
    int process2(Params& p) override
    {
        printf("END.\n");
        p.cards.push_back( std::shared_ptr<Card>(new END_CARD({})) );
        return -1;
    }
};


#endif //PROJECT_ENDD_HPP

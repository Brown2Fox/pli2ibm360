//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_ENDD_HPP
#define PROJECT_ENDD_HPP

#include "Operation.hpp"

class END: public Operation
{

public:
    END(): Operation(1, '\x00', "END  ") {};

public:

    int process1(const Params& p) override
    {
        printf("END.\n\n");
        return -1;
    }

    int process2(const Params& p) override
    {
        printf("END.\n\n");
        p.cards.push_back( std::shared_ptr<Card>(new END_CARD({})) );
        return -1;
    }

    ~END() override = default;
};


#endif //PROJECT_ENDD_HPP

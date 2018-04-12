//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_CARD_HPP
#define PROJECT_CARD_HPP


#include <cstdint>
#include <string>


class Card
{

public:
    virtual uint8_t* getBuffer() = 0;
    virtual std::string getFormatOutput() = 0;

    virtual ~Card() = default;
};



#endif //PROJECT_CARD_HPP

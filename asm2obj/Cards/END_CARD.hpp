//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_END_CARD_HPP
#define PROJECT_END_CARD_HPP


#include <cstdint>
#include <cstring>

#include "Card.hpp"




class END_CARD: public Card
{

    struct END_T
    {
        uint8_t PADDING1 = 0x02;
        uint8_t CARD_TYPE[3] = {'E','N','D'};
        uint8_t PADDING2[68] = {0x40};
        uint8_t ID_FIELD[8]  = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
    } card;

public:

    END_CARD(uint8_t id_field[8])
    {
        printf("%s\n\n", getFormatOutput().c_str());
    }


    std::string getFormatOutput() override
    {
        const char* fmt = "<%.3s: id_field=%.8s>";
        return format(fmt, card.CARD_TYPE, card.ID_FIELD);
    }

    uint8_t* getBuffer() override {
        return (uint8_t*)&card;
    }

};

#endif // #define PROJECT_END_CARD_HPP

//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_END_CARD_HPP
#define PROJECT_END_CARD_HPP


#include <cstdint>
#include <cstring>

#include "Card.hpp"

struct END_T /*структ.буфера карты end_map */
{
    uint8_t PADDING1; /*место для кода 0x02     */
    uint8_t CARD_TYPE[3]; /*поле типа об'ектн.карты */
    uint8_t PADDING2[68]; /*пробелы                 */
    uint8_t ID_FIELD[8]; /*идентификационное поле  */
};


class END_CARD: public Card
{
    union {
        END_T structure;
        uint8_t buffer[80];
    } card;

public:

    END_CARD(uint8_t id_field[8])
    {

        card.structure.PADDING1 = 0x02;
        memcpy(card.structure.CARD_TYPE, "END", 3);
        memset(card.structure.PADDING2, 0x40, 68);
        //memcpy(card.structure.ID_FIELD, id_field, 8);

        printf(" --- end card: \n\n");
    }

    uint8_t* getBuffer() override {
        return card.buffer;
    }

};

/*..........................................................................*/
//void GEN_END_CARD()
//{
//    memcpy(END_CARD.structure.ID_FIELD, ESD_CARD.structure.ID_FIELD, 8);
//    memcpy(obj_text[it_card], END_CARD.buffer, 80);
//
//    printf("end card: %.80s\n", END_CARD.buffer);
//
//    it_card += 1;
//}


#endif //PROJECT_END_CARD_HPP

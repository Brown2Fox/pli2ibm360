//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_RLD_HPP
#define PROJECT_RLD_HPP

#include <cstdint>
#include <cstring>
#include <cstdio>
#include "Card.hpp"

struct RLD_T
{
    uint8_t PADDING1; /*место для кода 0x02        */
    uint8_t CARD_TYPE[3]; /*поле типа об'ектн.карты     */
    uint8_t PADDING2; /*пробел                     */
    uint8_t ID_NUM[2]; /*внутр.ид-р имени прогр.    */
    uint8_t PADDING3[3]; /*пробелы                 */
    uint8_t SIGN[2]; /*знак операции               */
    uint8_t PADDING4[4]; /*пробелы                 */
    uint8_t OFFSET_ADDR[3]; /*адрес                   */
    uint8_t PADDING5[53];
    uint8_t ID_FIELD[8]; /*идентификационное поле */
};

class RLD_CARD: public Card
{
    union {
        RLD_T structure;
        uint8_t buffer[80];
    } card;

public:

    RLD_CARD(uint32_t offset_addr, uint8_t id_field[8])
    {
        card.structure.PADDING1 = 0x02;
        memcpy(card.structure.CARD_TYPE, "RLD", 3);
        card.structure.PADDING2 = 0x40;
        card.structure.ID_NUM[0] = 0x00;
        card.structure.ID_NUM[1] = 0x01;
        memset(card.structure.PADDING3, 0x40, 3);
        memcpy(card.structure.SIGN, "00", 2);
        memset(card.structure.PADDING4, 0x40, 4);
        memset(card.structure.OFFSET_ADDR, 0x40, 3);
        memset(card.structure.PADDING5, 0x40, 53);
        memset(card.structure.ID_FIELD, 0x40, 8);

        //===

        // FIXME: black magic in RLD_CARD.OFFSET_ADDR
//        card.structure.OFFSET_ADDR[0] = (uint8_t) '\x00';
//        card.structure.OFFSET_ADDR[1] = (uint8_t) *(&offset_addr + 1);
//        card.structure.OFFSET_ADDR[2] = (uint8_t) *(&offset_addr);

        card.structure.OFFSET_ADDR[0] = 0;
        card.structure.OFFSET_ADDR[1] = 0;
        card.structure.OFFSET_ADDR[2] = (uint8_t) offset_addr;

        card.structure.SIGN[0] = 0x0;
        card.structure.SIGN[1] = 0xC;

        //memcpy(card.structure.ID_FIELD, id_field, 8);

        printf(" --- rld card: offset_addr=%i\n\n", offset_addr);
    }

    uint8_t* getBuffer() override {
        return card.buffer;
    }
};


#endif //PROJECT_RLD_HPP

//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_TXT_HPP
#define PROJECT_TXT_HPP

#include <cstdint>
#include <cstring>
#include <cstdio>
#include "Card.hpp"

struct TXT_T /*структ.буфера карты txt_map */
{
    uint8_t PADDING1; /*место для кода 0x02     */
    uint8_t CARD_TYPE[3]; /*поле типа об'ектн.карты */
    uint8_t PADDING2; /*пробел                  */
    uint8_t OP_ADDR[3]; /*относит.адрес опреации  */
    uint8_t PADDING3[2]; /*пробелы                 */
    uint8_t OP_LENGTH[2]; /*длина операции          */
    uint8_t PADDING4[2]; /*пробелы                 */
    uint8_t ID_NUM[2]; /*внутренний идент.прогр. */
    uint8_t OP_BODY[56]; /*тело операции           */
    uint8_t ID_FIELD[8]; /*идентификационное поле  */
};


class TXT_CARD: public Card
{

    union {
        TXT_T structure;
        uint8_t buffer[80];
    } card;

public:

    TXT_CARD(uint8_t op_length, uint32_t op_addr, uint8_t* op_body, uint8_t id_field[8])
    {
        card.structure.PADDING1 = 0x02;
        memcpy(card.structure.CARD_TYPE, "TXT", 3);
        card.structure.PADDING2 = 0x40;
        memset(card.structure.OP_ADDR, 0x00, 3);
        memset(card.structure.PADDING3, 0x40, 2);
        memset(card.structure.OP_LENGTH, 0X00, 2);
        memset(card.structure.PADDING4, 0x40, 2);
        card.structure.ID_NUM[0] = 0x00;
        card.structure.ID_NUM[1] = 0x01;
        memset(card.structure.OP_BODY, 0x40, 56);
        memset(card.structure.ID_FIELD, 0x40, 8);

        // ===

        // FIXME: black magic in TXT_CARD.OP_ADDR
//        card.structure.OP_ADDR[0] = (uint8_t) '\x00';
//        card.structure.OP_ADDR[1] = (uint8_t) *(&op_addr + 1);
//        card.structure.OP_ADDR[2] = (uint8_t) *(&op_addr);

        card.structure.OP_ADDR[0] = 0;
        card.structure.OP_ADDR[1] = 0;
        card.structure.OP_ADDR[2] = (uint8_t) op_addr;

        card.structure.OP_LENGTH[1] = op_length;

        if (op_length == 2)
        {
            memset(card.structure.OP_BODY, 0x40, 4);
        }

        memcpy(card.structure.OP_BODY, op_body, op_length);
//        memcpy(card.structure.ID_FIELD, id_field, 8);

        printf(" --- txt card: op_len=%i, op_addr=%i, op_body=%s, id_field=%.8s\n\n", op_length, op_addr, op_body, card.structure.ID_FIELD);

    }

    uint8_t* getBuffer() override {
        return card.buffer;
    }

};



//void GEN_TXT_CARD(uint8_t op_length, uint32 op_addr, uint8_t* op_body)
//{
//    // FIXME: black magic in TXT_CARD.OP_ADDR
//    TXT_CARD.structure.OP_ADDR[0] = (uint8_t) '\x00';
//    TXT_CARD.structure.OP_ADDR[1] = (uint8_t) *(&op_addr + 1);
//    TXT_CARD.structure.OP_ADDR[2] = (uint8_t) *(&op_addr);
//
//    TXT_CARD.structure.OP_LENGTH[1] = op_length;
//
//    if (op_length == 2)
//    {
//        memset(TXT_CARD.structure.OP_BODY, 64, 4);
//    }
//
//    memcpy(TXT_CARD.structure.OP_BODY, op_body, op_length);
//    memcpy(TXT_CARD.structure.ID_FIELD, ESD_CARD.structure.ID_FIELD, 8);
//
//    memcpy(obj_text[it_card], TXT_CARD.buffer, 80);
//
//    printf("txt card: op_len=%i, op_addr=%i, op_body=%s, id_field=%.8s\n", op_length, op_addr, op_body, TXT_CARD.structure.ID_FIELD);
//
//    it_card += 1;
//}



#endif //PROJECT_TXT_HPP

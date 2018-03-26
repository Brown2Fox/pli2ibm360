//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_ESD_CARD_HPP
#define PROJECT_ESD_CARD_HPP

#include <cstdint>
#include <vector>
#include <cstring>
#include <cstdio>
#include "Card.hpp"

struct ESD_T /*структ.буфера карты ESD */
{
    uint8_t PADDING1; /*место для кода 0x02     */
    uint8_t CARD_TYPE[3]; /*поле типа об'ектн.карты */
    uint8_t PADDING2[10]; /*пробелы               */
    uint8_t ID_NUM[2]; /*внутр.ид-р имени прогр. */
    uint8_t SYM_NAME[8]; /*имя программы           */
    uint8_t SYM_TYPE; /*код типа ESD-имени      */
    uint8_t R_ADDR[3]; /*относит.адрес программы */
    uint8_t PADDING3; /*пробелы                 */
    uint8_t PR_LENGTH[3]; /*длина программы         */
    uint8_t PADDING4[40]; /*пробелы                 */
    uint8_t ID_FIELD[8]; /*идентификационное поле  */
};


class ESD_CARD: public Card
{
    union {
        ESD_T structure;
        uint8_t buffer[80];
    } card;

public:
    ESD_CARD(uint32_t pr_length, uint32_t r_addr, char* sym_name)
    {
        card.structure.PADDING1 = 0x02;
        memcpy(card.structure.CARD_TYPE, "ESD", 3);
        memset(card.structure.PADDING2, 0x40, 10);
        card.structure.ID_NUM[0] = 0x00;
        card.structure.ID_NUM[1] = 0x01;
        memset(card.structure.SYM_NAME, 0x40, 8);
        card.structure.SYM_TYPE = 0x00;
        memset(card.structure.R_ADDR, 0x00, 3);
        card.structure.PADDING3 = 0x40;
        memset(card.structure.PR_LENGTH, 0x00, 3);
        memset(card.structure.PADDING4, 0x40, 40);
        memset(card.structure.ID_FIELD, 0x40, 8);

        //===

        // FIXME: magic
        //swab((char*)&pr_length, (char*) &pr_length, 2);
//        card.structure.PR_LENGTH[0] = (uint8_t) 0;
//        card.structure.PR_LENGTH[1] = (uint8_t) *(&pr_length);
//        card.structure.PR_LENGTH[2] = (uint8_t) *(&pr_length + 1);

        card.structure.PR_LENGTH[0] = 0;
        card.structure.PR_LENGTH[1] = 0;
        card.structure.PR_LENGTH[2] = (uint8_t)pr_length;

//        card.structure.R_ADDR[2] = r_addr;

        // FIXME: black magic in card.R_ADDR
//        card.structure.R_ADDR[0] = (uint8_t) '\x00';
//        card.structure.R_ADDR[1] = (uint8_t) *(&r_addr + 1);
//        card.structure.R_ADDR[2] = (uint8_t) *(&r_addr);

        card.structure.R_ADDR[0] = 0;
        card.structure.R_ADDR[1] = 0;
        card.structure.R_ADDR[2] = (uint8_t)r_addr;

        memcpy(card.structure.SYM_NAME, sym_name, strlen(sym_name));
        memcpy(card.structure.ID_FIELD, sym_name, strlen(sym_name));

        printf(" --- esd card: pr_len=%i, r_addr=%i, sym_name=%s, id_field=%.8s\n\n", pr_length, r_addr,sym_name, sym_name);
    }

    uint8_t* getBuffer() override {
        return card.buffer;
    }

};


#endif //PROJECT_ESD_CARD_HPP

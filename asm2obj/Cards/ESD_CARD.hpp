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




class ESD_CARD: public Card
{

    struct ESD_T /*структ.буфера карты ESD */
    {
        uint8_t PADDING1     = 0x40; /*место для кода 0x02     */
        uint8_t CARD_TYPE[3] = {'E','S','D'}; /*поле типа об'ектн.карты */
        uint8_t PADDING2[10] = {0x40}; /*пробелы               */
        uint8_t ID_NUM[2]    = {0x00, 0x00}; /*внутр.ид-р имени прогр. */
        uint8_t SYM_NAME[8]  = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};/*имя программы           */
        uint8_t ESD_TYPE     = 0; /*код типа ESD-имени      */
        uint8_t R_ADDR[3]    = {0x00, 0x00, 0x00}; /*относит.адрес программы */
        uint8_t PADDING3     = {0x40}; /*пробелы                 */
        uint8_t PR_LENGTH[3] = {0x00, 0x00, 0x00}; /*длина программы         */
        uint8_t PADDING4[40] = {0x40}; /*пробелы                 */
        uint8_t ID_FIELD[8]  = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}; /*идентификационное поле  */
    } card;

public:
    static uint8_t ID_NUM;

public:
    ESD_CARD(uint32_t pr_length, uint32_t r_addr, char* sym_name, uint8_t esd_type)
    {
        // FIXME: magic
        //swab((char*)&pr_length, (char*) &pr_length, 2);

        card.ESD_TYPE = esd_type;

        if (esd_type == 0)
        {
            card.PR_LENGTH[0] = static_cast<uint8_t>(pr_length >> 16);
            card.PR_LENGTH[1] = static_cast<uint8_t>(pr_length >> 8);
            card.PR_LENGTH[2] = static_cast<uint8_t>(pr_length);

            card.R_ADDR[0] = static_cast<uint8_t>(r_addr >> 16);
            card.R_ADDR[1] = static_cast<uint8_t>(r_addr >> 8);
            card.R_ADDR[2] = static_cast<uint8_t>(r_addr);
        }
        else if (esd_type == 2)
        {
            card.PR_LENGTH[0] = 0;
            card.PR_LENGTH[1] = 0;
            card.PR_LENGTH[2] = 0;

            card.R_ADDR[0] = 0;
            card.R_ADDR[1] = 0;
            card.R_ADDR[2] = 0;
        }



        memcpy(card.SYM_NAME, sym_name, 8);
        memcpy(card.ID_FIELD, sym_name, 8);


        card.ID_NUM[0] = static_cast<uint8_t>(ID_NUM >> 8);
        card.ID_NUM[1] = static_cast<uint8_t>(ID_NUM);
        ID_NUM++;



        printf("%s\n\n", getFormatOutput().c_str());
    }


    std::string getFormatOutput() override
    {
        const char *fmt = "<%.3s: id_num=%i, sym_name=%.8s, esd_type=%i, r_addr=%i, pr_len=%i, id_field=%.8s>";
        return format(fmt,
                card.CARD_TYPE,
                card.ID_NUM[1],
                card.SYM_NAME,
                card.ESD_TYPE,
                card.R_ADDR[2],
                card.PR_LENGTH[2],
                card.ID_FIELD);
    }

    uint8_t* getBuffer() override {
        return (uint8_t*)&card;
    }
};

uint8_t ESD_CARD::ID_NUM = 0;

#endif //PROJECT_ESD_CARD_HPP

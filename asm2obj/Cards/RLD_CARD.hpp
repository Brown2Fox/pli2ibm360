//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_RLD_HPP
#define PROJECT_RLD_HPP

#include <cstdint>
#include <cstring>
#include <cstdio>
#include "Card.hpp"





class RLD_CARD: public Card
{

    struct RLD_T
    {
        uint8_t PADDING1       = 0x02; /*место для кода 0x02        */
        uint8_t CARD_TYPE[3]   = {'R','L','D'}; /*поле типа об'ектн.карты     */
        uint8_t PADDING2       = 0x40; /*пробел                     */
        uint8_t ID_NUM[2]      = {0x00,0x00};/*внутр.ид-р имени прогр.    */
        uint8_t PADDING3[3]    = {0x40, 0x40, 0x40};/*пробелы                 */
        uint8_t SIGN[2]        = {0x0, 0xC};/*знак операции               */
        uint8_t PADDING4[4]    = {0x40, 0x40, 0x40, 0x40};/*пробелы                 */
        uint8_t OFFSET_ADDR[3] = {0x00, 0x00, 0x00}; /*адрес                   */
        uint8_t PADDING5[53]   = {0x40};
        uint8_t ID_FIELD[8]    = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}; /*идентификационное поле */
    } card;


public:
    static uint8_t ID_NUM;

    RLD_CARD(uint32_t offset_addr, uint8_t id_field[8])
    {
        card.OFFSET_ADDR[0] = static_cast<uint8_t>(offset_addr >> 16);
        card.OFFSET_ADDR[1] = static_cast<uint8_t>(offset_addr >> 8);
        card.OFFSET_ADDR[2] = static_cast<uint8_t>(offset_addr);

        card.SIGN[0] = 0x0;
        card.SIGN[1] = 0xC;

        card.ID_NUM[0] = static_cast<uint8_t>(ID_NUM >> 8);
        card.ID_NUM[1] = static_cast<uint8_t>(ID_NUM);
        ID_NUM++;

        memcpy(card.ID_FIELD, id_field, 8);

        printf("%s\n\n", getFormatOutput().c_str());
    }


    std::string getFormatOutput() override
    {
        const char *fmt = "<%.3s: id_num=%i, sign=%i, offset_addr=%i, id_field=%.8s>";
        return format(fmt,
                      card.CARD_TYPE,
                      card.ID_NUM[0]*0x100 + card.ID_NUM[1],
                      card.SIGN[1],
                      card.OFFSET_ADDR[0]*0x10000 + card.OFFSET_ADDR[1]*0x100 + card.OFFSET_ADDR[2],
                      card.ID_FIELD);
    }

    uint8_t* getBuffer() override {
        return (uint8_t*)&card;
    }
};

uint8_t RLD_CARD::ID_NUM = 1;

#endif //PROJECT_RLD_HPP

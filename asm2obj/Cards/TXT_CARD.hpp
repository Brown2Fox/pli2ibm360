#ifndef PROJECT_TXT_HPP
#define PROJECT_TXT_HPP

#include <cstdint>
#include <cstring>
#include <cstdio>
#include "Card.hpp"



class TXT_CARD: public Card
{

    struct TXT_T /*структ.буфера карты txt_map */
    {
        uint8_t PADDING1     = 0x02; /*место для кода 0x02     */
        uint8_t CARD_TYPE[3] = {'T','X','T'}; /*поле типа об'ектн.карты */
        uint8_t PADDING2     = 0x40; /*пробел                  */
        uint8_t OP_ADDR[3]   = {0x00, 0x00, 0x00}; /*относит.адрес опреации  */
        uint8_t PADDING3[2]  = {0x40, 0x40}; /*пробелы                 */
        uint8_t OP_LENGTH[2] = {0x00, 0x00}; /*длина операции          */
        uint8_t PADDING4[2]  = {0x40, 0x40}; /*пробелы                 */
        uint8_t ID_NUM[2]    = {0x00, 0x00}; /*внутренний идент.прогр. */
        uint8_t OP_BODY[56]  = {0x00}; /*тело операции           */
        uint8_t ID_FIELD[8]  = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}; /*идентификационное поле  */
    } card;

public:

    static uint16_t ID_NUM;

public:

    TXT_CARD(uint16_t op_length, uint32_t op_addr, uint8_t* op_body, uint8_t id_field[8])
    {
        card.OP_ADDR[0] = static_cast<uint8_t>(op_addr >> 16); // OP_ADDR[2]*0x10000
        card.OP_ADDR[1] = static_cast<uint8_t>(op_addr >> 8);  // OP_ADDR[1]*0x100
        card.OP_ADDR[2] = static_cast<uint8_t>(op_addr);       // OP_ADDR[0]

        card.OP_LENGTH[0] = static_cast<uint8_t>(op_length >> 8);
        card.OP_LENGTH[1] = static_cast<uint8_t>(op_length);


        card.ID_NUM[0] = static_cast<uint8_t>(ID_NUM >> 8);
        card.ID_NUM[1] = static_cast<uint8_t>(ID_NUM);
        ID_NUM++;

        memcpy(card.OP_BODY, op_body, op_length);
        memcpy(card.ID_FIELD, id_field, 8);

        printf("%s\n\n", getFormatOutput().c_str());

    }

    public: std::string getFormatOutput() override
    {
        const char *fmt = "<%.3s: id_num=%i, op_addr=%i, op_len=%i, op_body=%.5s, id_field=%.8s>";
        return format(fmt,
                      card.CARD_TYPE,
                      card.ID_NUM[0]*0x100 + card.ID_NUM[1],
                      card.OP_ADDR[0]*0x10000 + card.OP_ADDR[1]*0x100 + card.OP_ADDR[2],
                      card.OP_LENGTH[0]*0x100 + card.OP_LENGTH[1],
                      card.OP_BODY,
                      card.ID_FIELD);
    }

    uint8_t* getBuffer() override {
        return (uint8_t*)&card;
    }

};


uint16_t TXT_CARD::ID_NUM = 0;

#endif //PROJECT_TXT_HPP

//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_RR_HPP
#define PROJECT_RR_HPP

#include "Operation.hpp"


class RR: public Operation {

    typedef struct { uint8_t OP_CODE; uint8_t R1_R2; } OP_RR;

protected:
    union {
        unsigned char buffer[2];
        OP_RR structure;
    } rr;

protected:
    RR() { op_len = 2; }
    RR(uint8_t op_type, uint8_t op_code, const char* op_name) : Operation(op_type, op_code, op_name) {
        op_len = 2;
    };

public:
    int process1(Params& p) override
    {
        if (p.label_flag == 'Y')
        {
            with(p.symbols.back(), sym)
                sym.length = this->op_len;
                sym.transfer_flag = 'R';
                printf("RR: symbols << val: %i, len: %i, name: %.8s\n", sym.val, sym.length, sym.name);
            end_with;
        }

        return this->op_len;
    }

    int process2(Params& p) override
    {
        uint8_t id_field[8] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
        char* sym_name_tbl;
        char* sym_name_asm_1;
        char* sym_name_asm_2;
        uint8_t R1R2;
        uint8_t R1 = 0;
        uint8_t R2 = 0;

        rr.structure.OP_CODE = this->op_code;

        sym_name_asm_1 = strtok((char*)p.asm_line.structure.operand, ",");
        sym_name_asm_2 = strtok(nullptr, " ");

        if (isalpha((int)*sym_name_asm_1))
        {
            bool found = false;
            for (auto& sym: p.symbols) // iterate over p.symbols
            {
                sym_name_tbl = strtok((char*)sym.name, " ");
                if (strcmp(sym_name_tbl, sym_name_asm_1) == 0)
                {
                    R1 = static_cast<uint8_t>(sym.val);
                    found = true;
                    break;
                }
            }
            assertf(found, "Symbol not found: %s", sym_name_asm_1);
        }
        else /*иначе, берем в качестве */
        { /*перв.операнда машинн.ком*/
            R1 = static_cast<uint8_t>(strtol(sym_name_asm_1, nullptr, 10)); /*значен.выбр.   лексемы  */
        }

        if (std::isalpha((int)*sym_name_asm_2))
        {
            bool found = false;
            for (auto& sym: p.symbols) // iterate over p.symbols
            {
                sym_name_tbl = strtok((char*)sym.name, " ");
                if (strcmp(sym_name_tbl, sym_name_asm_2) == 0)
                {
                    R2 = static_cast<uint8_t>(sym.val);
                    found = true;
                    break;
                }
            }
            assertf(found, "Symbol not found: %s", sym_name_asm_2);
        }
        else /*иначе, берем в качестве */
        { /*втор.операнда машинн.ком*/
            R2 = static_cast<uint8_t>(strtol(sym_name_asm_2, nullptr, 10));/*значен.выбр.   лексемы  */
        }


        rr.structure.R1_R2 = (R1 << 4) + R2;

        printf("RR: oper=%.5s, regnum1=%i, regnum2=%i | op_len=%i\n", this->op_name, R1, R2, op_len);

       p.cards.push_back( std::shared_ptr<Card>(new TXT_CARD(this->op_len, p.addr_counter, rr.buffer, id_field)) );

        return this->op_len;
    }
};

#endif //PROJECT_RR_HPP

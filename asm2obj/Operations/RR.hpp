//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_RR_HPP
#define PROJECT_RR_HPP

#include "Operation.hpp"


class RR: public Operation {

protected fields:
    union {
        unsigned char buffer[2];
        OP_RR structure;
    } rr;

protected ctors:
    RR() { op_len = 2; }
    RR(uint8 op_type, uint8 op_code, const char* op_name) : Operation(op_type, op_code, op_name) {
        op_len = 2;
    };
public methods:
    int process1(Params& p) override
    {
        if (p.sym_flag == 'Y') /*если ранее обнар.метка, */
        {
            with(p.sym_table[p.it_sym], sym)
                sym.length = this->op_len;
                sym.transfer_flag = 'R';
                printf("RR: sym_table << val: %i, len: %i, name: %.8s\n", sym.val, sym.length, sym.name);
            end_with;

        }
//        addr_counter += this->op_len;
        return this->op_len;
    }
    int process2(Params& p) override
    {
        char* sym_name_tbl;
        char* sym_name_asm_1;
        char* sym_name_asm_2;
        unsigned char R1R2;
        uint8_t R1 = 0;
        uint8_t R2 = 0;

        rr.structure.OP_CODE = this->op_code;

        sym_name_asm_1 = strtok((char*)p.asm_line.structure.operand, ",");
        sym_name_asm_2 = strtok(nullptr, " ");

        if (isalpha((int)*sym_name_asm_1))
        {
            bool found = false;
            for (auto& sym: p.sym_table) // iterate over p.sym_table
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
            for (auto& sym: p.sym_table) // iterate over p.sym_table
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

       p.cards.push_back( std::shared_ptr<Card>(new TXT_CARD(this->op_len, p.addr_counter, rr.buffer, {})) );

//        addr_counter += this->op_len;
        return this->op_len;
    }
};

#endif //PROJECT_RR_HPP

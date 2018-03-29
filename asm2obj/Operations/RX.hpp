//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_RX_HPP
#define PROJECT_RX_HPP


#include <cmath>
#include "Operation.hpp"

class RX: public Operation {

protected fields:
    union {
        unsigned char buffer[4];
        OP_RX structure;
    } rx;

protected ctors:
    RX() { op_len = 4; }
    RX(uint8 op_type, uint8 op_code, const char* op_name) :  Operation(op_type, op_code, op_name) {
        op_len = 4;
    };
public methods:
    int process1(Params& p) override
    {
        if (p.label_flag == 'Y') /*если ранее обнар.метка, */
        {
            with(p.symbols.back(), sym)
                sym.length = this->op_len;
                sym.transfer_flag = 'R';
                printf("RX: sym_tablea << val: %i, len: %i, name: %.8s}\n", sym.val, sym.length, sym.name);
            end_with;
        }
//        addr_counter += this->op_len;
        return op_len;
    }
    int process2(Params& p) override
    {
        uint8_t id_field[8] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
        char* sym_name_tbl = nullptr;
        char* sym_name_asm_1 = nullptr;
        char* sym_name_asm_2 = nullptr;
        char* PTR_ = nullptr;
        int B2D2 = 0;

        constexpr uint8_t BITS = 12;
        constexpr uint16_t MAX_DISP = ipow(2, BITS); // 2^12 == 0x1000

        uint8_t RegNum = 0;
        uint8_t BaseRegNum = 0;
        uint32_t Displacement = 0;

        rx.structure.OP_CODE = this->op_code;

        sym_name_asm_1 = strtok((char*)p.asm_line.structure.operand, ",");
        sym_name_asm_2 = strtok(nullptr, " ");

        if (isalpha((int)*sym_name_asm_1))
        {
            bool found = false;
            for (auto& sym: p.symbols) // iterate over p.symbols
            {
                sym_name_tbl = strtok((char*)sym.name, " ");
                if (!strcmp(sym_name_tbl, sym_name_asm_1)) /* и при совпадении:      */
                {
                    // FIXME: magic
//                    R1X2 = p.symbols[j].sym_addr << 4; /*  метки в качестве перв.*/
                    RegNum = (uint8_t) sym.val;
                    found = true;
                    break;
                }
            }
            assertf(found, "Symbol not found: %s", sym_name_asm_1);
        }
        else /*иначе, берем в качестве */
        { /*перв.операнда машинн.ком*/
//            R1X2 = strtol(sym_name_asm_1, nullptr, 10) << 4; /*значен.выбр.   лексемы  */
            RegNum = (uint8_t) strtol(sym_name_asm_1, nullptr, 10);
        }

        if (isalpha((int)*sym_name_asm_2))
        {
            bool found = false;
            for (auto& sym: p.symbols) // iterate over p.symbols
            {
                sym_name_tbl = strtok((char*)sym.name, " ");
                if (!strcmp(sym_name_tbl, sym_name_asm_2))
                {
                    Displacement = MAX_DISP;

                    with(p.baseregs, regs)

                        for (uint8_t i = 0; i < regs.size(); i++) // iterate over baseregs
                        {
                            if (regs[i].activity_flag == 'Y')
                            {
                                auto _disp = sym.val - regs[i].base_addr;
                                if (_disp <= Displacement)
                                {
                                    BaseRegNum = i + uint8_t(1);
                                    Displacement = _disp;
                                }
                            }
                        }

                        assertf(BaseRegNum > 0, "Basereg not found in baseregs table (should be set base address with USING operation)\n");
                        assertf(Displacement < MAX_DISP, "Unreachable label address from base address: base=%i, lbl=%i\n", regs[BaseRegNum-1].base_addr, sym.val);

                    end_with;

                    found = true;
                    break;
                }
            }
            assertf(found, "Symbol not found: %s", sym_name_asm_2);
        }
        else  assertf(false, "Wrong second operand format: %s", sym_name_asm_2);


        rx.structure.R1_X2 = RegNum; /*дозапись перв.операнда  */
        // FIXME: magic
//                    B2D2 = basereg_num;
//                    PTR_ = (char*)&B2D2; /* и в соглашениях ЕС ЭВМ */
//                    swab(PTR_, PTR_, 2); /* с записью в тело ком-ды*/
        rx.structure.B2_D2 = (BaseRegNum << BITS) + Displacement;


        printf("RX: oper=%.5s, regnum=%x, baseregnum=%i(%i), disp=%i | op_len=%i\n",
               this->op_name,
               RegNum,
               BaseRegNum,
               p.baseregs[BaseRegNum-1].base_addr,
               Displacement, op_len);

        p.cards.push_back( std::shared_ptr<Card>( new TXT_CARD(this->op_len, p.addr_counter, rx.buffer, id_field)) );
//        addr_counter += this->op_len;
        return op_len;
    }
};

#endif //PROJECT_RX_HPP

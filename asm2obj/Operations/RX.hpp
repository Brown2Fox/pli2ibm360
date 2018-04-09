//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_RX_HPP
#define PROJECT_RX_HPP


#include <cmath>
#include "Operation.hpp"

class RX: public Operation {

    using OP_RX = struct { uint8_t OP_CODE; uint8_t R1_X2; uint16_t B2_D2; };

protected /*FIELDS*/:

    union {
        uint8_t buffer[4];
        OP_RX structure;
    } rx {};

protected /*CTORS*/:

    RX() { op_len = 4; }
    RX(uint8_t op_type, uint8_t op_code, const char* op_name) :  Operation(op_type, op_code, op_name) {
        op_len = 4;
    };

public /*METHODS*/:

    int process1(const Params& p) override
    {
        if (p.label_flag == 'Y')
        {
            with(p.symbols.back(), sym)
                sym.length = op_len;
                sym.transfer_flag = 'R';
                printf("RX: sym_tablea << val: %i, len: %i, name: %.8s}\n", sym.val, sym.length, sym.name);
            end_with;
        }

        return op_len;
    }

    int process2(const Params& p) override
    {
        uint8_t id_field[8] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
        char* sym_name_tbl = nullptr;
        char* sym_name_asm_1 = nullptr;
        char* sym_name_asm_2 = nullptr;
        char* PTR_ = nullptr;
        int B2D2 = 0;

        constexpr uint8_t BITS = 12;
        constexpr uint16_t MAX_DISP = ipow(2, BITS); // 2^12 == 0x1000

        uint8_t R1 = 0;
        uint8_t X2 = 0;
        uint16_t B2 = 0;
        uint16_t D2 = 0;



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
                    R1 = (uint8_t) sym.val;
                    found = true;
                    break;
                }
            }
            assertf(found, "Symbol not found: %s", sym_name_asm_1);
        }
        else
        {
            // FIXME: RX magic
            R1 = (uint8_t) strtol(sym_name_asm_1, nullptr, 10);
        }

        if (isalpha((int)*sym_name_asm_2))
        {
            bool found = false;
            for (auto& sym: p.symbols) // iterate over p.symbols
            {
                sym_name_tbl = strtok((char*)sym.name, " ");
                if (!strcmp(sym_name_tbl, sym_name_asm_2))
                {
                    D2 = MAX_DISP;

                    with(p.baseregs, regs)

                        for (uint8_t i = 0; i < regs.size(); i++) // iterate over baseregs
                        {
                            if (regs[i].activity_flag == 'Y')
                            {
                                auto _disp = static_cast<uint16_t>(sym.val - regs[i].base_addr);
                                if (_disp <= D2)
                                {
                                    B2 = i + uint8_t(1);
                                    D2 = _disp;
                                }
                            }
                        }

                        assertf(B2 > 0, "Basereg not found in baseregs table (should be set base address with USING operation)\n");
                        assertf(D2 < MAX_DISP, "Unreachable label address from base address: base=%i, lbl=%i\n", regs[B2-1].base_addr, sym.val);

                    end_with;

                    found = true;
                    break;
                }
            }
            assertf(found, "Symbol not found: %s", sym_name_asm_2);
        }
        else  assertf(false, "Wrong second operand format: %s", sym_name_asm_2);



//                    B2D2 = basereg_num;
//                    PTR_ = (char*)&B2D2; /* и в соглашениях ЕС ЭВМ */
//                    swab(PTR_, PTR_, 2); /* с записью в тело ком-ды*/
        // FIXME: RX magic
        rx.structure.OP_CODE = this->op_code;
        rx.structure.R1_X2 = (R1 << 4) + X2;
        rx.structure.B2_D2 = (B2 << 12) + D2;


        printf("RX: oper=%.5s, regnum=%i, baseregnum=%i(%i), disp=%i | op_len=%i\n",
               this->op_name,
               R1,
               B2,
               p.baseregs[B2-1].base_addr,
               D2, op_len);


        // this is for conversion between little-endian and big-endian notations on writing uint16_t variable
        std::swap(rx.buffer[2], rx.buffer[3]);

        for (auto c: rx.buffer)
            printf("%i.", c);
        printf("\n");


        p.cards.push_back( std::shared_ptr<Card>( new TXT_CARD(this->op_len, p.addr_counter, rx.buffer, id_field)) );

        return op_len;
    }

    ~RX() { std::printf("~RX()\n"); }
};

#endif //PROJECT_RX_HPP

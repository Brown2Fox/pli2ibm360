//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_DS_HPP
#define PROJECT_DS_HPP

#include "RX.hpp"

class DS: public RX
{
public ctors:
            DS(): RX(1, '\x00', "DS   ") {};
public methods:
    int process1(Params& p) override
    {
        op_len = 4;

        if (p.addr_counter % 4 != 0)
        {
            p.addr_counter = (p.addr_counter / 4 + 1) * 4; /* установ.p.addr_counter на гр.сл.*/
        }

        if (p.sym_flag == 'Y')
        {
            p.sym_flag = 'N'; // clear flag

            with(p.sym_table[p.it_sym], sym)
                sym.length = 4;
                sym.transfer_flag = 'R';
                sym.val = p.addr_counter;
            end_with;
        }

        with(p.sym_table[p.it_sym], sym)
            printf("DS: sym_table << val: %i, len: %i, name: %.8s\n", sym.val, sym.length, sym.name);
        end_with;

        return op_len;
    }
    int process2(Params& p) override
    {
        rx.structure.OP_CODE = 0;
        rx.structure.R1_X2 = 0;
        rx.structure.B2_D2 = 0;

        if (p.asm_line.structure.operand[0] == 'F' )
        {
            op_len = 4;
        }
        else assertf(false,"Unknown format: %s", p.asm_line.structure.operand);

        printf("DS: r1_x2=%i, b2_d2=%i | op_len=%i\n",
               rx.structure.R1_X2,
               rx.structure.B2_D2, op_len);

        p.cards.push_back( std::shared_ptr<Card>(new TXT_CARD(op_len, p.addr_counter, rx.buffer, {})) ); /*формирование TXT_CARD-карты  */

        return op_len;
    }
};



#endif //PROJECT_DS_HPP

//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_DC_HPP
#define PROJECT_DC_HPP


#include "RX.hpp"





class DC: public RX {

public ctors:
    DC(): RX(1, '\x00', "DC   ") {};

public methods:
    int process1(Params& p) override
    {
        op_len = 4;
        if (p.addr_counter % 4)
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
            printf("DC: sym_table << val: %i, len: %i, name: %.8s\n", sym.val, sym.length, sym.name);
        end_with;
//        p.addr_counter += op_len;
        return op_len;
    }

    int process2(Params& p) override
    {
        char* sym_name_tbl = nullptr;
        char* val_asm = nullptr;
        rx.structure.OP_CODE = 0; /*занулим два старших     */
        rx.structure.R1_X2 = 0; /*байта RX.structure          */
        rx.structure.B2_D2 = 0;
        bool need_rld = false;


        // variants:
        if (std::memcmp(p.asm_line.structure.operand, "F'", 2) == 0)
        {
            val_asm = std::strtok((char*)p.asm_line.structure.operand + 2, "'");
            rx.structure.B2_D2 = std::strtol(val_asm, nullptr, 10); /*перевод ASCII-> int     */
//            swab((char*)&rx.structure.B2_D2,(char*)&rx.structure.B2_D2,2);
            op_len = 4;
        }
        else if (std::memcmp(p.asm_line.structure.operand, "E'", 2) == 0)
        {
            // TODO: add float point magic
            val_asm = std::strtok((char*)p.asm_line.structure.operand + 2, "'");

            op_len = 4;
        }
        else if (std::memcmp(p.asm_line.structure.operand, "A(", 2) == 0)
        {
            need_rld = true;
            val_asm = std::strtok((char*)p.asm_line.structure.operand + 2, ")");

            bool found = false;
            for (auto& sym: p.sym_table) // iterate over p.sym_table
            {
                sym_name_tbl = std::strtok((char*)sym.name, " ");
                if (strcmp(sym_name_tbl, val_asm) == 0)
                {
                    rx.structure.B2_D2 = sym.val;
                    found = true;
                    break;
                }
            }
            assertf(found,"Symbol not found: %s", val_asm);

            op_len = 4;
        }
        else assertf(false,"Unknown format: %s", p.asm_line.structure.operand);

        printf("DC: r1_x2=%i, b2_d2=%i | op_len=%i\n",
               rx.structure.R1_X2,
               rx.structure.B2_D2, op_len);

        p.cards.push_back( std::shared_ptr<Card>(new TXT_CARD(op_len, p.addr_counter, rx.buffer, {})) );
        if (need_rld) p.cards.push_back( std::shared_ptr<Card>( new RLD_CARD(p.addr_counter, {})) );

        return op_len; /*успешн.завершение подпр.*/
    }
};


#endif //PROJECT_DC_HPP

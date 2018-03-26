//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_START_HPP
#define PROJECT_START_HPP

#include "Operation.hpp"

class START: public Operation
{
public ctors:
    START(): Operation(1, '\x00', "START") {};
public methods:
    int process1(Params& p) override
    {
        p.sym_flag = 'N'; // clear flag

        p.addr_counter = static_cast<uint32>(strtol((const char *)p.asm_line.structure.operand, nullptr, 10));

        if (p.addr_counter % 8 != 0)
        {
            p.addr_counter = (p.addr_counter + (8 - p.addr_counter % 8)); /*кратным                 */
        }

        with(p.sym_table[p.it_sym], sym)
            sym.val = p.addr_counter; /* p.addr_counter в поле sym_addr,    */
            sym.length = this->op_len; /* 1 в поле sym_length,        */
            sym.transfer_flag = 'R'; /* 'R' в поле transfer_flag       */
            printf("START: sym_table << val: %i, len: %i, name: %.8s\n", sym.val, sym.length, sym.name);
        end_with;

        return 0;
    }
    int process2(Params& p) override
    {
        char* sym_name_asm = nullptr;
        char* sym_name_tbl = nullptr;
        uint32 program_len = 0; /*                        */

        sym_name_asm = strtok((char*)p.asm_line.structure.label, " ");

        bool found = false;
        for (auto& sym: p.sym_table) // iterate over p.sym_table
        {
            sym_name_tbl = strtok((char*)sym.name, " ");
            if (strcmp(sym_name_tbl, sym_name_asm) == 0)
            {
                program_len = p.addr_counter - sym.val; /*  знач.этой метки, обра-*/
                p.addr_counter = sym.val; /*устанавл.p.addr_counter, равным  */
                found = true;
                break;
            }
        }
        assertf(found,"Symbol not found: %s", sym_name_asm);

        printf("START: \n");

        p.cards.push_back( std::shared_ptr<Card>(new ESD_CARD(program_len, p.addr_counter, sym_name_tbl)) );
        return 0;
    }
};


#endif //PROJECT_START_HPP

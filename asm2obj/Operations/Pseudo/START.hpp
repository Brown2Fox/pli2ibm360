//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_START_HPP
#define PROJECT_START_HPP

#include "Operation.hpp"

class START: public Operation
{
public:

    START(): Operation(1, '\x00', "START") {};

public:

    int process1(const Params& p) override
    {
        p.label_flag = 'N'; // clear flag

        p.addr_counter = static_cast<uint32_t>(strtol((const char *)p.asm_line.structure.operand, nullptr, 10));

        alignAddr(p.addr_counter, 8);

        with(p.symbols.back(), sym)
            sym.val = p.addr_counter;
            sym.length = op_len;
            sym.transfer_flag = 'R';
            printf("START: local symbol << val: %i, len: %i, name: %.8s\n", sym.val, sym.length, sym.name);
        end_with;

        return 0;
    }
    int process2(const Params& p) override
    {
        char* sym_name_asm = nullptr;
        char* sym_name_tbl = nullptr;
        uint32_t program_len = 0;

        sym_name_asm = strtok((char*)p.asm_line.structure.label, " ");

        bool found = false;
        for (auto& sym: p.symbols) // iterate over p.symbols
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

        p.cards.push_back( std::shared_ptr<Card>(new ESD_CARD(program_len, p.addr_counter, sym_name_tbl, 0)) );
        return 0;
    }

    ~START() { std::printf("~START()\n");  }
};


#endif //PROJECT_START_HPP

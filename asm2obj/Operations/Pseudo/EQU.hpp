//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_EQU_HPP
#define PROJECT_EQU_HPP

#include "Operation.hpp"

class EQU: public Operation
{

public:
    EQU(): Operation(1, '\x00', "EQU  ") {};

public:
    int process1(const Params& p) override
    {
        with(p.symbols.back(), sym)

            p.label_flag = 'N';
            sym.length = op_len;

            if (p.asm_line.structure.operand[0] == '*')
            {
                sym.val = p.addr_counter;
                sym.transfer_flag = 'R';
            }
            else
            {
                sym.val = (uint32_t) std::strtol(reinterpret_cast<char *>(p.asm_line.structure.operand), nullptr, 10);
                sym.transfer_flag = 'A';
            }

            printf("EQU: local symbol << val: %i, len: %i, name: %.8s\n", sym.val, sym.length, sym.name);

        end_with;
        return 0;
    }

    int process2(const Params& p) override
    {
        return 0;
    }


    ~EQU() override = default;
};


#endif //PROJECT_EQU_HPP

#ifndef PROJECT_EXTRN_HPP
#define PROJECT_EXTRN_HPP

#include <memory>
#include <ibm360_types.h>
#include "Operation.hpp"

class EXTRN: public Operation
{
public ctors:
    EXTRN(): Operation(1, '\x00', "EXTRN") {};
public methods:
    int process1(Params& p) override
    {

        printf("EXTRN: \n");

        return 0;
    }
    int process2(Params& p) override
    {
        char* sym_name_asm = nullptr;
        uint32 program_len = 0;

        sym_name_asm = strtok((char*)p.asm_line.structure.operand, " ");


        printf("EXTRN: sym=%s\n", sym_name_asm);


        p.cards.push_back( std::shared_ptr<Card>(new ESD_CARD(program_len, p.addr_counter, sym_name_asm, 2)) );
        return 0;
    }
};



#endif //PROJECT_EXTRN_HPP

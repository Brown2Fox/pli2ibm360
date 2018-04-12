#ifndef PROJECT_EXTRN_HPP
#define PROJECT_EXTRN_HPP

#include <memory>
#include "Operation.hpp"

class EXTRN: public Operation
{
public:

    EXTRN(): Operation(1, '\x00', "EXTRN") {};

public:

    int process1(const Params& p) override
    {

        printf("EXTRN: extern symbol << name: %.12s\n", p.asm_line.structure.operand);

        return 0;
    }

    int process2(const Params& p) override
    {
        char* sym_name_asm = nullptr;
        uint32_t program_len = 0;

        sym_name_asm = strtok((char*)p.asm_line.structure.operand, " ");

        printf("EXTRN: sym=%s\n", sym_name_asm);

        p.cards.push_back( std::shared_ptr<Card>(new ESD_CARD(program_len, p.addr_counter, sym_name_asm, 2)) );
        return 0;
    }

    ~EXTRN() override = default;
};



#endif //PROJECT_EXTRN_HPP

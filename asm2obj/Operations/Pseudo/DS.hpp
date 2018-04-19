//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_DS_HPP
#define PROJECT_DS_HPP

#include <Operation.hpp>

class DS: public Operation
{

public:

    DS(): Operation(1, '\x00', "DS   ") {};

public:
    int process1(const  Params& p) override
    {
        alignAddr(p.addr_counter, 4);

        op_len = 4;

        if (p.label_flag == 'Y')
        {
            p.label_flag = 'N'; // clear flag

            with(p.symbols.back(), sym)
                sym.length = 4;
                sym.transfer_flag = 'R';
                sym.val = p.addr_counter;
            end_with;
        }

        with(p.symbols.back(), sym)
            printf("DS: local symbol << val: %i, len: %i, name: %.8s\n", sym.val, sym.length, sym.name);
        end_with;

        return op_len;
    }

    int process2(const Params& p) override
    {
        alignAddr(p.addr_counter, 4);

        uint8_t id_field[8] = {' '};
        uint8_t val_buff[56] = {0x40};

        char* sym_name_tbl = nullptr;
        char* val_asm = nullptr;

        union { int32_t _int; float_t _float; uint32_t _address; } val = {0};
        auto* pVal = (uint8_t*)&val;

        enum OPT { Nothing, Int, Float, Addr };
        int opt = OPT::Nothing;
        if (std::memcmp(p.asm_line.structure.operand, "F", 1) == 0) opt = OPT::Int;
        if (std::memcmp(p.asm_line.structure.operand, "E", 1) == 0) opt = OPT::Float;
        if (std::memcmp(p.asm_line.structure.operand, "A", 1) == 0) opt = OPT::Addr;

        switch (opt)
        {
            case OPT::Int:
            {
                op_len = sizeof(int32_t);

                printf("DS: intVal=%i | len=%i\n", val._int, op_len);
                break;
            }

            case OPT::Float:
            {
                op_len = sizeof(float_t);

                printf("DS: floatVal=%f | len=%i\n", val._float, op_len);
                break;
            }

            case OPT::Addr:
            {
                op_len = sizeof(uint32_t);

                printf("DS: addrVal=%i | len=%i\n", val._address, op_len);
                break;
            }

            case OPT::Nothing:
            default:
            {
                assertf(false, "Unknown format: %s", p.asm_line.structure.operand);
                break;
            }
        }

        for (int i = 0; i < op_len; i++)
        {
            val_buff[i] = 0;
        }

        p.cards.push_back( std::shared_ptr<Card>(new TXT_CARD(op_len, p.addr_counter, val_buff, id_field)) );

        return op_len;
    }

    ~DS() override = default;
};



#endif //PROJECT_DS_HPP

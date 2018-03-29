//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_DC_HPP
#define PROJECT_DC_HPP


#include "RX.hpp"





class DC: public Operation {

public ctors:
    DC(): Operation(1, '\x00', "DC   ") {};

public methods:
    int process1(Params& p) override
    {
        op_len = 4;
        if (p.addr_counter % 4)
        {
            p.addr_counter = (p.addr_counter / 4 + 1) * 4; /* установ.p.addr_counter на гр.сл.*/
        }

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
            printf("DC: symbols << val: %i, len: %i, name: %.8s\n", sym.val, sym.length, sym.name);
        end_with;
//        p.addr_counter += op_len;
        return op_len;
    }

    int process2(Params& p) override
    {
        uint8_t val_buff[56] = {0x40};
        uint8_t id_field[8] = {0x0};

        char* sym_name_tbl = nullptr;
        char* val_asm = nullptr;

        union { int32_t _int; float_t _float; uint32_t _address; } val = {0};
        auto* pVal = (uint8_t*)&val;

        enum OPT { Nothing, Int, Float, Addr };
        int opt = OPT::Nothing;
        if (std::memcmp(p.asm_line.structure.operand, "F'", 2) == 0) opt = OPT::Int;
        if (std::memcmp(p.asm_line.structure.operand, "E'", 2) == 0) opt = OPT::Float;
        if (std::memcmp(p.asm_line.structure.operand, "A(", 2) == 0) opt = OPT::Addr;

        switch (opt)
        {
            case OPT::Int:
            {
                val_asm = std::strtok((char *) p.asm_line.structure.operand + 2, "'");
                val._int = std::strtol(val_asm, nullptr, 10);

                pVal = (uint8_t *) &val._int;
                op_len = sizeof(int32_t);

                printf("DC: intVal=%i | len=%i\n", val._int, op_len);
                break;
            }

            case OPT::Float:
            {
                val_asm = std::strtok((char *) p.asm_line.structure.operand + 2, "'");
                val._float = std::strtof(val_asm, nullptr);

                pVal = (uint8_t *) &val._float;
                op_len = sizeof(float_t);

                printf("DC: floatVal=%f | len=%i\n", val._float, op_len);
                break;
            }

            case OPT::Addr:
            {
                val_asm = std::strtok((char *) p.asm_line.structure.operand + 2, ")");

                for (auto &sym: p.symbols) // iterate over p.symbols
                {
                    sym_name_tbl = std::strtok((char *) sym.name, " ");
                    if (strcmp(sym_name_tbl, val_asm) == 0)
                    {
                        val._address = sym.val;
                        break;
                    }
                }

                pVal = (uint8_t *) &val._address;
                op_len = sizeof(uint32_t);

                memcpy(id_field, val_asm, 8);

                printf("DC: addrVal=%i | len=%i\n", val._address, op_len);
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
            val_buff[i] = *(pVal + i);
        }


        p.cards.push_back( std::shared_ptr<Card>(new TXT_CARD(op_len, p.addr_counter, val_buff, id_field)) );

        if (opt == OPT::Addr)
        {
            p.cards.push_back( std::shared_ptr<Card>( new RLD_CARD(p.addr_counter, id_field)) );
        }


        return op_len;
    }
};


#endif //PROJECT_DC_HPP

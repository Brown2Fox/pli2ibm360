//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_DC_HPP
#define PROJECT_DC_HPP

#include <algorithm>

#include <Operation.hpp>
#include "../../ibm360_types.hpp"

class DC: public Operation {

public:
    DC(): Operation(1, '\x00', "DC   ") {};

public:
    int process1(const  Params& p) override
    {
//        alignAddr(p.addr_counter, 4);

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
            printf("DC: local symbol << val: %i, len: %i, name: %.8s\n", sym.val, sym.length, sym.name);
        end_with;

        return op_len;
    }

    int process2(const Params& p) override
    {
//        alignAddr(p.addr_counter, 4);

        uint8_t val_buff[56] = {[0 ... 55] = 0x40};
        uint8_t id_field[8] = {[0 ... 7] = 0x0};

        union { int32_t _int; float_t _float; uint32_t _address; } val = {0};
        auto pVal = reinterpret_cast<uint8_t*>(&val);

        enum class OPT : int { Nothing, Int, Float, Addr };
        auto opt = OPT::Nothing;
        if (std::memcmp(p.asm_line.structure.operand, "F'", 2) == 0) opt = OPT::Int;
        if (std::memcmp(p.asm_line.structure.operand, "E'", 2) == 0) opt = OPT::Float;
        if (std::memcmp(p.asm_line.structure.operand, "A(", 2) == 0) opt = OPT::Addr;

        switch (opt)
        {
            case OPT::Int:
            {
                char* val_asm = std::strtok((char *) p.asm_line.structure.operand + 2, "'");
                val._int = std::strtol(val_asm, nullptr, 10);

                pVal = (uint8_t *) &val._int;
                op_len = sizeof(int32_t);

                printf("DC: intVal=%i | len=%i\n", val._int, op_len);
                break;
            }

            case OPT::Float:
            {
                char* val_asm = std::strtok((char *) p.asm_line.structure.operand + 2, "'");
                val._float = std::strtof(val_asm, nullptr);

                pVal = (uint8_t *) &val._float;
                op_len = sizeof(float_t);

                printf("DC: floatVal=%f | len=%i\n", val._float, op_len);
                break;
            }

            case OPT::Addr:
            {
                char* val_asm = std::strtok((char *) p.asm_line.structure.operand + 2, ")");

                for (auto &sym: p.symbols) // iterate over p.symbols
                {
                    char* sym_name_tbl = std::strtok((char *) sym.name, " ");
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

        // store by bytes in reverse order
        for (int i = 0; i < op_len; i++)
        {
            val_buff[i] = *(pVal + (op_len-1) - i);
        }


        p.cards.push_back( std::shared_ptr<Card>(new TXT_CARD(op_len, p.addr_counter, val_buff, id_field)) );

        if (opt == OPT::Addr)
        {
            p.cards.push_back( std::shared_ptr<Card>( new RLD_CARD(p.addr_counter, id_field)) );
        }


        return op_len;
    }

    ~DC() override = default;
};


#endif //PROJECT_DC_HPP

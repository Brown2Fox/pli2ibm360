//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_USING_HPP
#define PROJECT_USING_HPP

#include "Operation.hpp"

class USING: public Operation
{
public ctors:
    USING(): Operation(1, '\x00', "USING") {};
public methods:
    int process1(Params& p) override
    {
        return 0; /*успешное заверш.подпрогр*/
    }
    int process2(Params& p) override
    {
        char* sym_name_asm_1 = nullptr;
        char* sym_name_asm_2 = nullptr;
        char* sym_name_tbl = nullptr;
        int BaseRegNum = 0;

        sym_name_asm_1 = strtok((char*)p.asm_line.structure.operand, ",");
        sym_name_asm_2 = strtok(nullptr, " ");

        if (isalpha((int)*sym_name_asm_2)) // is begin from letter
        {
            bool found = false;
            for (auto& sym: p.sym_table) // iterate over p.sym_table
            {
                sym_name_tbl = strtok((char*)sym.name, " ");
                if (strcmp(sym_name_tbl, sym_name_asm_2) == 0) // is found in p.sym_table
                {
                    BaseRegNum = sym.val;
                    found = true;
                    break;
                }
            }
            assertf(found, "Symbol not found: %s", sym_name_asm_2);
        }
        else
        {
            BaseRegNum = strtol(sym_name_asm_2, nullptr, 10);
        }

        assertf(BaseRegNum < p.baseregs.size(), "Wrong register number: %i", BaseRegNum);

        p.baseregs[BaseRegNum - 1].activity_flag = 'Y'; /* взвести призн.активн.  */
        if (sym_name_asm_1[0] == '*') // first parameter of ASM_LINE.structure.operand
        {
            p.baseregs[BaseRegNum - 1].base_addr = p.addr_counter;
        }
        else
        {
            bool found = false;
            for (auto& sym: p.sym_table) // iterate over p.sym_table
            {
                sym_name_tbl = strtok((char*)sym.name, " ");
                if (strcmp(sym_name_tbl, sym_name_asm_1) == 0) // is found in p.sym_table
                {
                    p.baseregs[BaseRegNum - 1].base_addr = sym.val;
                    found = true;
                    break;
                }
            }
            assertf(found, "Symbol not found: %s", sym_name_asm_1);
        }
        return 0;
    }
};


#endif //PROJECT_USING_HPP

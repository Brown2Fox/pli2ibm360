//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_EQU_HPP
#define PROJECT_EQU_HPP

#include "Operation.hpp"

class EQU: public Operation
{

public ctors:
    EQU(): Operation(1, '\x00', "EQU  ") {};

public methods:
    int process1(Params& p) override
    {
        with(p.sym_table[p.it_sym], sym)

            p.sym_flag = 'N';
            sym.length = this->op_len;

            if (p.asm_line.structure.operand[0] == '*') /*то                      */
            { /* запомнить в табл.симв.:*/
                sym.val = p.addr_counter; /*  addr_counter в поле sym_addr,   */
                sym.transfer_flag = 'R'; /*  'R' в пооле transfer_flag     */
            }
            else /*иначе запомн.в табл.симв*/
            { /* значение оп-нда пс.оп. */
                sym.val = (uint32) std::strtol(reinterpret_cast<char *>(p.asm_line.structure.operand), nullptr, 10);
                sym.transfer_flag = 'A'; /* 'A' в поле transfer_flag       */
            }

            printf("EQU: sym_table << val: %i, len: %i, name: %.8s\n", sym.val, sym.length, sym.name);

        end_with;
        return 0; /*успешное заверш.подпр.  */
    }

    int process2(Params& p) override
    {
        return 0; /*успешное заверш.подпр.  */
    }
};


#endif //PROJECT_EQU_HPP

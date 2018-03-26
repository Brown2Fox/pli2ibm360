#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <unistd.h>

#include <string>
#include <vector>
#include <memory>
#include <fstream>

#include "ibm360_types.h"
#include "Regular/BALR.hpp"
#include "Regular/BCR.hpp"
#include "Regular/ST.hpp"
#include "Regular/L.hpp"
#include "Regular/A.hpp"
#include "Regular/S.hpp"
#include "DC.hpp"
#include "DS.hpp"
#include "Pseudo/END.hpp"
#include "Pseudo/EQU.hpp"
#include "Pseudo/START.hpp"
#include "Pseudo/USING.hpp"


struct {
    const char *outFileName;
    const char *inFileName;
    bool verbosity;
} gl_opts;


void parse_args(int argc, char* argv[])
{
    int opt = 0;
    const char *optString = "i:o:v";

    gl_opts.inFileName = "";
    gl_opts.outFileName = "";
    gl_opts.verbosity = false;

    while( (opt = getopt( argc, argv, optString )) != -1 ) {
        switch( opt ) {

            case 'o':
                gl_opts.outFileName = optarg;
                break;

            case 'i':
                gl_opts.inFileName = optarg;
                break;

            case 'v':
                gl_opts.verbosity = true;
                break;

            default:
                break;
        }
    }
}


class Compiler
{

private fields:

    std::vector< std::shared_ptr< Operation > > operations = {
            std::shared_ptr<Operation>(new BALR()),
            std::shared_ptr<Operation>(new BCR()),
            std::shared_ptr<Operation>(new ST()),
            std::shared_ptr<Operation>(new L()),
            std::shared_ptr<Operation>(new A()),
            std::shared_ptr<Operation>(new S()),
            std::shared_ptr<Operation>(new DC()),
            std::shared_ptr<Operation>(new DS()),
            std::shared_ptr<Operation>(new END()),
            std::shared_ptr<Operation>(new EQU()),
            std::shared_ptr<Operation>(new START()),
            std::shared_ptr<Operation>(new USING())
    };

    std::vector<TBASR> baseregs = {
            { 0x00, 'N' },
            { 0x00, 'N' },
            { 0x00, 'N' },
            { 0x00, 'N' },
            { 0x00, 'N' },
            { 0x00, 'N' },
            { 0x00, 'N' },
            { 0x00, 'N' },
            { 0x00, 'N' },
            { 0x00, 'N' },
            { 0x00, 'N' },
            { 0x00, 'N' },
            { 0x00, 'N' },
            { 0x00, 'N' },
            { 0x00, 'N' }
    };

    std::vector<TSYM> sym_table;
    int it_sym = -1;
    char sym_flag =  'N';

    uint32 addr_counter = 0;

    std::vector<asm_mapping_u> asm_lines;

    std::vector< std::shared_ptr<Card> > cards;

public ctors:

    Compiler()
    {

        for (const auto& op: operations)
        {
            op->print();
        }

    };

public methods:

    int first_iterate()
    {

        for (auto& asm_line: asm_lines) // iterate over ASM_TEXT
        {

            if (asm_line.structure.label[0] != ' ')
            {
                it_sym++;
                sym_flag = 'Y';
                TSYM tsym = {};
                memcpy(tsym.name, asm_line.structure.label, 8);
                tsym.val = addr_counter;

                sym_table.push_back(tsym);
            }

            bool op_found = false;

            for (const auto &op: operations) // iterate over op_table
            {
                if (op->isOperation(asm_line.structure.op_name))
                {
                    auto p = Params(sym_flag, sym_table, it_sym, addr_counter, asm_line, baseregs, cards);

                    int retval = op->process1( p );

                    sym_flag = 'N';

                    if (retval < 0)
                    {
                        return retval;
                    }

                    addr_counter += retval;

                    op_found = true;
                    break;
                }
            }

            printf("1 addr_count=%i\n", addr_counter);

            assertf(op_found, "Operation not found: %.5s", asm_line.structure.op_name);
        }
        return -1;
    }

    int second_iterate()
    {

        for (auto& asm_line: asm_lines) // iterate over ASM_TEXT
        {

            bool op_found = false;
            for (const auto &op: operations) // iterate over op_table
            {
                if (op->isOperation(asm_line.structure.op_name))
                {
                    auto p = Params(sym_flag, sym_table, it_sym, addr_counter, asm_line, baseregs, cards);
                    int retval = op->process2(p);

                    if (retval < 0)
                    {
                        return retval;
                    }

                    addr_counter += retval;
                    op_found = true;
                    break;
                }
            }

            assertf(op_found, "Operation not found: %.5s", asm_line.structure.op_name);
        }
        return -1;
    }


    int read_file()
    {

        std::ifstream input(gl_opts.inFileName);
        assertf(input, "Error while trying to open file: %s", gl_opts.inFileName);

        for (std::string line; std::getline(input, line); )
        {
            asm_mapping_u asm_line = {};
            memcpy(asm_line.buffer, line.c_str(), 80);
            asm_line.structure.COMMENT[51] = '\0';

            asm_lines.push_back(asm_line);

            printf("%s <<\n", asm_line.buffer);
        }

        input.close();
        return 0;
    }

    int write_file()
    {
        std::ofstream output(gl_opts.outFileName);
        assertf(output, "Error while trying to open file: %s", gl_opts.outFileName);

        for (const auto& card: cards)
        {
            output.write(reinterpret_cast<const char *>(card->getBuffer()), 80);
        }

        output.close();
        return 0;
    }

};




int main(int argc, char* argv[]) 
{
    parse_args(argc, argv);

    Compiler comp;

    comp.read_file();


    comp.first_iterate();
    comp.second_iterate();

    comp.write_file();


    return 0;
}

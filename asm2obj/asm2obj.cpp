#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <unistd.h>

#include <string>
#include <vector>
#include <memory>
#include <fstream>

#include "ibm360_types.hpp"
#include "Regular/BALR.hpp"
#include "Regular/BCR.hpp"
#include "Regular/ST.hpp"
#include "Regular/L.hpp"
#include "Regular/A.hpp"
#include "Regular/S.hpp"

#include "Pseudo/DC.hpp"
#include "Pseudo/DS.hpp"
#include "Pseudo/END.hpp"
#include "Pseudo/EQU.hpp"
#include "Pseudo/START.hpp"
#include "Pseudo/USING.hpp"
#include <Pseudo/EXTRN.hpp>

struct {
    std::vector<std::string> outFileNames = {};
    std::vector<std::string> inFileNames = {};
    bool verbosity = false;
} gl_args;


void parse_args(int argc, char* argv[])
{
    int opt = 0;
    const char *optString = "i:o:v";

    while( (opt = getopt( argc, argv, optString )) != -1 ) {
        switch( opt ) {

            case 'o':
                gl_args.outFileNames.emplace_back(optarg);
                break;

            case 'i':
                gl_args.inFileNames.emplace_back(optarg);
                break;

            case 'v':
                gl_args.verbosity = true;
                break;

            default:
                break;
        }
    }
}


class Compiler
{

private:

    typedef std::shared_ptr< Operation > OpPtr;
    std::vector< OpPtr > operations = {
            OpPtr(new BALR()),
            OpPtr(new BCR()),
            OpPtr(new ST()),
            OpPtr(new L()),
            OpPtr(new A()),
            OpPtr(new S()),
            OpPtr(new DC()),
            OpPtr(new DS()),
            OpPtr(new END()),
            OpPtr(new EQU()),
            OpPtr(new START()),
            OpPtr(new USING()),
            OpPtr(new EXTRN())
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

    std::vector<TSYM> symbols;
    char label_flag =  'N';

    uint32_t addr_counter = 0;

    std::vector<asm_mapping_u> asm_lines;

    typedef std::shared_ptr<Card> CardPtr;
    std::vector< CardPtr > cards;

public:

    Compiler()
    {
        ESD_CARD::ID_NUM = 0;
        TXT_CARD::ID_NUM = 0;
        RLD_CARD::ID_NUM = 1;
    };

public:

    int first_iterate()
    {

        for (auto& asm_line: asm_lines) // iterate over ASM_TEXT
        {

            if (asm_line.structure.label[0] != ' ')
            {
                label_flag = 'Y';
                TSYM tsym = {};
                memcpy(tsym.name, asm_line.structure.label, 8);
                tsym.val = addr_counter;

                symbols.push_back(tsym);
            }

            bool op_found = false;

            for (const auto &op: operations) // iterate over op_table
            {
                if (op->isOperation(asm_line.structure.op_name))
                {
                    auto p = Params(label_flag, symbols, symbols.back(), addr_counter, asm_line, baseregs, cards);

                    int retval = op->process1( p );

                    label_flag = 'N';

                    if (retval < 0)
                    {
                        return retval;
                    }

                    printf(" --- First: addr_counter=%i, OP=%.5s, op_len=%i\n", addr_counter, asm_line.structure.op_name, retval);
                    addr_counter += retval;

                    op_found = true;
                    break;
                }
            }

            assertf(op_found, "Operation not found: %.5s", asm_line.structure.op_name);
        }
        return -1;
    }

    int second_iterate()
    {

        for (auto& asm_line: asm_lines) // iterate over asm_lines
        {

            bool op_found = false;
            for (const auto &op: operations) // iterate over operations
            {
                if (op->isOperation(asm_line.structure.op_name))
                {
                    auto p = Params(label_flag, symbols, symbols.back(), addr_counter, asm_line, baseregs, cards);
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


    int read_file(std::string& inFileName)
    {

        std::ifstream input(inFileName);
        assertf(input, "Error while trying to open file: %s", inFileName);

        for (std::string line; std::getline(input, line); )
        {
            asm_mapping_u asm_line = {};
            memcpy(asm_line.buffer, line.c_str(), 80);
            asm_line.structure.comment[51] = '\0';

            asm_lines.push_back(asm_line);

            printf("%s <<\n", asm_line.buffer);
        }

        input.close();
        return 0;
    }

    int write_file(std::string& outFileName)
    {
        std::ofstream output(outFileName, std::ofstream::binary);
        assertf(output, "Error while trying to open file: %s", outFileName);

        auto outReadable = outFileName + ".txt";
        std::ofstream output_readable(outReadable);
        assertf(output_readable, "Error while trying to open file: %s", outReadable);

        for (const auto& card: cards)
        {
            output.write(reinterpret_cast<const char *>(card->getBuffer()), 80);
            output_readable << card->getFormatOutput() << std::endl;
        }

        output.close();
        return 0;
    }


    int hexDump() {

        for (auto& card: cards)
        {
            uint8_t* buff = card->getBuffer();
            for (int i = 0; i < 80; i++)
            {
                printf("%c", buff[i]);
            }
            printf("\n\n");
        }

        return 0;
    }

};


int main(int argc, char* argv[]) 
{
    parse_args(argc, argv);

    for (auto in = gl_args.inFileNames.begin(), out = gl_args.outFileNames.begin();
         in != gl_args.inFileNames.end() || out != gl_args.outFileNames.end();
         in++, out++)
    {
        Compiler comp;

        comp.read_file(*in);

        comp.first_iterate();
        comp.second_iterate();

        comp.write_file(*out);

    }

    return 0;
}

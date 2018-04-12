#ifndef _IBM_360_OPERATION_
#define _IBM_360_OPERATION_

#include "../Utils.hpp"
#include <cstring>
#include <cstdio>
#include <Card.hpp>
#include <memory>
#include "ESD_CARD.hpp"
#include "TXT_CARD.hpp"
#include "RLD_CARD.hpp"
#include "END_CARD.hpp"
#include "../ibm360_types.hpp"

class Params
{

public:

    char& label_flag;
    std::vector<TSYM>& symbols;
    TSYM& it_sym;
    uint32_t &addr_counter;
    asm_mapping_u &asm_line;
    std::vector<TBASR>& baseregs;
    std::vector< std::shared_ptr<Card> >& cards;

public:
    Params(char &sym_flag_,
           std::vector<TSYM>& sym_table_,
           TSYM& it_sym_,
           uint32_t &addr_counter_,
           asm_mapping_u &asm_line_,
           std::vector<TBASR>& baseregs_,
           std::vector< std::shared_ptr<Card> >& cards_) :
        label_flag(sym_flag_),
        symbols(sym_table_),
        it_sym(it_sym_),
        addr_counter(addr_counter_),
        asm_line(asm_line_),
        baseregs(baseregs_),
        cards(cards_)
    {}
};


class Operation
{
protected:
    uint8_t op_type = 0;
    uint8_t op_code = 0;
    uint8_t op_len = 0;
    uint8_t op_name[5] = {};
public:
    Operation() = default;
    Operation(uint8_t op_type, uint8_t op_code, const char* op_name)
    {
        this->op_type = op_type;
        this->op_code = op_code;
        std::strncpy(reinterpret_cast<char *>(this->op_name), op_name, 5);
    }
public:

    virtual int process1(const Params &p) = 0;
    virtual int process2(const Params &p) = 0;

    bool isOperation(unsigned char *op_name)
    {
        return memcmp(op_name, this->op_name, 5) == 0;
    }


    void print()
    {
        std::printf("op_type=%i,op_code=%i,op_len=%i,op_name=%.5s\n",op_type, op_code, op_len, op_name);
    }


    virtual ~Operation() = default;

protected:
    void alignAddr(uint32_t& addr, uint8_t by)
    {
        if (addr % by != 0)
        {
            addr = addr + (by - addr % by);
        }
    }

    template<typename Tchar>
    bool isIdentifier(Tchar c)
    {
        return  (c == '$' || std::isalpha(c, std::locale{}) || c == '^');
    }

};

#endif // _IBM_360_OPERATION_
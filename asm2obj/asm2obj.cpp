#include <cstring> /*вкл.строковые подпрогр. */
#include <cstdlib> /*вкл.подпрогр.преобр.данн*/
#include <cstdio> /*вкл.подпр.станд.вв/выв  */
#include <cctype> /*вкл.подпр.классиф.симв. */
#include <unistd.h>
#include <ibm360_types.h>
#include <cassert>
#include <cerrno>
#include <string>
#include <vector>
#include <memory>


#define clean_errno() (errno == 0 ? "None" : strerror(errno))
#define log_error(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define assertf(A, M, ...) if(!(A)) {log_error(M, ##__VA_ARGS__); assert(A); }


#define ASM_TEXT_LEN 100
#define OBJTEXT_LEN 50 /*длина об'ектн. текста   */



#define CODE_END 100


struct {
    const char *outFileName;
    const char *inFileName;
    int verbosity;
} gl_opts;



void parse_args(int argc, char* argv[])
{
    int opt = 0;
    const char *optString = "i:o:v";

    gl_opts.inFileName = "";
    gl_opts.outFileName = "";
    gl_opts.verbosity = 0;

    while( (opt = getopt( argc, argv, optString )) != -1 ) {
        switch( opt ) {

            case 'o':
                gl_opts.outFileName = optarg;
                break;

            case 'i':
                gl_opts.inFileName = optarg;
                break;

            case 'v':
                gl_opts.verbosity = 1;
                break;

            default:
                break;
        }
    }
}



// callbacks for first iteration

int F_DC(TMOP*);
int F_DS(TMOP*);
int F_END(TMOP*);
int F_EQU(TMOP*);
int F_START(TMOP*);
int F_USING(TMOP*);

int F_RR(TMOP*);
int F_RX(TMOP*);

// callbacks for second iteration

int S_DC(TMOP*);
int S_DS(TMOP*);
int S_END(TMOP*);
int S_EQU(TMOP*);
int S_START(TMOP*);
int S_USING(TMOP*);

int S_RR(TMOP*);
int S_RX(TMOP*);

// Card generators

void GEN_ESD_CARD(uint32 pr_length, uint32 r_addr, char* sym_name);
void GEN_TXT_CARD(uint8 op_length, uint32 op_addr, uint8* op_body);
void GEN_RLD_CARD(uint32 offset_addr);
void GEN_END_CARD();


char asm_text[ASM_TEXT_LEN][80];
asm_mapping_u asm_line;

uint32 addr_counter = 0;

const int NSYM = 10;
TSYM sym_table[NSYM];

int it_sym = -1;
char sym_flag =  'N';

const int ALL_OPS = 12;


/*
***** ТАБЛИЦА базовых регистров
*/

const int NUM_OF_BASEREGS = 15;
TBASR baseregs[NUM_OF_BASEREGS] = {
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

/*
***** Б Л О К   об'явления массива с об'ектным текстом
*/

uint8 obj_text[OBJTEXT_LEN][80]; /*массив об'ектных карт   */
int it_card = 0; /*указатель текущ.карты   */

///////////////////////////////////////////////////////////////////////////////


union {
    ESD_ structure;
    unsigned char buffer[80];
} ESD_CARD;

union {
    RLD_ structure;
    unsigned char buffer[80];
} RLD_CARD;

union {
    TXT_ structure;
    unsigned char buffer[80];
} TXT_CARD;

union {
    END_ structure;
    unsigned char buffer[80];
} END_CARD;

#define fields /*fields*/
#define methods /*methods*/
#define ctors /*ctors*/

class Operation
{
protected fields:
    uint8 op_type = 0;
    uint8 op_code = 0;
    uint8 op_len = 0;
    uint8 op_name[5] = {};
public ctors:
    Operation() = default;
    Operation(uint8 op_type, uint8 op_code, const char* op_name)
    {
        this->op_type = op_type;
        this->op_code = op_code;
        std::strncpy(reinterpret_cast<char *>(this->op_name), op_name, 5);
    }
public methods:
    virtual int process1() = 0;
    virtual int process2() = 0;
    bool isOperation(unsigned char *op_name)
    {
        return memcmp(op_name, this->op_name, 5) == 0;
    }
};


class RR: public Operation
{
protected fields:
    union {
        unsigned char buffer[2];
        OP_RR structure;
    } rr;

protected ctors:
    RR() { op_len = 2; }
    RR(uint8 op_type, uint8 op_code, const char* op_name) : Operation(op_type, op_code, op_name) {
        RR();
    };
public methods:
    int process1() override
    {
        if (sym_flag == 'Y') /*если ранее обнар.метка, */
        {
            sym_table[it_sym].sym_length = this->op_len;
            sym_table[it_sym].transfer_flag = 'R';
        }
        addr_counter += this->op_len;
        return 0;
    }
    int process2() override
    {
        char* sym_name_tbl;
        char* sym_name_asm_1;
        char* sym_name_asm_2;
        unsigned char R1R2;
        int J;

        rr.structure.OP_CODE = this->op_code;

        sym_name_asm_1 = strtok((char*)asm_line.structure.operand, ",");
        sym_name_asm_2 = strtok(NULL, " ");

        if (isalpha((int)*sym_name_asm_1))
        {
            bool found = false;
            for (J = 0; J <= it_sym; J++) // iterate over sym_table
            {
                sym_name_tbl = strtok((char*)sym_table[J].sym_name, " ");
                if (strcmp(sym_name_tbl, sym_name_asm_1) == 0)
                {
                    found = true;
                    R1R2 = sym_table[J].sym_addr << 4;
                }
            }
            assertf(found, "Symbol not found: %s", sym_name_asm_1);
        }
        else /*иначе, берем в качестве */
        { /*перв.операнда машинн.ком*/
            R1R2 = strtol(sym_name_asm_1, NULL, 10) << 4; /*значен.выбр.   лексемы  */
        }

        if (std::isalpha((int)*sym_name_asm_2))
        {
            bool found = false;
            for (J = 0; J <= it_sym; J++) // iterate over sym_table
            {
                sym_name_tbl = strtok((char*)sym_table[J].sym_name, " ");
                if (strcmp(sym_name_tbl, sym_name_asm_2) == 0)
                {
                    found = true;
                    R1R2 = R1R2 + sym_table[J].sym_addr;
                }
            }
            assertf(found, "Symbol not found: %s", sym_name_asm_1);
        }
        else /*иначе, берем в качестве */
        { /*втор.операнда машинн.ком*/
            R1R2 = R1R2 + atoi(sym_name_asm_2); /*значен.выбр.   лексемы  */
        }


        rr.structure.R1_R2 = R1R2;

        printf("RR: oper=%.5s, op_code=%x, r1_r2=%x\n",
               this->op_name,
               rr.structure.OP_CODE,
               rr.structure.R1_R2);

        GEN_TXT_CARD(this->op_len, addr_counter, rr.buffer);

        addr_counter += this->op_len;
        return 0; /*выйти из подпрограммы   */
    }
};

class RX: public Operation
{
protected fields:
    union {
        unsigned char buffer[4];
        OP_RX structure;
    } rx;

protected ctors:
    RX() { op_len = 4; }
    RX(uint8 op_type, uint8 op_code, const char* op_name) : Operation(op_type, op_code, op_name) {
        RX();
    };
public methods:
    int process1() override
    {
        if (sym_flag == 'Y') /*если ранее обнар.метка, */
        {
            sym_table[it_sym].sym_length = this->op_len;
            sym_table[it_sym].transfer_flag = 'R';
        }
        addr_counter += this->op_len;
        return 0;
    }
    int process2() override
    {
        char* sym_name_tbl;
        char* sym_name_asm_1;
        char* sym_name_asm_2;
        char* PTR_;
        int delta;
        int sym_addr;
        int basereg_num;
        uint8 R1X2;
        int B2D2;

        rx.structure.OP_CODE = this->op_code;

        /*в перем. c указат.sym_name_asm_1*/
        /*выбираем первую лексему */
        /*операнда текущей карты  */
        /*исх.текста АССЕМБЛЕРА   */
        sym_name_asm_1 = strtok((char*)asm_line.structure.operand, ",");

        /*в перем. c указат.sym_name_asm_2*/
        /*выбираем вторую лексему */
        /*операнда текущей карты  */
        /*исх.текста АССЕМБЛЕРА   */
        sym_name_asm_2 = strtok(NULL, " ");

        if (isalpha((int)*sym_name_asm_1))
        {
            bool found = false;
            for (int j = 0; j <= it_sym; j++) // iterate over sym_table
            {
                sym_name_tbl = strtok((char*)sym_table[j].sym_name, " ");
                if (!strcmp(sym_name_tbl, sym_name_asm_1)) /* и при совпадении:      */
                {
                    found = true;
                    R1X2 = sym_table[j].sym_addr << 4; /*  метки в качестве перв.*/
                }
            }
            assertf(found, "Symbol not found: %s", sym_name_asm_1);
        }
        else /*иначе, берем в качестве */
        { /*перв.операнда машинн.ком*/
            R1X2 = strtol(sym_name_asm_1, NULL, 10) << 4; /*значен.выбр.   лексемы  */
        }

        if (isalpha((int)*sym_name_asm_2))
        {
            bool found = false;
            for (int j = 0; j <= it_sym; j++) // iterate over sym_table
            {
                sym_name_tbl = strtok((char*)sym_table[j].sym_name, " ");
                if (!strcmp(sym_name_tbl, sym_name_asm_2))
                {
                    found = true;
                    basereg_num = 0; /*   номера базов.регистра*/
                    delta = 0xfff - 1; /*   и его значен.,а также*/
                    sym_addr = sym_table[j].sym_addr; /*   смещен.втор.операнда */
                    for (int i = 0; i < 15; i++) // iterate over baseregs
                    {
                        /* призн.активности,      */
                        /*  и                     */
                        /* значенение, меньшее по */
                        /* величине,но наиболее   */
                        /* близкое к смещению вто-*/
                        /* рого операнда          */
                        if (baseregs[i].activity_flag == 'Y' && sym_addr - baseregs[i].base_addr >= 0 && sym_addr - baseregs[i].base_addr < delta)
                        {
                            basereg_num = i + 1;
                            delta = sym_addr - baseregs[i].base_addr;
                        }
                    }
                    if (basereg_num == 0 || delta > 0xfff) /*если баз.рег.не выбр.,то*/
                        return (5); /* заверш.подпр.по ошибке */
                    else /*иначе                   */
                    { /* сформировыать машинное */
                        B2D2 = basereg_num << 12; /* представление второго  */
                        B2D2 = B2D2 + delta; /* операнда в виде B2_D2   */
                        PTR_ = (char*)&B2D2; /* и в соглашениях ЕС ЭВМ */
                        swab(PTR_, PTR_, 2); /* с записью в тело ком-ды*/
                        rx.structure.B2_D2 = B2D2;
                    }
                }
            }
            assertf(found, "Symbol not found: %s", sym_name_asm_2);
        }
        else  assertf(false, "Wrong second operand format: %s", sym_name_asm_2);

        rx.structure.R1_X2 = R1X2; /*дозапись перв.операнда  */

        printf("RX: oper=%.5s, op_code=%x, r1_x2=%x, b2_d2=%x\n",
               this->op_name,
               rx.structure.OP_CODE,
               rx.structure.R1_X2,
               rx.structure.B2_D2);

        GEN_TXT_CARD(this->op_len, addr_counter, rx.buffer);
        addr_counter += this->op_len;
        return 0;
    }
};

class BALR: public RR
{
public ctors:
    BALR(): RR(0, '\x05', "BALR ") {};
};
class BCR: public RR {
public ctors:
    BCR(): RR(0, '\x07', "BCR  ") {};
};

class ST: public RX {
public ctors:
    ST(): RX(0, '\x50', "ST   ") {};
};

class L: public RX {
public ctors:
    L(): RX(0, '\x58', "L     ") {};
};

class A: public RX {
public ctors:
    A(): RX(0, '\x5A', "A     ") {};
};

class S: public RX {
public ctors:
    S(): RX(0, '\x5B', "S     ") {};
};


class DC: public RX
{
public ctors:
    DC(): RX(1, '\x00', "DC   ") {};

public methods:
    int process1() override
    {
        uint8 offset = 4;
        if (addr_counter % 4)
        {
            addr_counter = (addr_counter / 4 + 1) * 4; /* установ.addr_counter на гр.сл.*/
        }

        if (sym_flag == 'Y')
        {
            sym_flag = 'N'; // clear flag

            sym_table[it_sym].sym_length = 4; /*  уст.длину симв. =  4, */
            sym_table[it_sym].transfer_flag = 'R'; /*  а,призн.перемест.='R' */
            sym_table[it_sym].sym_addr = addr_counter; /*   запомн. в табл.симв. */
        }

        addr_counter += offset;
        return 0;
    }

    int process2() override
    {
        char* sym_name_tbl = NULL;
        char* sym_name_asm = NULL;
        rx.structure.OP_CODE = 0; /*занулим два старших     */
        rx.structure.R1_X2 = 0; /*байта RX.structure          */
        rx.structure.B2_D2 = 0;
        bool need_rld = false;


        // variants:
        if (std::memcmp(asm_line.structure.operand, "F'", 2) == 0)
        {
            sym_name_asm = std::strtok((char*)asm_line.structure.operand + 2, "'");
            rx.structure.B2_D2 = std::strtol(sym_name_asm, NULL, 10); /*перевод ASCII-> int     */
            swab((char*)&rx.structure.B2_D2,(char*)&rx.structure.B2_D2,2);
            op_len = 4;
        }
        else if (std::memcmp(asm_line.structure.operand, "E'", 2) == 0)
        {
            // TODO: add float point magic
            sym_name_asm = std::strtok((char*)asm_line.structure.operand + 2, "'");

            op_len = 4;
        }
        else if (std::memcmp(asm_line.structure.operand, "A(", 2) == 0)
        {
            need_rld = true;
            sym_name_asm = std::strtok((char*)asm_line.structure.operand + 2, ")");

            bool found = false;
            for (int i = 0; i <= it_sym; i++) // iterate over sym_table
            {
                sym_name_tbl = std::strtok((char*)sym_table[i].sym_name, " ");
                if (strcmp(sym_name_tbl, sym_name_asm) == 0)
                {

                    found = true;
                    rx.structure.B2_D2 = sym_table[i].sym_addr;
                    break;
                }
            }
            assertf(found,"Symbol not found: %s", sym_name_asm);

            op_len = 4;
        }
        else assertf(false,"Unknown format: %s", asm_line.structure.operand);

        printf("RX: oper=%.5s, sym=%s, op_code=%x, r1_x2=%x, b2_d2=%x\n",
               this->op_name,
               sym_name_asm,
               rx.structure.OP_CODE,
               rx.structure.R1_X2,
               rx.structure.B2_D2);

        GEN_TXT_CARD(op_len, addr_counter, rx.buffer); /*формирование TXT_CARD-карты  */
        if (need_rld) GEN_RLD_CARD(addr_counter);

        addr_counter += op_len;
        return 0; /*успешн.завершение подпр.*/
    }
};

class DS: public RX
{
public ctors:
    DS(): RX(1, '\x00', "DS   ") {};
public methods:
    int process1() override
    {
        uint8 offset = 4;

        if (addr_counter % 4)
        {
            addr_counter = (addr_counter / 4 + 1) * 4; /* установ.addr_counter на гр.сл.*/
        }

        if (sym_flag == 'Y')
        {
            sym_flag = 'N'; // clear flag

            sym_table[it_sym].sym_length = 4;
            sym_table[it_sym].transfer_flag = 'R';
            sym_table[it_sym].sym_addr = addr_counter;
        }

        addr_counter += offset;
        return 0;
    }
    int process2() override
    {
        uint8 offset = 0;
        rx.structure.OP_CODE = 0; /*занулим два старших     */
        rx.structure.R1_X2 = 0; /*байта RX.structure          */
        /* если операнд начинается*/
        /* с комбинации F'        */
        if (asm_line.structure.operand[0] == 'F' )
        {
            rx.structure.B2_D2 = 0; /*занулим RX.structure.B2_D2   */
            offset = 4;
        }
        else assertf(0,"Unknown format: %s", asm_line.structure.operand);

        printf("RX: oper=%.5s, op_code=%x, r1_x2=%x, b2_d2=%x\n",
               this->op_name,
               rx.structure.OP_CODE,
               rx.structure.R1_X2,
               rx.structure.B2_D2);

        GEN_TXT_CARD(offset, addr_counter, rx.buffer); /*формирование TXT_CARD-карты  */

        addr_counter += offset;
        return 0; /*успешно завершить подпр.*/
    }
};

class END: public Operation
{
public ctors:
    END(): Operation(1, '\x00', "END  ") {};
public methods:
    int process1() override
    {
        return CODE_END; /*выход с призн.конца 1-го*/
    }
    int process2() override
    {
        printf("PS: oper=%.5s\n", this->op_name);
        GEN_END_CARD();
        return CODE_END; /*выход с призн.конца 2-го просмотра*/
    }
};

class EQU: public Operation
{
public ctors:
    EQU(): Operation(1, '\x00', "EQU  ") {};
public methods:
    int process1() override
    {
        sym_flag = 'N';
        sym_table[it_sym].sym_length = this->op_len;

        if (asm_line.structure.operand[0] == '*') /*то                      */
        { /* запомнить в табл.симв.:*/
            sym_table[it_sym].sym_addr = addr_counter; /*  addr_counter в поле sym_addr,   */
            sym_table[it_sym].transfer_flag = 'R'; /*  'R' в пооле transfer_flag     */
        }
        else /*иначе запомн.в табл.симв*/
        { /* значение оп-нда пс.оп. */
            sym_table[it_sym].sym_addr = std::strtol((const char *)asm_line.structure.operand, NULL, 10);
            sym_table[it_sym].transfer_flag = 'A'; /* 'A' в поле transfer_flag       */
        }

        return 0; /*успешное заверш.подпр.  */
    }
    int process2() override
    {
        return (0); /*успешное заверш.подпр.  */
    }
};

class START: public Operation
{
public ctors:
    START(): Operation(1, '\x00', "START") {};
public methods:
    int process1() override
    {
        sym_flag = 'N'; // clear flag

        addr_counter = strtol((const char *)asm_line.structure.operand, NULL, 10);

        if (addr_counter % 8) /*если это значение не    */
        { /*кратно 8, то сделать его*/
            addr_counter = (addr_counter + (8 - addr_counter % 8)); /*кратным                 */
        } /*запомнить в табл.симв.: */

        sym_table[it_sym].sym_addr = addr_counter; /* addr_counter в поле sym_addr,    */
        sym_table[it_sym].sym_length = this->op_len; /* 1 в поле sym_length,        */
        sym_table[it_sym].transfer_flag = 'R'; /* 'R' в поле transfer_flag       */

        return 0; /*успешное заверш.подпрогр*/
    }
    int process2() override
    {
        char* sym_name_asm = NULL; /*рабочих                 */
        char* sym_name_tbl = NULL;
        uint32 addr_offset = 0; /*                        */

        sym_name_asm = strtok((char*)asm_line.structure.label, " ");

        bool found = false;
        for (int i = 0; i <= it_sym; i++) // iterate over sym_table
        {
            sym_name_tbl = strtok((char*)sym_table[i].sym_name, " ");
            if (strcmp(sym_name_tbl, sym_name_asm) == 0)
            {
                found = true;
                addr_offset = addr_counter - sym_table[i].sym_addr; /*  знач.этой метки, обра-*/
                addr_counter = sym_table[i].sym_addr; /*устанавл.addr_counter, равным  */
                printf("PS: oper=%.5s, sym=%s\n", this->op_name, sym_name_asm);
                GEN_ESD_CARD(addr_offset, addr_counter, sym_name_tbl);
            }
        }
        assertf(found,"Symbol not found: %s", sym_name_asm);
        return 0;
    }
};

class USING: public Operation
{
public ctors:
    USING(): Operation(1, '\x00', "USING") {};
public methods:
    int process1() override
    {
        return 0; /*успешное заверш.подпрогр*/
    }
    int process2() override
    {
        char* sym_name_asm_1 = NULL;
        char* sym_name_asm_2 = NULL;
        char* sym_name_tbl = NULL;
        int basereg_num = 0;

        sym_name_asm_1 = strtok((char*)asm_line.structure.operand, ",");
        sym_name_asm_2 = strtok(NULL, " ");

        if (isalpha((int)*sym_name_asm_2)) // is begin from letter
        {
            bool found = false;
            for (int i = 0; i <= it_sym; i++) // iterate over sym_table
            {
                sym_name_tbl = strtok((char*)sym_table[i].sym_name, " ");
                if (strcmp(sym_name_tbl, sym_name_asm_2) == 0) // is found in sym_table
                {
                    found = true;
                    basereg_num = sym_table[i].sym_addr;
                    assertf(basereg_num < NUM_OF_BASEREGS, "Wrong register number: %i", basereg_num);
                }
            }
            assertf(found, "Symbol not found: %s", sym_name_asm_2);
        }
        else
        {
            basereg_num = strtol(sym_name_asm_2, NULL, 10);
            assertf(basereg_num < NUM_OF_BASEREGS, "Wrong register number: %i", basereg_num);
        }

        baseregs[basereg_num - 1].activity_flag = 'Y'; /* взвести призн.активн.  */
        if (*sym_name_asm_1 == '*') // first parameter of ASM_LINE.structure.operand
        {
            baseregs[basereg_num - 1].base_addr = addr_counter;
        }
        else
        {
            bool found = false;
            for (int i = 0; i <= it_sym; i++) // iterate over sym_table
            {
                sym_name_tbl = strtok((char*)sym_table[i].sym_name, " ");
                if (strcmp(sym_name_tbl, sym_name_asm_1) == 0) // is found in sym_table
                {
                    found = true;
                    baseregs[basereg_num - 1].base_addr = sym_table[i].sym_addr;
                }
            }
            assertf(found, "Symbol not found: %s", sym_name_asm_1);
        }
        return 0;
    }
};


//////////////////////////////////////////////////////////////////////////////

void GEN_ESD_CARD(uint32 pr_length, uint32 r_addr, char * sym_name)
{
    swab((char*)&pr_length, (char*) &pr_length, 2); /*  соглашениях ЕС ЭБМ, и */
    ESD_CARD.structure.PR_LENGTH[0] = (uint8) 0; /*  записыв.ее в ESD_CARD-карту*/
    ESD_CARD.structure.PR_LENGTH[1] = (uint8) *(&pr_length); /*  побайтно              */
    ESD_CARD.structure.PR_LENGTH[2] = (uint8) *(&pr_length + 1); /*                        */

    // FIXME: black magic in ESD_CARD.R_ADDR
    ESD_CARD.structure.R_ADDR[0] = (uint8) '\x00'; /*в соглашениях ЕС ЭВМ    */
    ESD_CARD.structure.R_ADDR[1] = (uint8) *(&r_addr + 1); /*двоичного целого        */
    ESD_CARD.structure.R_ADDR[2] = (uint8) *(&r_addr); /*ESD_CARD-карты в формате     */

    memcpy(ESD_CARD.structure.SYM_NAME, sym_name, strlen(sym_name));
    memcpy(ESD_CARD.structure.ID_FIELD, sym_name, strlen(sym_name));

    memcpy(obj_text[it_card], ESD_CARD.buffer, 80);

    printf("esd card: pr_len=%i, r_addr=%i, sym_name=%s, id_field=%s\n", pr_length,r_addr,sym_name, ESD_CARD.structure.ID_FIELD);

    it_card += 1; /*коррекц.инд-са своб.к-ты*/
}

/*..........................................................................*/
void GEN_TXT_CARD(uint8 op_length, uint32 op_addr, uint8* op_body)
{
    // FIXME: black magic in TXT_CARD.OP_ADDR
    TXT_CARD.structure.OP_ADDR[0] = (uint8) '\x00';
    TXT_CARD.structure.OP_ADDR[1] = (uint8) *(&op_addr + 1);
    TXT_CARD.structure.OP_ADDR[2] = (uint8) *(&op_addr);

    TXT_CARD.structure.OP_LENGTH[1] = op_length;

    if (op_length == 2)
    {
        memset(TXT_CARD.structure.OP_BODY, 64, 4);
    }

    memcpy(TXT_CARD.structure.OP_BODY, op_body, op_length);
    memcpy(TXT_CARD.structure.ID_FIELD, ESD_CARD.structure.ID_FIELD, 8);

    memcpy(obj_text[it_card], TXT_CARD.buffer, 80);

    printf("txt card: op_len=%i, op_addr=%i, op_body=%s, id_field=%.8s\n", op_length, op_addr, op_body, TXT_CARD.structure.ID_FIELD);

    it_card += 1;
}

/*..........................................................................*/
void GEN_RLD_CARD(uint32 offset_addr)
{
    // FIXME: black magic in RLD_CARD.OFFSET_ADDR
    RLD_CARD.structure.OFFSET_ADDR[0] = (uint8) '\x00';
    RLD_CARD.structure.OFFSET_ADDR[1] = (uint8) *(&offset_addr + 1);
    RLD_CARD.structure.OFFSET_ADDR[2] = (uint8) *(&offset_addr);

    RLD_CARD.structure.SIGN[0] = 0x0;
    RLD_CARD.structure.SIGN[1] = 0xC;

    memcpy(RLD_CARD.structure.ID_FIELD, ESD_CARD.structure.ID_FIELD, 8);

    memcpy(obj_text[it_card], RLD_CARD.buffer, 80);

    printf("rld card: offset_addr=%i\n", offset_addr);

    it_card += 1;
}

/*..........................................................................*/
void GEN_END_CARD()
{
    memcpy(END_CARD.structure.ID_FIELD, ESD_CARD.structure.ID_FIELD, 8);
    memcpy(obj_text[it_card], END_CARD.buffer, 80);

    printf("end card: %.80s\n", END_CARD.buffer);

    it_card += 1;
}

//////////////////////////////////////////////////////////////////////////////

int GEN_OBJ_FILE()
{
    std::FILE* pFile = std::fopen(gl_opts.outFileName, "wb");
    assertf(pFile, "Error while trying to write result in output file");

    auto ret = std::fwrite(obj_text, 80, (size_t) it_card, pFile);

    std::fclose(pFile);
    return static_cast<int>(ret);
}

/*..........................................................................*/
void INIT()
{

    /*
    ***** и н и ц и а л и з а ц и я   полей буфера формирования записей ESD_CARD-типа
    *****                             для выходного объектного файла
    */

    ESD_CARD.structure.PADDING1 = 0x02;
    memcpy(ESD_CARD.structure.CARD_TYPE, "ESD", 3);
    memset(ESD_CARD.structure.PADDING2, 0x40, 10);
    ESD_CARD.structure.ID_NUM[0] = 0x00;
    ESD_CARD.structure.ID_NUM[1] = 0x01;
    memset(ESD_CARD.structure.SYM_NAME, 0x40, 8);
    ESD_CARD.structure.SYM_TYPE = 0x00;
    memset(ESD_CARD.structure.R_ADDR, 0x00, 3);
    ESD_CARD.structure.PADDING3 = 0x40;
    memset(ESD_CARD.structure.PR_LENGTH, 0x00, 3);
    memset(ESD_CARD.structure.PADDING4, 0x40, 40);
    memset(ESD_CARD.structure.ID_FIELD, 0x40, 8);

    RLD_CARD.structure.PADDING1 = 0x02;
    memcpy(RLD_CARD.structure.CARD_TYPE, "RLD", 3);
    RLD_CARD.structure.PADDING2 = 0x40;
    RLD_CARD.structure.ID_NUM[0] = 0x00;
    RLD_CARD.structure.ID_NUM[1] = 0x01;
    memset(RLD_CARD.structure.PADDING3, 0x40, 3);
    memcpy(RLD_CARD.structure.SIGN, "00", 2);
    memset(RLD_CARD.structure.PADDING4, 0x40, 4);
    memset(RLD_CARD.structure.OFFSET_ADDR, 0x40, 3);
    memset(RLD_CARD.structure.PADDING5, 0x40, 53);
    memset(RLD_CARD.structure.ID_FIELD, 0x40, 8);

    /*
    ***** и н и ц и а л и з а ц и я   полей буфера формирования записей TXT_CARD-типа
    *****                             для выходного объектного файла
    */

    TXT_CARD.structure.PADDING1 = 0x02;
    memcpy(TXT_CARD.structure.CARD_TYPE, "TXT", 3);
    TXT_CARD.structure.PADDING2 = 0x40;
    memset(TXT_CARD.structure.OP_ADDR, 0x00, 3);
    memset(TXT_CARD.structure.PADDING3, 0x40, 2);
    memset(TXT_CARD.structure.OP_LENGTH, 0X00, 2);
    memset(TXT_CARD.structure.PADDING4, 0x40, 2);
    TXT_CARD.structure.ID_NUM[0] = 0x00;
    TXT_CARD.structure.ID_NUM[1] = 0x01;
    memset(TXT_CARD.structure.OP_BODY, 0x40, 56);
    memset(TXT_CARD.structure.ID_FIELD, 0x40, 8);

    /*
    ***** и н и ц и а л и з а ц и я   полей буфера формирования записей END_CARD-типа
    *****                             для выходного объектного файла
    */

    END_CARD.structure.PADDING1 = 0x02;
    memcpy(END_CARD.structure.CARD_TYPE, "END", 3);
    memset(END_CARD.structure.PADDING2, 0x40, 68);
    memset(END_CARD.structure.ID_FIELD, 0x40, 8);
}



class Compiler
{

private fields:
    std::vector< std::shared_ptr< Operation > > operations;
public ctors:
    Compiler()
    {
        operations.push_back( std::shared_ptr<Operation>(new BALR()) );
        operations.push_back( std::shared_ptr<Operation>(new BCR()) );
        operations.push_back( std::shared_ptr<Operation>(new ST()) );
        operations.push_back( std::shared_ptr<Operation>(new L()) );
        operations.push_back( std::shared_ptr<Operation>(new A()) );
        operations.push_back( std::shared_ptr<Operation>(new S()) );

        operations.push_back( std::shared_ptr<Operation>(new DC()) );
        operations.push_back( std::shared_ptr<Operation>(new DS()) );

        operations.push_back( std::shared_ptr<Operation>(new END()) );
        operations.push_back( std::shared_ptr<Operation>(new EQU()) );
        operations.push_back( std::shared_ptr<Operation>(new START()) );
        operations.push_back( std::shared_ptr<Operation>(new USING()) );

    };
public methods:
    int first_iterate()
    {

        for (int i = 0; i < ASM_TEXT_LEN; i++) // iterate over ASM_TEXT
        {
            memcpy(asm_line.buffer, asm_text[i], 80);

            if (asm_line.structure.label[0] != ' ')
            {
                it_sym++;
                sym_flag = 'Y';

                memcpy(sym_table[it_sym].sym_name, asm_line.structure.label, 8);
                sym_table[it_sym].sym_addr = addr_counter;
            }

            bool op_found = false;

            for (const auto op: operations) // iterate over op_table
            {
                if (op->isOperation(asm_line.structure.op_name))
                {
                    op_found = true;
                    int ret_code = op->process1();

                    sym_flag = 'N';

                    if (ret_code == 0)
                    {
                        continue;
                    }
                    else
                    {
                        return ret_code;
                    }
                }
            }
            printf("1 addr_count=%i\n", addr_counter);

            assertf(op_found, "Operation not found: %.5s", asm_line.structure.op_name);
        }
        return -1;
    }

    int second_iterate()
    {

        for (int i = 0; i < ASM_TEXT_LEN; i++) // iterate over ASM_TEXT
        {
            memcpy(asm_line.buffer, asm_text[i], 80);

            bool op_found = false;
            for (const auto op: operations) // iterate over op_table
            {
                if (op->isOperation(asm_line.structure.op_name))
                {
                    op_found = true;
                    int ret_code = op->process2();
                    if (ret_code == 0)
                    {
                        continue;
                    }
                    else
                    {
                        return ret_code;
                    }
                }
            }

            printf("2 addr_count=%i\n", addr_counter);
            assertf(op_found, "Operation not found: %s", asm_line.structure.op_name);
        }
        return -1;
    }


};




int main(int argc, char* argv[]) 
{
    parse_args(argc, argv);

    INIT();

    Compiler comp;

    std::FILE* fp = std::fopen(gl_opts.inFileName, "r");
    assertf(fp, "Error while trying to open file: %s", gl_opts.inFileName);


    for (int i = 0; i <= ASM_TEXT_LEN; i++)
    {
        char trash[2];
        if (std::fread(asm_text[i], sizeof(char), 80, fp) != 80 || std::fread(trash, sizeof(char), 2, fp) != 2)
        {
            if (feof(fp))
            {
                break;
            }
            else
            {
                printf("%s\n", "Error reading source file");
                return -1;
            }
        }

        printf("%.80s <<\n", asm_text[i]);
    }

    std::fclose(fp);

    comp.first_iterate();
    comp.second_iterate();

    if (it_card == (GEN_OBJ_FILE()))
    {
        printf("%s\n", "успешое завершение трансляции");
    }
    else 
    {
        fprintf(stderr, "%s\n","ошибка при формировании об'ектного файла");
    }
    return 0;
}

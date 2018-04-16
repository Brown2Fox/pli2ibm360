#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <cstring>
#include <sstream>
#include <memory>
#include <cmath>
#include <algorithm>

#if defined(_WIN32) || defined(_WIN64)
#include <ncurses/ncurses.h>
#else
#include <ncurses.h>
#endif

static const uint32_t REGISTERS_AMOUNT = 16;
static const uint32_t OPERATIONS_AMOUNT = 6;
static const uint32_t OPERATION_NAME_LENGTH = 5;
static const uint32_t CARD_SIZE = 80;
static const uint32_t MEMORY_SIZE = 1024;
static const uint32_t START_ADDRESS = 0;
static const uint32_t ONE_BYTE_NUMBER_LIMIT = 100;
static const uint32_t TWO_BYTES_NUMBER_LIMIT = 10000;
static const uint32_t THREE_BYTES_NUMBER_LIMIT = 1000000;
static const uint32_t SYMBOL_LENGTH = 8;
static const uint32_t ADDR_LENGTH = 4;
static const int RR_OPERATION = 0x0;
static const int RX_OPERATION = 0x1;

static unsigned char MEMORY[MEMORY_SIZE];
static std::map<std::string, uint32_t> GLOBAL_TABLE;
static std::stringstream charToNumber;

typedef int(*implementation)(void);
int BALR_OP();
int BCR_OP();
int ST_OP();
int L_OP();
int A_OP();
int S_OP();

typedef int(*handler)(unsigned char *);
int RR(unsigned char *operation);
int RX(unsigned char *operation);

struct VM_UI
{
    WINDOW *REGISTERS;
    WINDOW *PROGRAM_TEXT;
    WINDOW *DUMP;
    WINDOW *STATUS_BAR;
    WINDOW *HELP;

    int OPERATION_TYPE;
    int DUMP_INDEX;
    int TEXT_X;
    int TEXT_Y;
};

static VM_UI UI;

struct REGISTERS_MAP
{
    unsigned long GENERAL_REGISTER[REGISTERS_AMOUNT]; // General purpose registers
    int R1; // Reg number of 1 operand in RR/RX
    int R2; // Reg number of 2 operand in RX
    int D; // Displacement in RX format
    int X; // Index register in RX format
    int B; // Base register in RX format
    unsigned long I; // Program counter
    unsigned long ADDR;
};

static REGISTERS_MAP REGISTERS_DATA;
static unsigned long FAKE_START_ADDR = 0x800000; // Displacement to imitate particular load area

unsigned long realAddress(unsigned long address) // Return real address
{
    return address - FAKE_START_ADDR;
}

unsigned long fakeAddress(unsigned long address) // Return fake address
{
    return address + FAKE_START_ADDR;
}

struct OPERATION
{
    char NAME[OPERATION_NAME_LENGTH];
    char CODE;
    int LENGTH;
    implementation IMPLEMENTATION;
    handler HANDLER;

    OPERATION(const char *name, char code, int length, implementation impl, handler opHandler)
    {
        std::memcpy(NAME, name, 5);
        CODE = code;
        LENGTH = length;
        IMPLEMENTATION = impl;
        HANDLER = opHandler;
    }
};

static const OPERATION OPERATIONS[OPERATIONS_AMOUNT] = {
    OPERATION("BALR ", '\x05', 2, BALR_OP, RR),
    OPERATION("BCR  ", '\x07', 2, BCR_OP, RR),
    OPERATION("ST   ", '\x50', 4, ST_OP, RX),
    OPERATION("L    ", '\x58', 4, L_OP, RX),
    OPERATION("A    ", '\x5A', 4, A_OP, RX),
    OPERATION("S    ", '\x5B', 4, S_OP, RX)
};

struct CARD
{
    unsigned char DATA[CARD_SIZE]; /*данные карты          */
};

struct ESD
{
    unsigned char PADDING1; /*место для кода 0x02          */
    unsigned char TYPE[3]; /*поле типа об'ектн.карты       */
    unsigned char PADDING2[10]; /*пробелы                  */
    unsigned char IDNUM[2]; /*внутр.ид-р имени прогр.      */
    unsigned char SYMBOL[SYMBOL_LENGTH]; /*имя программы   */
    unsigned char SYMTYP; /*код типа ESD-имени             */
    unsigned char OADR[3]; /*относит.адрес программы       */
    unsigned char PADDING3; /*пробелы                      */
    unsigned char DLINA[3]; /*длина программы              */
    unsigned char PADDING4[48]; /*пробелы                  */
};

struct TXT
{
    unsigned char PADDING1; /*место для кода 0x02          */
    unsigned char TYPE[3]; /*поле типа об'ектн.карты       */
    unsigned char PADDING2; /*пробел                       */
    unsigned char ADOP[3]; /*относит.адрес опреации        */
    unsigned char PADDING3[2]; /*пробелы                   */
    unsigned char DLNOP[2]; /*длина операции               */
    unsigned char PADDING4[4]; /*пробелы                   */
    unsigned char OPERND[56]; /*тело операции              */
    unsigned char PADDING5[8]; /*идентификационное поле    */
};

struct RLD
{
    unsigned char PADDING1; /*место для кода 0x02          */
    unsigned char TYPE[3]; /*поле типа об'ектн.карты       */
    unsigned char PADDING2; /*пробел                       */
    unsigned char IDNUM[2]; /*внутр.ид-р имени прогр.      */
    unsigned char PADDING3[3]; /*пробелы                   */
    unsigned char ZNAK[2]; /*знак операции                 */
    unsigned char PADDING4[4]; /*пробелы                   */
    unsigned char ADRSMESH[3]; /*адрес                     */
    unsigned char PADDING5[61]; /*идентификационное поле   */
};

struct END
{
    unsigned char PADDING1; /*место для кода 0x02          */
    unsigned char TYPE[3]; /*поле типа об'ектн.карты       */
    unsigned char PADDING2[68]; /*пробелы                  */
    unsigned char ID[8]; /*идентификационное поле          */
};

std::vector<CARD> CARDS;

void initUI()
{
    // Init curses
    initscr();
    curs_set(0);
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    start_color();

    refresh();

    if (has_colors())
    {
        init_pair(COLOR_BLUE, COLOR_WHITE, COLOR_BLUE);
        init_pair(COLOR_GREEN, COLOR_BLACK, COLOR_GREEN);
        init_pair(COLOR_RED, COLOR_WHITE, COLOR_RED);
        init_pair(COLOR_CYAN, COLOR_BLACK, COLOR_CYAN);
        init_pair(COLOR_MAGENTA, COLOR_WHITE, COLOR_MAGENTA);
    }

    // Init UI
    UI.REGISTERS = newwin(16, 12, 0, 68);
    wbkgd(UI.REGISTERS, COLOR_PAIR(COLOR_BLUE));
    UI.TEXT_X = 0;
    UI.TEXT_Y = 14;
    UI.PROGRAM_TEXT = newwin(11, 67, UI.TEXT_Y, UI.TEXT_X);
    wbkgd(UI.PROGRAM_TEXT, COLOR_PAIR(COLOR_GREEN));
    UI.DUMP = newwin(8, 67, 15, 0);
    wbkgd(UI.DUMP, COLOR_PAIR(COLOR_RED));
    UI.STATUS_BAR = newwin(1, 80, 23, 0);
    wbkgd(UI.STATUS_BAR, COLOR_PAIR(COLOR_CYAN));
    UI.HELP = newwin(1, 80, 24, 0);
    wbkgd(UI.HELP, COLOR_PAIR(COLOR_MAGENTA));
    waddstr(UI.HELP, "PgUp,PgDn,Up,Down->viewing a dump; Enter->execute next command");
    keypad(UI.HELP, TRUE);

    wrefresh(UI.REGISTERS);
    wrefresh(UI.PROGRAM_TEXT);
    wrefresh(UI.DUMP);
    wrefresh(UI.STATUS_BAR);
    wrefresh(UI.HELP);

    UI.DUMP_INDEX = (int)REGISTERS_DATA.I;
}

void updateDumpWindow()
{
    int LOCAL_DUMP_INDEX = UI.DUMP_INDEX;
    werase(UI.DUMP);
    for (int index = 0; index < 15; index++)
    {
        wprintw(UI.DUMP, "%.06lX: ", fakeAddress(LOCAL_DUMP_INDEX));
        for (int wordIndex = 0; wordIndex < 4; wordIndex++)
        {
            for (int byteIndex = 0; byteIndex < 4; byteIndex++)
                wprintw(UI.DUMP, "%.02X", MEMORY[LOCAL_DUMP_INDEX + wordIndex * 4 + byteIndex]);
            waddstr(UI.DUMP, " ");
        }

        waddstr(UI.DUMP, "/* ");
        for (int dataIndex = 0; dataIndex < 16; dataIndex++)
        {
            if (isprint(MEMORY[LOCAL_DUMP_INDEX + dataIndex]))
            {
                waddch(UI.DUMP, MEMORY[LOCAL_DUMP_INDEX + dataIndex]);
                wrefresh(UI.DUMP);
            } else
            {
                waddstr(UI.DUMP, ".");
            }
        }

        waddstr(UI.DUMP, " */\n");
        LOCAL_DUMP_INDEX += 16;
    }
    wrefresh(UI.DUMP);
}

void updateUI(int i)
{
    wprintw(UI.PROGRAM_TEXT, "%.06lX: ", fakeAddress(REGISTERS_DATA.I));
    for (int index = 0; index < OPERATIONS[i].LENGTH; ++index)
    {
        wprintw(UI.PROGRAM_TEXT, "%.02X", MEMORY[REGISTERS_DATA.I + index]);
    }

    if (UI.OPERATION_TYPE == RR_OPERATION)
    {
        // Render RR operation
        waddstr(UI.PROGRAM_TEXT, "      ");
        for (int index = 0; index < OPERATION_NAME_LENGTH; ++index)
        {
            waddch(UI.PROGRAM_TEXT, OPERATIONS[i].NAME[index]);
        }
        waddstr(UI.PROGRAM_TEXT, " ");
        wprintw(UI.PROGRAM_TEXT, "%1d, ", REGISTERS_DATA.R1);
        wprintw(UI.PROGRAM_TEXT, "%1d\n", REGISTERS_DATA.R2);
    }
    else if (UI.OPERATION_TYPE == RX_OPERATION)
    {
        // Render RX operation
        waddstr(UI.PROGRAM_TEXT, "  ");
        for (int index = 0; index < OPERATION_NAME_LENGTH; ++index)
        {
            waddch(UI.PROGRAM_TEXT, OPERATIONS[i].NAME[index]);
        }
        waddstr(UI.PROGRAM_TEXT, " ");
        wprintw(UI.PROGRAM_TEXT, "%1d, ", REGISTERS_DATA.R1);
        wprintw(UI.PROGRAM_TEXT, "X'%.3X'(", REGISTERS_DATA.D);
        wprintw(UI.PROGRAM_TEXT, "%1d, ", REGISTERS_DATA.X);
        wprintw(UI.PROGRAM_TEXT, "%1d)", REGISTERS_DATA.B);
        wprintw(UI.PROGRAM_TEXT, "        %.06lX       \n", fakeAddress(REGISTERS_DATA.ADDR));
    }

    if (UI.TEXT_Y > 4)
        mvwin(UI.PROGRAM_TEXT, UI.TEXT_Y--, UI.TEXT_X);
    else
    {
        char winstr[80];
        for (int index = 1; index < 11; index++)
        {
            mvwinnstr(UI.PROGRAM_TEXT, index, 0, winstr, 67);
            wmove(UI.PROGRAM_TEXT, index, 0);
            wclrtoeol(UI.PROGRAM_TEXT);
            mvwaddnstr(UI.PROGRAM_TEXT, index - 1, 0, winstr, 67);
            wrefresh(UI.PROGRAM_TEXT);
        }
    }
    wrefresh(UI.PROGRAM_TEXT);

    werase(UI.REGISTERS);
    for (int index = 0; index < REGISTERS_AMOUNT; index++)
    {
        (index < 10) ? waddstr(UI.REGISTERS, "R0") : waddstr(UI.REGISTERS, "R");
        wprintw(UI.REGISTERS, "%d:", index);
        wprintw(UI.REGISTERS, "%.08lX", REGISTERS_DATA.GENERAL_REGISTER[index]);
    }
    wrefresh(UI.REGISTERS);

    werase(UI.STATUS_BAR);
    waddstr(UI.STATUS_BAR, "Readiness to execute the next command with the address ");
    wprintw(UI.STATUS_BAR, "%.06lX", fakeAddress(REGISTERS_DATA.I));
    waddstr(UI.STATUS_BAR, "\n");
    wrefresh(UI.STATUS_BAR);

    werase(UI.HELP);
    waddstr(UI.HELP, "PgUp,PgDn,Up,Down->viewing a dump; Enter->execute next command");
    wrefresh(UI.HELP);

    updateDumpWindow();
}

void handleKey()
{
    int keyCode = 0;
    while (true)
    {
        keyCode = wgetch(UI.HELP);
        switch (keyCode)
        {
            case 10:
            {
                return;
            }

            case KEY_UP:
            {
                if (UI.DUMP_INDEX > 16)
                {
                    UI.DUMP_INDEX = UI.DUMP_INDEX - 16;
                }
                else
                {
                    UI.DUMP_INDEX = 0;
                }
                updateDumpWindow();
                break;
            }

            case KEY_DOWN:
            {
                if (UI.DUMP_INDEX < MEMORY_SIZE - 16)
                {
                    UI.DUMP_INDEX = UI.DUMP_INDEX + 16;
                }
                else
                {
                    UI.DUMP_INDEX = MEMORY_SIZE;
                }
                updateDumpWindow();
                break;
            }

            case KEY_PPAGE:
            {
                if (UI.DUMP_INDEX > 128)
                {
                    UI.DUMP_INDEX = UI.DUMP_INDEX - 128;
                }
                else
                {
                    UI.DUMP_INDEX = 0;
                }
                updateDumpWindow();
                break;
            }

            case KEY_NPAGE:
            {
                if (UI.DUMP_INDEX < MEMORY_SIZE - 128)
                {
                    UI.DUMP_INDEX = UI.DUMP_INDEX + 128;
                }
                else
                {
                    UI.DUMP_INDEX = MEMORY_SIZE;
                }
                updateDumpWindow();
                break;
            }
        }
    }
}

int zeroScan(const char *modulesList)
{
    std::ifstream modules(modulesList);
    if (!modules.is_open())
    {
        std::cout << "Module list" << modulesList << " don't opened\n";
        return 0;
    }
    std::string moduleName;
    std::vector<std::string> modulesNames;
    while (std::getline(modules, moduleName))
    {
        moduleName.erase(std::remove(moduleName.begin(), moduleName.end(), '\n'), moduleName.end());
        moduleName.erase(std::remove(moduleName.begin(), moduleName.end(), '\r'), moduleName.end());
        modulesNames.push_back(moduleName);
    }
    modules.close();
    char moduleCard[CARD_SIZE];
    for (auto &modulesName : modulesNames)
    {
        std::vector<ESD> esds;
        std::vector<TXT> txts;
        std::vector<RLD> rlds;
        std::vector<END> ends;
        std::ifstream module(modulesName.c_str());
        if (!module.is_open())
        {
            std::cout << "Module don't opened\n";
            return 0;
        }
        module.seekg(0, std::ios_base::end);
        int64_t moduleSize = module.tellg();
        std::unique_ptr<char> moduleData(new char[moduleSize]);
        module.seekg(0, std::ios_base::beg);
        module.read(moduleData.get(), moduleSize);
        module.close();
        for(uint32_t offset = 0; offset < moduleSize; offset += CARD_SIZE)
        {
            if (moduleSize - offset < CARD_SIZE)
            {
                break;
            }
            std::memcpy(moduleCard, moduleData.get() + offset, CARD_SIZE);
            switch (moduleCard[1])
            {
                case 'E':
                    if (moduleCard[2] == 'S')
                    {
                        esds.push_back(*((ESD *)moduleCard));
                    }
                    else if (moduleCard[2] == 'N')
                    {
                        ends.push_back(*((END *)moduleCard));
                        break;
                    }
                    else
                    {
                        return 0; // Wrong card
                    }
                    break;
                case 'T':
                    txts.push_back(*((TXT *)moduleCard));
                    break;
                case 'R':
                    rlds.push_back(*((RLD *)moduleCard));
                    break;
                default:
                    std::cout << "Wrong card\n";
                    return 0; // Wrong card
            }
        }
        CARD base{};
        for (auto &esd : esds)
        {
            std::memcpy(&base, &esd, sizeof base);
            CARDS.push_back(base);
        }
        for (auto &txt : txts)
        {
            std::memcpy(&base, &txt, sizeof base);
            CARDS.push_back(base);
        }
        for (auto &rld : rlds)
        {
            std::memcpy(&base, &rld, sizeof base);
            CARDS.push_back(base);
        }
        for (auto &end : ends)
        {
            std::memcpy(&base, &end, sizeof base);
            CARDS.push_back(base);
        }
    }
    return 1;
}

int firstScan()
{
    uint32_t absLoadAddress = 0;
    uint32_t sectionLength = 0;
    uint32_t relLoadAddress = 0;

    ESD esd{};
    uint32_t loadAddress = 0;
    for (auto &card : CARDS)
    {
        switch (card.DATA[1])
        {
            case 'E':
                {
                    if (card.DATA[2] == 'S')
                    {
                        std::memcpy(&esd, &card, sizeof esd);
                        if (esd.SYMTYP != 2) {
                            if (esd.SYMTYP == 0) {
                                absLoadAddress = START_ADDRESS + loadAddress;
                                // Actually, value resides only in DLINA[2]
                                sectionLength = (uint32_t)esd.DLINA[0] * TWO_BYTES_NUMBER_LIMIT +
                                                (uint32_t)esd.DLINA[1] * ONE_BYTE_NUMBER_LIMIT +
                                                (uint32_t)esd.DLINA[2];
                                charToNumber << std::hex << esd.OADR;
                                charToNumber >> relLoadAddress;
                            } else if (esd.SYMTYP == 1) {
                                sectionLength = (uint32_t)esd.DLINA[0] * TWO_BYTES_NUMBER_LIMIT +
                                                (uint32_t)esd.DLINA[1] * ONE_BYTE_NUMBER_LIMIT +
                                                (uint32_t)esd.DLINA[2];
                                charToNumber << std::hex << esd.OADR;
                                charToNumber >> relLoadAddress;
                                absLoadAddress = START_ADDRESS + loadAddress + relLoadAddress;
                            }
                            std::string name((char *) esd.SYMBOL, SYMBOL_LENGTH);
                            if (GLOBAL_TABLE.count(name))
                                return 0;
                            GLOBAL_TABLE[name] = absLoadAddress;
                        }
                        charToNumber.str("");
                        charToNumber.clear();
                    }
                    else if (card.DATA[2] == 'N')
                    {
                        // TODO: Round to double word
                        loadAddress += sectionLength;
                    }
                    else
                    {
                        return 0;
                    }
                }
                break;
            case 'T':
            case 'R':
                break;
            default:
                return 0;
        }
    }
    return 1;
}

int secondScan()
{
    std::map<uint32_t, uint32_t> LOCAL_TABLE;
    uint32_t absLoadAddress = 0;
    uint32_t sectionLength = 0;
    uint32_t loadAddress = 0;

    ESD esd{};
    TXT txt{};
    RLD rld{};

    uint32_t offset = 0;
    for (auto &card : CARDS)
    {
        switch (card.DATA[1])
        {
            case 'E':
                {
                    if (card.DATA[2] == 'S')
                    {
                        std::memcpy(&esd, &card, sizeof esd);
                        std::string name((char *) esd.SYMBOL, 8);
                        if (esd.SYMTYP == 0) {
                            absLoadAddress = GLOBAL_TABLE[name];
                            sectionLength = (uint32_t)esd.DLINA[0] * TWO_BYTES_NUMBER_LIMIT +
                                            (uint32_t)esd.DLINA[1] * ONE_BYTE_NUMBER_LIMIT +
                                            (uint32_t)esd.DLINA[2];
                        }
                        if (GLOBAL_TABLE.count(name)) {
                            uint32_t id = (uint32_t)esd.IDNUM[0] * ONE_BYTE_NUMBER_LIMIT + (uint32_t)esd.IDNUM[1];
                            LOCAL_TABLE[id] = GLOBAL_TABLE[name];
                        }
                        else
                        {
                            return 0;
                        }
                    }
                    else if (card.DATA[2] == 'N')
                    {
                        loadAddress += sectionLength;
                        LOCAL_TABLE.clear();
                    }
                    else
                    {
                        return 0;
                    }
                }
                break;
            case 'T':
                {
                    std::memcpy(&txt, &card, sizeof txt);
                    uint32_t operationLength = (uint32_t) txt.DLNOP[0] * ONE_BYTE_NUMBER_LIMIT +
                                               (uint32_t) txt.DLNOP[1];
                    uint32_t operationOffset = (uint32_t) txt.ADOP[0] * TWO_BYTES_NUMBER_LIMIT +
                                               (uint32_t) txt.ADOP[1] * ONE_BYTE_NUMBER_LIMIT +
                                               (uint32_t) txt.ADOP[2];
                    offset = absLoadAddress + operationOffset;
                    std::memcpy(MEMORY + offset, txt.OPERND, operationLength);
                }
                break;
            case 'R':
                {
                    std::memcpy(&rld, &card, sizeof rld);
                    uint32_t id = (uint32_t)rld.IDNUM[0] * 10 + (uint32_t)rld.IDNUM[1];
                    uint32_t operationOffset = (uint32_t) rld.ADRSMESH[0] * TWO_BYTES_NUMBER_LIMIT +
                                               (uint32_t) rld.ADRSMESH[1] * ONE_BYTE_NUMBER_LIMIT +
                                               (uint32_t) rld.ADRSMESH[2];
                    offset = absLoadAddress + operationOffset;
                    uint32_t procAbsAddr = fakeAddress(LOCAL_TABLE[id]); // L also used for data loading, so we can't use fake/realAddress in L_OP
                    unsigned char procAddrBytes[ADDR_LENGTH];
                    for (int16_t i = 3; i >= 0; --i)
                    {
                        procAddrBytes[3 - i] = (unsigned char)std::trunc((double)procAbsAddr / std::pow(ONE_BYTE_NUMBER_LIMIT, i));
                        procAbsAddr -= (uint32_t)(procAddrBytes[3 - i] * std::pow(ONE_BYTE_NUMBER_LIMIT, i));
                    }
                    std::memcpy(MEMORY + offset, procAddrBytes, ADDR_LENGTH);
                }
                break;
            default:
                return 0;
        }
    }
    return 1;
}

static unsigned long ARGUMENT = 0;

int interpreter()
{
    int result = 0;
    REGISTERS_DATA.I = 0;
    unsigned long CURRENT_INDEX = 0;
    initUI();
    while (true)
    {
        CURRENT_INDEX = REGISTERS_DATA.I;
        for (int i = 0; i < OPERATIONS_AMOUNT; ++i)
        {
            if (MEMORY[REGISTERS_DATA.I] == OPERATIONS[i].CODE)
            {
                if ((result = OPERATIONS[i].HANDLER(MEMORY + REGISTERS_DATA.I)) != 0)
                {
                    return result;
                }
                updateUI(i);
                handleKey();
                if ((result = OPERATIONS[i].IMPLEMENTATION()) != 0)
                {
                    return result;
                }
                if (REGISTERS_DATA.I == CURRENT_INDEX)
                {
                    REGISTERS_DATA.I += OPERATIONS[i].LENGTH;
                }
                break;
            }
        }
    }
}

int BALR_OP()
{
    if (REGISTERS_DATA.R1 != 0)
        REGISTERS_DATA.GENERAL_REGISTER[REGISTERS_DATA.R1] = fakeAddress(REGISTERS_DATA.I) + OPERATIONS[0].LENGTH;
    if (REGISTERS_DATA.R2 != 0) {
        REGISTERS_DATA.I = realAddress(REGISTERS_DATA.GENERAL_REGISTER[REGISTERS_DATA.R2]);
    }
    return 0;
}

int BCR_OP()
{
    int retCode = 1;
    if (REGISTERS_DATA.R1 == 15)
    {
        retCode = 0;
        if ((REGISTERS_DATA.GENERAL_REGISTER[REGISTERS_DATA.R2] != 0) && (REGISTERS_DATA.R2 != 0))
            REGISTERS_DATA.I = realAddress(REGISTERS_DATA.GENERAL_REGISTER[REGISTERS_DATA.R2]);
        else
        {
            if (REGISTERS_DATA.R2 != 0)
            {
                retCode = 1;
            }
        }
    }
    return retCode;
}

int ST_OP()
{
    unsigned char bytes[ADDR_LENGTH];
    REGISTERS_DATA.ADDR = realAddress(REGISTERS_DATA.GENERAL_REGISTER[REGISTERS_DATA.B]) +
                            REGISTERS_DATA.GENERAL_REGISTER[REGISTERS_DATA.X] +
                            REGISTERS_DATA.D;
    unsigned long value = REGISTERS_DATA.GENERAL_REGISTER[REGISTERS_DATA.R1];
    for (int16_t i = 3; i >= 0; --i)
    {
        bytes[3 - i] = (unsigned char)std::trunc((double)value / std::pow(ONE_BYTE_NUMBER_LIMIT, i));
        value -= (uint32_t)(bytes[3 - i] * std::pow(ONE_BYTE_NUMBER_LIMIT, i));
    }
    for (int i = 0; i < ADDR_LENGTH; i++)
        MEMORY[REGISTERS_DATA.ADDR + i] = bytes[i];
    return 0;
}

int L_OP()
{
    REGISTERS_DATA.ADDR = realAddress(REGISTERS_DATA.GENERAL_REGISTER[REGISTERS_DATA.B]) +
                            REGISTERS_DATA.GENERAL_REGISTER[REGISTERS_DATA.X] + REGISTERS_DATA.D;
    REGISTERS_DATA.GENERAL_REGISTER[REGISTERS_DATA.R1] = (unsigned long)(MEMORY[REGISTERS_DATA.ADDR] * THREE_BYTES_NUMBER_LIMIT +
                                                                            MEMORY[REGISTERS_DATA.ADDR + 1] * TWO_BYTES_NUMBER_LIMIT +
                                                                            MEMORY[REGISTERS_DATA.ADDR + 2] * ONE_BYTE_NUMBER_LIMIT +
                                                                            MEMORY[REGISTERS_DATA.ADDR + 3]);
    return 0;
}

int A_OP()
{
    REGISTERS_DATA.ADDR = realAddress(REGISTERS_DATA.GENERAL_REGISTER[REGISTERS_DATA.B]) +
                            REGISTERS_DATA.GENERAL_REGISTER[REGISTERS_DATA.X] + REGISTERS_DATA.D;
    ARGUMENT = (unsigned long)(MEMORY[REGISTERS_DATA.ADDR] * THREE_BYTES_NUMBER_LIMIT +
                                MEMORY[REGISTERS_DATA.ADDR + 1] * TWO_BYTES_NUMBER_LIMIT +
                                MEMORY[REGISTERS_DATA.ADDR + 2] * ONE_BYTE_NUMBER_LIMIT +
                                MEMORY[REGISTERS_DATA.ADDR + 3]);
    REGISTERS_DATA.GENERAL_REGISTER[REGISTERS_DATA.R1] = REGISTERS_DATA.GENERAL_REGISTER[REGISTERS_DATA.R1] + ARGUMENT;
    return 0;
}

int S_OP()
{
    REGISTERS_DATA.ADDR = realAddress(REGISTERS_DATA.GENERAL_REGISTER[REGISTERS_DATA.B]) +
                            REGISTERS_DATA.GENERAL_REGISTER[REGISTERS_DATA.X] + REGISTERS_DATA.D;
    ARGUMENT = (unsigned long)(MEMORY[REGISTERS_DATA.ADDR] * THREE_BYTES_NUMBER_LIMIT +
                                MEMORY[REGISTERS_DATA.ADDR + 1] * TWO_BYTES_NUMBER_LIMIT +
                                MEMORY[REGISTERS_DATA.ADDR + 2] * ONE_BYTE_NUMBER_LIMIT +
                                MEMORY[REGISTERS_DATA.ADDR + 3]);
    REGISTERS_DATA.GENERAL_REGISTER[REGISTERS_DATA.R1] = REGISTERS_DATA.GENERAL_REGISTER[REGISTERS_DATA.R1] - ARGUMENT;
    return 0;
}

int RR(unsigned char *operation)
{
    for (int i = 0; i < OPERATIONS_AMOUNT; i++)
    {
        if (operation[0] == OPERATIONS[i].CODE)
        {
            REGISTERS_DATA.R1 = operation[1] >> 4;  // R1 -> first 4 bits of second byte
            REGISTERS_DATA.R2 = operation[1] & 0xF; // R2 -> last 4 bits of second byte
            break;
        }
    }
    // Mark for UI that it was RR operation
    UI.OPERATION_TYPE = RR_OPERATION;
    return 0;
}

int RX(unsigned char *operation)
{
    for (int i = 0; i < OPERATIONS_AMOUNT; i++)
    {
        if (operation[0] == OPERATIONS[i].CODE)
        {
            REGISTERS_DATA.R1 = operation[1] >> 4; // R1 -> first 4 bits of second byte
            REGISTERS_DATA.X = operation[1] & 0xF; // X -> last 4 bits of second byte
            REGISTERS_DATA.B = operation[2] >> 4;  // B -> first 4 bits of third byte
            REGISTERS_DATA.D = ((operation[2] & 0xF) << 8) + operation[3]; // D -> last 4 bits of third byte and all bits of last byte
            REGISTERS_DATA.ADDR = REGISTERS_DATA.GENERAL_REGISTER[REGISTERS_DATA.B] +
                                    REGISTERS_DATA.GENERAL_REGISTER[REGISTERS_DATA.X] + REGISTERS_DATA.D;
            break;
        }
    }
    // Mark for UI that it was RX operation
    UI.OPERATION_TYPE = RX_OPERATION;
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2)
    {
        std::cout << "Pass path to file, which specify binaries to load\n";
        return -1;
    }
    if (!zeroScan(argv[1]))
        return 1;
    if (!firstScan())
        return 2;
    if (!secondScan())
        return 3;
    if (!interpreter())
        return 4;

    delwin(UI.REGISTERS);
    delwin(UI.STATUS_BAR);
    delwin(UI.DUMP);
    delwin(UI.HELP);
    endwin();
    return 0;
}
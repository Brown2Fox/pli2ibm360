#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <cstring>
#include <sstream>
#include <memory>
#include <cmath>

static const uint32_t REGISTERS_AMOUNT = 16;
static const uint32_t OPERATIONS_AMOUNT = 6;
static const uint32_t CARD_SIZE = 80;
static const uint32_t MEMORY_SIZE = 1024;
static const uint32_t START_ADDRESS = 0;
static const uint32_t ONE_BYTE_NUMBER_LIMIT = 100;
static const uint32_t TWO_BYTES_NUMBER_LIMIT = 10000;
static const uint32_t THREE_BYTES_NUMBER_LIMIT = 1000000;
static const uint32_t SYMBOL_LENGTH = 8;
static const uint32_t ADDR_LENGTH = 4;

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
    char NAME[5];
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

int zeroScan(const char *modulesList)
{
    std::ifstream modules(modulesList);
    std::string moduleName;
    std::vector<std::string> modulesNames;
    while (std::getline(modules, moduleName))
    {
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
        std::ifstream module(modulesName);
        if (!module.is_open())
        {
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
    return 0;
}

int main() {
    if (!zeroScan("spis.mod"))
        return 1;
    if (!firstScan())
        return 2;
    if (!secondScan())
        return 3;
    if (!interpreter())
        return 4;
    return 0;
}
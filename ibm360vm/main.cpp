#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <cstring>
#include <sstream>
#include <memory>

static const uint32_t CARD_SIZE = 80;
static const uint32_t MEMORY_SIZE = 1024;
static const uint32_t START_ADDRESS = 0;

static char MEMORY[MEMORY_SIZE];
static std::map<std::string, uint32_t> GLOBAL_TABLE;
static std::stringstream charToNumber;

struct CARD
{
    unsigned char DATA[80]; /*данные карты              */
};

struct ESD
{
    unsigned char PADDING1; /*место для кода 0x02       */
    unsigned char TYPE[3]; /*поле типа об'ектн.карты    */
    unsigned char PADDING2[10]; /*пробелы               */
    unsigned char IDNUM[2]; /*внутр.ид-р имени прогр.   */
    unsigned char SYMBOL[8]; /*имя программы            */
    unsigned char SYMTYP; /*код типа ESD-имени          */
    unsigned char OADR[3]; /*относит.адрес программы    */
    unsigned char PADDING3; /*пробелы                   */
    unsigned char DLINA[3]; /*длина программы           */
    unsigned char PADDING4[48]; /*пробелы               */
};

struct TXT
{
    unsigned char PADDING1; /*место для кода 0x02       */
    unsigned char TYPE[3]; /*поле типа об'ектн.карты    */
    unsigned char PADDING2; /*пробел                    */
    unsigned char ADOP[3]; /*относит.адрес опреации     */
    unsigned char PADDING3[2]; /*пробелы                */
    unsigned char DLNOP[2]; /*длина операции            */
    unsigned char PADDING4[4]; /*пробелы                */
    unsigned char OPERND[56]; /*тело операции           */
    unsigned char PADDING5[8]; /*идентификационное поле */
};

struct RLD
{
    unsigned char PADDING1; /*место для кода 0x02        */
    unsigned char TYPE[3]; /*поле типа об'ектн.карты     */
    unsigned char PADDING2; /*пробел                     */
    unsigned char IDNUM[2]; /*внутр.ид-р имени прогр.    */
    unsigned char PADDING3[3]; /*пробелы                 */
    unsigned char ZNAK[2]; /*знак операции               */
    unsigned char PADDING4[4]; /*пробелы                 */
    unsigned char ADRSMESH[3]; /*адрес                   */
    unsigned char PADDING5[61]; /*идентификационное поле */
};

struct END
{
    unsigned char PADDING1; /*место для кода 0x02         */
    unsigned char TYPE[3]; /*поле типа об'ектн.карты      */
    unsigned char PADDING2[68]; /*пробелы                 */
    unsigned char ID[8]; /*идентификационное поле         */
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
                                sectionLength = (uint32_t)esd.DLINA[0] * 10000 +
                                                (uint32_t)esd.DLINA[1] * 100 +
                                                (uint32_t)esd.DLINA[2];
                                charToNumber << std::hex << esd.OADR;
                                charToNumber >> relLoadAddress;
                            } else if (esd.SYMTYP == 1) {
                                sectionLength = (uint32_t)esd.DLINA[0] * 10000 +
                                                (uint32_t)esd.DLINA[1] * 100 +
                                                (uint32_t)esd.DLINA[2];
                                charToNumber << std::hex << esd.OADR;
                                charToNumber >> relLoadAddress;
                                absLoadAddress = START_ADDRESS + loadAddress + relLoadAddress;
                            }
                            std::string name((char *) esd.SYMBOL, 8);
                            if (GLOBAL_TABLE.count(name))
                                return 0;
                            GLOBAL_TABLE[name] = absLoadAddress;
                        }
                        charToNumber.str("");
                        charToNumber.clear();
                    }
                    else if (card.DATA[2] == 'N')
                    {
                        // Round to double world after
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
                            sectionLength = (uint32_t)esd.DLINA[0] * 10000 +
                                            (uint32_t)esd.DLINA[1] * 100 +
                                            (uint32_t)esd.DLINA[2];
                        }
                        if (GLOBAL_TABLE.count(name)) {
                            uint32_t id = (uint32_t)esd.IDNUM[0] * 10 + (uint32_t)esd.IDNUM[1];
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
                    uint32_t operationLength = (uint32_t) txt.DLNOP[0] * 100 +
                                               (uint32_t) txt.DLNOP[1];
                    uint32_t operationOffset = (uint32_t) txt.ADOP[0] * 10000 +
                                               (uint32_t) txt.ADOP[1] * 100 +
                                               (uint32_t) txt.ADOP[2];
                    offset = absLoadAddress + operationOffset;
                    std::memcpy(MEMORY + offset, txt.OPERND, operationLength);
                }
                break;
            case 'R':
                {
                    std::memcpy(&rld, &card, sizeof rld);
                    uint32_t id = (uint32_t)rld.IDNUM[0] * 10 + (uint32_t)rld.IDNUM[1];
                    uint32_t operationOffset = (uint32_t) rld.ADRSMESH[0] * 10000 +
                                               (uint32_t) rld.ADRSMESH[1] * 100 +
                                               (uint32_t) rld.ADRSMESH[2];
                    offset = absLoadAddress + operationOffset;
                    std::memcpy(MEMORY + offset, &LOCAL_TABLE[id], sizeof(uint32_t));
                }
                break;
            default:
                return 0;
        }
    }
    return 1;
}

int main() {
    if (!zeroScan("spis.mod"))
        return 1;
    if (!firstScan())
        return 2;
    if (!secondScan())
        return 3;
    return 0;
}
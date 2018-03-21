#define ASM_TEXT_LEN 100
#define OBJTEXT_LEN 50 /*длина об'ектн. текста   */
#define NSYM 10 /*размер табл.символов    */
#define N_PSEUDO_OPS 6 /*размер табл.псевдоопер. */
#define N_OPS 6 /*размер табл.операций    */
#include <string.h> /*вкл.строковые подпрогр. */
#include <stdlib.h> /*вкл.подпрогр.преобр.данн*/
#include <stdio.h> /*вкл.подпр.станд.вв/выв  */
#include <ctype.h> /*вкл.подпр.классиф.симв. */
#include <unistd.h>

#include "ibm360_types.h"

/*
******* Б Л О К  об'явлений статических рабочих переменных
*/

char NFIL[30] = "\x0";

unsigned char LABEL_FLAG = 'N'; /*индикатор обнаруж.метки */
int k; /*счетчик цикла           */

struct {
    const char *outFileName;
    const char *inFileName;
    int verbosity;
} globalArgs;
static const char *optString = "i:o:v";


/*
***** Б Л О К  об'явлений прототипов обращений к подпрограммам 1-го просмотра
*/

/*п р о т о т и п  обращ.к*/
int F_DC(); /*подпр.обр.пс.опер.DC    */
/*..........................................................................*/
/*п р о т о т и п  обращ.к*/
int F_DS(); /*подпр.обр.пс.опер.DS    */
/*..........................................................................*/
/*п р о т о т и п  обращ.к*/
int F_END(); /*подпр.обр.пс.опер.end_map   */
/*..........................................................................*/
/*п р о т о т и п  обращ.к*/
int F_EQU(); /*подпр.обр.пс.опер.EQU   */
/*..........................................................................*/
/*п р о т о т и п  обращ.к*/
int F_START(); /*подпр.обр.пс.опер.START */
/*..........................................................................*/
/*п р о т о т и п  обращ.к*/
int F_USING(); /*подпр.обр.пс.опер.USING */
/*..........................................................................*/
/*п р о т о т и п  обращ.к*/
int F_RR(); /*подпр.обр.опер.RR-форм. */
/*..........................................................................*/
/*п р о т о т и п  обращ.к*/
int F_RX(); /*подпр.обр.опер.RX-форм. */
/*..........................................................................*/

/*
***** Б Л О К  об'явлений прототипов обращений к подпрограммам 2-го просмотра
*/

/*п р о т о т и п  обращ.к*/
int S_DC(); /*подпр.обр.пс.опер.DC    */
/*..........................................................................*/
/*п р о т о т и п  обращ.к*/
int S_DS(); /*подпр.обр.пс.опер.DS    */
/*..........................................................................*/
/*п р о т о т и п  обращ.к*/
int S_END(); /*подпр.обр.пс.опер.end_map   */
/*..........................................................................*/
/*п р о т о т и п  обращ.к*/
int S_EQU(); /*подпр.обр.пс.опер.EQU   */
/*..........................................................................*/
/*п р о т о т и п  обращ.к*/
int S_START(); /*подпр.обр.пс.опер.START */
/*..........................................................................*/
/*п р о т о т и п  обращ.к*/
int S_USING(); /*подпр.обр.пс.опер.USING */
/*..........................................................................*/
/*п р о т о т и п  обращ.к*/
int S_RR(); /*подпр.обр.опер.RR-форм. */
/*..........................................................................*/
/*п р о т о т и п  обращ.к*/
int S_RX(); /*подпр.обр.опер.RX-форм. */
/*..........................................................................*/

/*
******* Б Л О К  об'явлений таблиц базы данных компилятора
*/

/*
******* ОБ'ЯВЛЕНИЕ структуры строки (карты) исходного текста
*/

struct ASM_MAP /*структ.карты АССЕМБЛЕРА */
{
    unsigned char LABEL[8]; /*поле метки              */
    unsigned char SPACES_1[1]; /*пробел-разделитель      */
    unsigned char OPERATION[5]; /*поле операции           */
    unsigned char SPACES_2[1]; /*пробел-разделитель      */
    unsigned char OPERAND[12]; /*поле операнда           */
    unsigned char SPACES_3[1]; /*пробел разделитель      */
    unsigned char COMMENT[52]; /*поле комментария        */
};

/*
******* НАЛОЖЕНИЕ структуры карты исх. текста на входной буфер
*/

union /*определить об'единение  */
{
    unsigned char buffer[80]; /*буфер карты.исх.текста  */
    struct ASM_MAP structure; /*наложение шабл.на буфер */
} SRC_CODE;

/*
***** СЧЕТЧИК относительного адреса (смещешия относительно базы )
*/

int ADDR_COUNTER; /*счетчик                 */

/*
***** ТАБЛИЦА символов
*/

int ITSYM = -1; /*инд.своб.стр. табл.симв.*/
struct TSYM /*структ.строки табл.симв.*/
{
    unsigned char SYM_NAME[8]; /*имя символа             */
    int SYM_ADDR; /*значение символа        */
    int SYM_LENGTH; /*длина символа           */
    char PRPER; /*признак перемещения     */
};

struct TSYM T_SYM[NSYM]; /*определение табл.симв.  */

/*
***** ТАБЛИЦА машинных операций
*/

struct TMOP /*структ.стр.табл.маш.опер*/
{
    unsigned char OP_NAME[5]; /*мнемокод операции       */
    unsigned char OP_CODE; /*машинный код операции   */
    unsigned char OP_LEN; /*длина операции в байтах */
    int (*CALLBACK)(); /*указатель на подпр.обраб*/
} OP_TABLE[N_OPS] = /*об'явление табл.маш.опер*/
    {
            // Branch And Link Register
            // BALR target_register,source_register (both register values are 0 to 15)
      { { 'B', 'A', 'L', 'R', ' ' }, '\x05', 2, F_RR },
            // Branch on Condition Register
            // BCR mask_value, branch_register (both values are 0 to 15)
      { { 'B', 'C', 'R', ' ', ' ' }, '\x07', 2, F_RR },

      { { 'S', 'T', ' ', ' ', ' ' }, '\x50', 4, F_RX }, /* Store */
      { { 'L', ' ', ' ', ' ', ' ' }, '\x58', 4, F_RX }, /* Load */
      { { 'A', ' ', ' ', ' ', ' ' }, '\x5A', 4, F_RX }, /* Add */
      { { 'S', ' ', ' ', ' ', ' ' }, '\x5B', 4, F_RX }, /* Substract */
    };

/*
***** ТАБЛИЦА псевдоопераций
*/

struct TPOP /*структ.стр.табл.пс.опeр.*/
{
    unsigned char PSEUDO_OP_NAME[5]; /*мнемокод псевдооперации */
    int (*CALLBACK)(); /*указатель на подпр.обраб*/
} PSEUDO_OP_TABLE[N_PSEUDO_OPS] = /*об'явление табл.псевдооп*/
    {
      { { 'D', 'C', ' ', ' ', ' ' }, F_DC }, /*инициализация           */
      { { 'D', 'S', ' ', ' ', ' ' }, F_DS }, /*строк                   */
      { { 'E', 'N', 'D', ' ', ' ' }, F_END }, /*таблицы                 */
      { { 'E', 'Q', 'U', ' ', ' ' }, F_EQU }, /*псевдоопераций          */
      { { 'S', 'T', 'A', 'R', 'T' }, F_START }, /*                        */
      { { 'U', 'S', 'I', 'N', 'G' }, F_USING } /*                        */
    };

/*
***** ТАБЛИЦА базовых регистров
*/

struct TBASR /*структ.стр.табл.баз.рег.*/
{
    int OFFSET; /*                        */
    char PRDOST; /*                        */
} BASE_REGS_TABLE[15] = /*                        */
    {
      { 0x00, 'N' }, /*инициализация           */
      { 0x00, 'N' }, /*строк                   */
      { 0x00, 'N' }, /*таблицы                 */
      { 0x00, 'N' }, /*базовых                 */
      { 0x00, 'N' }, /*регистров               */
      { 0x00, 'N' }, /*                        */
      { 0x00, 'N' }, /*                        */
      { 0x00, 'N' }, /*                        */
      { 0x00, 'N' }, /*                        */
      { 0x00, 'N' }, /*                        */
      { 0x00, 'N' }, /*                        */
      { 0x00, 'N' }, /*                        */
      { 0x00, 'N' }, /*                        */
      { 0x00, 'N' }, /*                        */
      { 0x00, 'N' } /*                        */
    };

/*
***** Б Л О К   об'явления массива с об'ектным текстом
*/

unsigned char OBJTEXT[OBJTEXT_LEN][80]; /*массив об'ектных карт   */
int ITCARD = 0; /*указатель текущ.карты   */

struct OP_RR /*структ.буф.опер.форм.RR */
{
    unsigned char OP_CODE; /*код операции            */
    unsigned char R1_R2; /*R1 - первый операнд     */
    /*R2 - второй операнд     */
};

union /*определить об'единение  */
{
    unsigned char buffer[2]; /*оределить буфер         */
    struct OP_RR structure; /*структурировать его     */
} RR;

struct OP_RX /*структ.буф.опер.форм.RX */
{
    unsigned char OP_CODE; /*код операции            */
    unsigned char R1_X2; /*R1 - первый операнд     */
//    short B2_D2; /*X2 - второй операнд     */
    int B2_D2;
    /*X2 - второй операнд     */
    /*B2 - баз.рег.2-го оп-да */
    /*D2 - смещен.относит.базы*/
};

union /*определить об'единение  */
{
    unsigned char buffer[4]; /*оределить буфер         */
    struct OP_RX structure; /*структурировать его     */
} RX;



union
{
    ESD structure;
    unsigned char buffer[80];
} esd_map;

union
{
    RLD structure;
    unsigned char buffer[80];
} rld_map;

union
{
    TXT structure;
    unsigned char buffer[80];
} txt_map;

union
{
    END structure;
    unsigned char buffer[80];
} end_map;



/*
******* Б Л О К  об'явлений подпрограмм, используемых при 1-ом просмотре
*/
/*подпр.обр.пс.опер.DC    */
int F_DC()
{
    /*если псевдооп.DC помеч.,*/
    if (LABEL_FLAG == 'Y')
    {
        /* если псевдооперация DC */
        /* определяет константу   */
        /* типа F, то выполнить   */
        /* следующее:             */
        if (SRC_CODE.structure.OPERAND[0] == 'F')
        {
            T_SYM[ITSYM].SYM_LENGTH = 4; /*  уст.длину симв. =  4, */
            T_SYM[ITSYM].PRPER = 'R'; /*  а,призн.перемест.='R' */
            /*  и, если ADDR_COUNTER не указ.*/
            /*  на границу слова, то: */
            if (ADDR_COUNTER % 4)
            {
                ADDR_COUNTER = (ADDR_COUNTER / 4 + 1) * 4; /*   уст.ADDR_COUNTER на гр.сл. и*/
                T_SYM[ITSYM].SYM_ADDR = ADDR_COUNTER; /*   запомн. в табл.симв. */
            }
            LABEL_FLAG = 'N'; /*  занулить LABEL_FLAG зн.'N'*/
        }
        else
        {
            return 1; /* иначе выход по ошибке  */
        }
    }
    /*если же псевдооп. не помеч*/
    /*и ADDR_COUNTER не кратен 4,то: */
    else if (ADDR_COUNTER % 4)
    {
        ADDR_COUNTER = (ADDR_COUNTER / 4 + 1) * 4; /* установ.ADDR_COUNTER на гр.сл.*/
    }

    ADDR_COUNTER = ADDR_COUNTER + 4; /*увелич.ADDR_COUNTER на 4 и     */
    return 0; /*успешно завершить подпр.*/
}
/*..........................................................................*/
/*подпр.обр.пс.опер.DS    */
int F_DS()
{
    /*если псевдооп.DC помеч.,*/
    if (LABEL_FLAG == 'Y')
    {
        /* если псевдооперация DC */
        /* определяет константу   */
        /* типа F, то выполнить   */
        /* следующее:             */
        if (SRC_CODE.structure.OPERAND[0] == 'F')
        {
            T_SYM[ITSYM].SYM_LENGTH = 4; /*  уст.длину симв. =  4, */
            T_SYM[ITSYM].PRPER = 'R'; /*  а,призн.перемест.='R' */
            /*  и, если ADDR_COUNTER не указ.*/
            /*  на границу слова, то: */
            if (ADDR_COUNTER % 4)
            {
                ADDR_COUNTER = (ADDR_COUNTER / 4 + 1) * 4; /*   уст.ADDR_COUNTER на гр.сл. и*/
                T_SYM[ITSYM].SYM_ADDR = ADDR_COUNTER; /*   запомн. в табл.симв. */
            }
            LABEL_FLAG = 'N'; /*  занулить LABEL_FLAG зн.'N'*/
        }
        else
        {
            return 1; /* иначе выход по ошибке  */
        }
    }
    /*если же псевдооп.непомеч*/
    /*и ADDR_COUNTER не кратен 4,то: */
    else if (ADDR_COUNTER % 4)
    {
        ADDR_COUNTER = (ADDR_COUNTER / 4 + 1) * 4; /* установ.ADDR_COUNTER на гр.сл.*/
    }

    ADDR_COUNTER = ADDR_COUNTER + 4; /*увелич.ADDR_COUNTER на 4 и     */
    return 0; /*успешно завершить подпр.*/
}
/*..........................................................................*/
int F_END() /*подпр.обр.пс.опер.end_map   */
{
    return (100); /*выход с призн.конца 1-го*/
    /*просмотра               */
}
/*..........................................................................*/
int F_EQU() /*подпр.обр.пс.опер.EQU   */
{
    if /*если в операнде         */
        (/*псевдооперации DC       */
            SRC_CODE.structure.OPERAND[0] == '*' /*использован симв. '*',  */
            ) /*то                      */
    { /* запомнить в табл.симв.:*/
        T_SYM[ITSYM].SYM_ADDR = ADDR_COUNTER; /*  ADDR_COUNTER в поле SYM_ADDR,   */
        T_SYM[ITSYM].SYM_LENGTH = 1; /*  1 в поле SYM_LENGTH,       */
        T_SYM[ITSYM].PRPER = 'R'; /*  'R' в пооле PRPER     */
    }
    else /*иначе запомн.в табл.симв*/
    { /* значение оп-нда пс.оп. */
        T_SYM[ITSYM].SYM_ADDR = atoi(/* DC в поле SYM_ADDR,       */
            (char*)SRC_CODE.structure.OPERAND);
        T_SYM[ITSYM].SYM_LENGTH = 1; /* 1 в поле SYM_LENGTH,        */
        T_SYM[ITSYM].PRPER = 'A'; /* 'A' в поле PRPER       */
    }
    LABEL_FLAG = 'N'; /*занул.LABEL_FLAG значен.'N' */
    return (0); /*успешное заверш.подпр.  */
}
/*..........................................................................*/
int F_START() /*подпр.обр.пс.опер.START */
{ /*ADDR_COUNTER установить равным */
    ADDR_COUNTER = /*значению операнда       */
        atoi((char*)SRC_CODE.structure.OPERAND /*псевдооперации START и, */
            );
    if (ADDR_COUNTER % 8) /*если это значение не    */
    { /*кратно 8, то сделать его*/
        ADDR_COUNTER = (ADDR_COUNTER + (8 - ADDR_COUNTER % 8)); /*кратным                 */
    } /*запомнить в табл.симв.: */
    T_SYM[ITSYM].SYM_ADDR = ADDR_COUNTER; /* ADDR_COUNTER в поле SYM_ADDR,    */
    T_SYM[ITSYM].SYM_LENGTH = 1; /* 1 в поле SYM_LENGTH,        */
    T_SYM[ITSYM].PRPER = 'R'; /* 'R' в поле PRPER       */
    LABEL_FLAG = 'N'; /*занул.LABEL_FLAG значен.'N' */
    return (0); /*успешное заверш.подпрогр*/
}
/*..........................................................................*/
int F_USING() /*подпр.обр.пс.опер.USING */
{
    return (0); /*успешное заверш.подпрогр*/
}
/*..........................................................................*/
int F_RR() /*подпр.обр.опер.RR-форм. */
{
    ADDR_COUNTER = ADDR_COUNTER + 2; /*увеличить сч.адр. на 2  */
    if (LABEL_FLAG == 'Y') /*если ранее обнар.метка, */
    { /*то в табл. символов:    */
        T_SYM[ITSYM].SYM_LENGTH = 2; /*запомнить длину маш.опер*/
        T_SYM[ITSYM].PRPER = 'R'; /*и установить призн.перем*/
    }
    return (0); /*выйти из подпрограммы   */
}
/*..........................................................................*/
int F_RX() /*подпр.обр.опер.RX-форм. */
{
    ADDR_COUNTER = ADDR_COUNTER + 4; /*увеличить сч.адр. на 4  */
    if (LABEL_FLAG == 'Y') /*если ранее обнар.метка, */
    { /*то в табл. символов:    */
        T_SYM[ITSYM].SYM_LENGTH = 4; /*запомнить длину маш.опер*/
        T_SYM[ITSYM].PRPER = 'R'; /*и установить призн.перем*/
    }
    return (0); /*выйти из подпрограммы   */
}
/*..........................................................................*/

/*
******* Б Л О К  об'явлений подпрограмм, используемых при 2-ом просмотре
*/

void S_TXT(int ARG) /*подпр.формир.txt_map-карты  */
{
    char* pTmpStr; /*рабоч.переменная-указат.*/

    pTmpStr = (char*)&ADDR_COUNTER; /*формирование поля OP_ADDR  */
    txt_map.structure.OP_ADDR[2] = (unsigned char) *pTmpStr; /*txt_map-карты в формате     */
    txt_map.structure.OP_ADDR[1] = (unsigned char) *(pTmpStr + 1); /*двоичного целого        */
    txt_map.structure.OP_ADDR[0] = '\x00'; /*в соглашениях ЕС ЭВМ    */

    if (ARG == 2) /*формирование поля OP_BODY  */
    {
        memset(txt_map.structure.OP_BODY, 64, 4);
        memcpy(txt_map.structure.OP_BODY, RR.buffer, 2); /* для RR-формата         */
        txt_map.structure.OP_LENGTH[1] = 2;
    }
    else
    {
        memcpy(txt_map.structure.OP_BODY, RX.buffer, 4); /* для RX-формата         */
        txt_map.structure.OP_LENGTH[1] = 4;
    }
    memcpy(txt_map.structure.ID_FIELD, esd_map.structure.ID_FIELD, 8); /*формиров.идентифик.поля */

    memcpy(OBJTEXT[ITCARD], txt_map.buffer, 80); /*запись об'ектной карты  */

    // txt_map 1 3 1 3 2 2 2 2 56 8
    // FIELD_1 MAP_TYPE SPACES_1 OP_ADDR SPACES_2 OP_LENGTH SPACES_3  PROGRAM_ID OP_BODY ID_FIELD
    printf("%1.1s|%3.3s|%1.1s|%3.3s|%2.2s|%2.2s|%2.2s|%2.2s|%56.56s|%8.8s\n",
            " ",
            txt_map.structure.MAP_TYPE,
            " ",
            txt_map.structure.OP_ADDR,
            "  ",
            txt_map.structure.OP_LENGTH,
            "  ",
            txt_map.structure.ID_NUM,
            txt_map.structure.OP_BODY,
            txt_map.structure.ID_FIELD);

    ITCARD += 1; /*коррекц.инд-са своб.к-ты*/
    ADDR_COUNTER = ADDR_COUNTER + ARG;
}

int S_DC() /*подпр.обр.пс.опер.DC    */
{
    char* pTmpStr; /*рабочая переменная      */

    RX.structure.OP_CODE = 0; /*занулим два старших     */
    RX.structure.R1_X2 = 0; /*байта RX.structure          */
    /* если операнд начинается*/
    /* с комбинации           */
    /* F',                    */
    if (memcmp(SRC_CODE.structure.OPERAND, "F'", 2) == 0)
    {
        /*в перем. c указат.pTmpStr   */
        /*выбираем первую лексему */
        /*операнда текущей карты  */
        /*исх.текста АССЕМБЛЕРА   */
        pTmpStr = strtok((char*)SRC_CODE.structure.OPERAND + 2, "'");

        RX.structure.B2_D2 = strtol(pTmpStr, NULL, 10); /*перевод ASCII-> int     */
        pTmpStr = (char*)&RX.structure.B2_D2; /*приведение к соглашениям*/
        swab(pTmpStr, pTmpStr, 2); /* ЕС ЭВМ                 */
    }
    else if (memcmp(SRC_CODE.structure.OPERAND, "E'", 2) == 0)
    {
        // do stuff
        pTmpStr = strtok((char*)SRC_CODE.structure.OPERAND + 2, "'");

    }
    else if (memcmp(SRC_CODE.structure.OPERAND, "A(", 2) == 0)
    {
        // do stuff
        pTmpStr = strtok((char*)SRC_CODE.structure.OPERAND + 2, ")");

    }
    else return 1; /*сообщение об ошибке     */

    S_TXT(4); /*формирование txt_map-карты  */

    return (0); /*успешн.завершение подпр.*/
}
/*..........................................................................*/
int S_DS() /*подпр.обр.пс.опер.DS    */
{

    RX.structure.OP_CODE = 0; /*занулим два старших     */
    RX.structure.R1_X2 = 0; /*байта RX.structure          */
    /* если операнд начинается*/
    /* с комбинации F'        */
    if (SRC_CODE.structure.OPERAND[0] == 'F' )
    {
        RX.structure.B2_D2 = 0; /*занулим RX.structure.B2_D2   */
    }
    else
    {
        return (1); /*сообщение об ошибке     */
    }

    S_TXT(4); /*формирование txt_map-карты  */

    return 0; /*успешно завершить подпр.*/
}
/*..........................................................................*/
int S_END() /*подпр.обр.пс.опер.end_map   */
{
    /*формирование            */
    /*идентификационнго поля  */
    /* end_map - карты            */
    memcpy(end_map.structure.ID_FIELD, esd_map.structure.ID_FIELD, 8);
    /*запись об'ектной карты  */
    /* в                      */
    /* массив                 */
    /* об'ектных              */
    /* карт * */
    memcpy(OBJTEXT[ITCARD], end_map.buffer, 80);
    ITCARD += 1; /*коррекц.инд-са своб.к-ты*/
    return (100); /*выход с призн.конца 2-го*/
    /*просмотра               */
}
/*..........................................................................*/
int S_EQU() /*подпр.обр.пс.опер.EQU   */
{
    return (0); /*успешное заверш.подпр.  */
}
/*..........................................................................*/
int S_START() /*подпр.обр.пс.опер.START */
{
    char* PTR; /*набор                   */
    char* pLabel; /*рабочих                 */
    char* pLabel_1; /*переменных              */
    int J; /*подпрограммы            */
    int RAB; /*                        */

    /*в перем. c указат.pLabel_1*/
    /*выбираем первую лексему */
    /*операнда текущей карты  */
    /*исх.текста АССЕМБЛЕРА   */
    pLabel_1 = strtok((char*)SRC_CODE.structure.LABEL, " ");
    /* все метки исх.текста в */
    /* табл. T_SYM сравниваем */
    /* со знач.перем. *pLabel_1 */
    for (J = 0; J <= ITSYM; J++)
    {
        pLabel = strtok((char*)T_SYM[J].SYM_NAME, " ");
        /* и при совпадении:      */
        if (strcmp(pLabel, pLabel_1) == 0)
        { /*  берем разность сч.адр.*/
            RAB = ADDR_COUNTER - T_SYM[J].SYM_ADDR; /*  знач.этой метки, обра-*/
            PTR = (char*)&RAB; /*  зуя длину программы в */
            swab(PTR, PTR, 2); /*  соглашениях ЕС ЭБМ, и */
            esd_map.structure.LENGTH[0] = 0; /*  записыв.ее в esd_map-карту*/
            esd_map.structure.LENGTH[1] = *PTR; /*  побайтно              */
            esd_map.structure.LENGTH[2] = *(PTR + 1); /*                        */
            ADDR_COUNTER = T_SYM[J].SYM_ADDR; /*устанавл.ADDR_COUNTER, равным  */
            /*операнду операт.START   */
            /*исходного текста        */
            PTR = (char*)&ADDR_COUNTER; /*формирование поля R_ADDR */
            esd_map.structure.R_ADDR[2] = *PTR; /*esd_map-карты в формате     */
            esd_map.structure.R_ADDR[1] = *(PTR + 1); /*двоичного целого        */
            esd_map.structure.R_ADDR[0] = '\x00'; /*в соглашениях ЕС ЭВМ    */
            /*формирование            */
            /* имени программы        */
            /*  и                     */
            /*                        */
            memcpy(esd_map.structure.SYM_NAME, pLabel, strlen(pLabel));
            /*идентификационнго поля  */
            /* esd_map - карты            */
            memcpy(esd_map.structure.ID_FIELD, pLabel, strlen(pLabel));
            /*запись об'ектной карты  */
            /* в                      */
            /* массив                 */
            /* об'ектных              */
            /* карт                   */
            memcpy(OBJTEXT[ITCARD], esd_map.buffer, 80);

            ITCARD += 1; /*коррекц.инд-са своб.к-ты*/
            return (0); /*успешное заверш.подпрогр*/
        }
    }
    return (2); /*ошибочное заверш.прогр. */
}
/*..........................................................................*/
int S_USING() /*подпр.обр.пс.опер.USING */
{
    /*набор                   */
    char* METKA; /*рабочих                 */
    char* METKA1; /*переменных              */
    char* METKA2; /*                        */
    int J; /*                        */
    int NBASRG; /*                        */
    METKA1 = strtok /*в перем. c указат.METKA1*/
        (/*выбираем первую лексему */
            (char*)SRC_CODE.structure.OPERAND, /*операнда текущей карты  */
            "," /*исх.текста АССЕМБЛЕРА   */
            );
    METKA2 = strtok /*в перем. c указат.METKA2*/
        (/*выбираем вторую лексему */
            NULL, /*операнда текущей карты  */
            " " /*исх.текста АССЕМБЛЕРА   */
            );
    if (isalpha((int)*METKA2)) /*если лексема начинается */
    { /*с буквы, то:            */

        for (J = 0; J <= ITSYM; J++) /* все метки исх.текста в */
        { /* табл. T_SYM сравниваем */
            /* со знач.перем. *METKA2 */
            METKA = strtok((char*)T_SYM[J].SYM_NAME, " ");
            if (!strcmp(METKA, METKA2)) /* и при совпадении:      */
            { /*  запоминаем значение   */
                if ((NBASRG = T_SYM[J].SYM_ADDR) <= 0x0f) /*  метки в NBASRG и в сл.*/
                    goto S_USING1; /*  NBASRG <= 0x0f идем на*/
                /*  устан.регистра базы   */
                else /* иначе:                 */
                    return (6); /*  сообщение об ошибке   */
            }
        }
        return (2); /*заверш.подпр.по ошибке  */
    }
    else /*иначе, если второй опер.*/
    { /*начинается с цифры, то: */
        NBASRG = atoi(METKA2); /* запомним его в NBASRG  */
            if ((NBASRG = T_SYM[J].SYM_ADDR) <= 0x0f) /* и,если он <= 0x0f,то:  */
            goto S_USING1; /* идем на устан.рег.базы */
        else /*иначе:                  */
            return (6); /* сообщение об ошибке    */
    }

S_USING1: /*установить базовый рег. */

    BASE_REGS_TABLE[NBASRG - 1].PRDOST = 'Y'; /* взвести призн.активн.  */
    if (*METKA1 == '*') /* если перв.оп-нд == '*',*/
    {
        BASE_REGS_TABLE[NBASRG - 1].OFFSET = ADDR_COUNTER; /* выбир.знач.базы ==ADDR_COUNTER*/
    }
    else /*иначе:                  */
    {
        for (J = 0; J <= ITSYM; J++) /* все метки исх.текста в */
        { /* табл. T_SYM сравниваем */
            /* со знач.перем. *METKA1 */
            METKA = strtok((char*)T_SYM[J].SYM_NAME, " ");
            if (!strcmp(METKA, METKA1)) /* и при совпадении:      */
            { /*  берем значение этой   */
                BASE_REGS_TABLE[NBASRG - 1].OFFSET = T_SYM[J].SYM_ADDR; /*  этой метки как базу   */
            }
        }
        return (2); /*завершение прогр.по ошиб*/
    }
    return (0); /*успешное заверш.подпрогр*/
}
/*..........................................................................*/
int S_RR() /*подпр.обр.опер.RR-форм. */
{
    char* METKA; /*набор                   */
    char* METKA1; /*рабочих                 */
    char* METKA2; /*переменных              */
    unsigned char R1R2; /*                        */
    int J; /*                        */
    RR.structure.OP_CODE = OP_TABLE[k].OP_CODE; /*формирование кода операц*/

    METKA1 = strtok /*в перем. c указат.METKA1*/
        (/*выбираем первую лексему */
            (char*)SRC_CODE.structure.OPERAND, /*операнда текущей карты  */
            "," /*исх.текста АССЕМБЛЕРА   */
            );

    METKA2 = strtok /*в перем. c указат.METKA2*/
        (/*выбираем вторую лексему */
            NULL, /*операнда текущей карты  */
            " " /*исх.текста АССЕМБЛЕРА   */
            );

    if (isalpha((int)*METKA1)) /*если лексема начинается */
    { /*с буквы, то:            */
        for (J = 0; J <= ITSYM; J++) /* все метки исх.текста в */
        { /* табл. T_SYM сравниваем */
            /* со знач.перем. *METKA1 */
            METKA = strtok((char*)T_SYM[J].SYM_NAME, " ");
            if (!strcmp(METKA, METKA1)) /* и при совпадении:      */
            { /*  берем значение этой   */
                R1R2 = T_SYM[J].SYM_ADDR << 4; /*  метки в качестве перв.*/
                goto S_RR1;
            } /*  опреранда машинной ком*/
        }
        return (2); /*сообщ."необ'явл.идентиф"*/
    }
    else /*иначе, берем в качестве */
    { /*перв.операнда машинн.ком*/
        R1R2 = atoi(METKA1) << 4; /*значен.выбр.   лексемы  */
    }


S_RR1:


    if (isalpha((int)*METKA2)) /*если лексема начинается */
    { /*с буквы, то:            */
        for (J = 0; J <= ITSYM; J++) /* все метки исх.текста в */
        { /* табл. T_SYM сравниваем */
            /* со знач.перем. *МЕТКА2 */
            METKA = strtok((char*)T_SYM[J].SYM_NAME, " ");
            if (!strcmp(METKA, METKA2)) /* и при совпадении:      */
            { /*  берем значение этой   */
                R1R2 = R1R2 + T_SYM[J].SYM_ADDR; /*  метки в качестве втор.*/
                goto SRR2; /*                        */
            } /*  опреранда машинной ком*/
        } /*                        */
        return (2); /*сообщ."необ'явл.идентиф"*/
    }
    else /*иначе, берем в качестве */
    { /*втор.операнда машинн.ком*/
        R1R2 = R1R2 + atoi(METKA2); /*значен.выбр.   лексемы  */
    }

SRR2:

    RR.structure.R1_R2 = R1R2; /*формируем опер-ды маш-ой*/
    /*команды                 */

    S_TXT(2);
    return (0); /*выйти из подпрограммы   */
}
/*..........................................................................*/
int S_RX() /*подпр.обр.опер.RX-форм. */
{
    char* METKA; /*набор                   */
    char* METKA1; /*рабочих                 */
    char* METKA2; /*переменных              */
    char* PTR; /*                        */
    int DELTA; /*                        */
    int ZNSYM; /*                        */
    int NBASRG; /*                        */
    int J; /*                        */
    int I; /*                        */
    unsigned char R1X2; /*                        */
    int B2D2; /*                        */
    RX.structure.OP_CODE = OP_TABLE[k].OP_CODE; /*формирование кода операц*/
    METKA1 = strtok /*в перем. c указат.METKA1*/
        (/*выбираем первую лексему */
            (char*)SRC_CODE.structure.OPERAND, /*операнда текущей карты  */
            "," /*исх.текста АССЕМБЛЕРА   */
            );

    METKA2 = strtok /*в перем. c указат.METKA2*/
        (/*выбираем вторую лексему */
            NULL, /*операнда текущей карты  */
            " " /*исх.текста АССЕМБЛЕРА   */
            );

    if (isalpha((int)*METKA1)) /*если лексема начинается */
    { /*с буквы, то:            */
        for (J = 0; J <= ITSYM; J++) /* все метки исх.текста в */
        { /* табл. T_SYM сравниваем */
            /* со знач.перем. *LABEL  */
            METKA = strtok((char*)T_SYM[J].SYM_NAME, " ");
            if (!strcmp(METKA, METKA1)) /* и при совпадении:      */

            { /*  берем значение этой   */
                R1X2 = T_SYM[J].SYM_ADDR << 4; /*  метки в качестве перв.*/
                goto S_RX1;
            } /*  опреранда машинной ком*/
        }
        return (2); /*сообщ."необ'явл.идентиф"*/
    }
    else /*иначе, берем в качестве */
    { /*перв.операнда машинн.ком*/
        R1X2 = atoi(METKA1) << 4; /*значен.выбр.   лексемы  */
    }


S_RX1:


    if (isalpha((int)*METKA2)) /*если лексема начинается */
    { /*с буквы, то:            */
        for (J = 0; J <= ITSYM; J++) /* все метки исх.текста в */
        { /* табл. T_SYM сравниваем */
            /* со знач.перем. *МЕТКА  */
            METKA = strtok((char*)T_SYM[J].SYM_NAME, " ");
            if (!strcmp(METKA, METKA2)) /* и при совпадении:      */
            { /*  установить нач.знач.: */
                NBASRG = 0; /*   номера базов.регистра*/
                DELTA = 0xfff - 1; /*   и его значен.,а также*/
                ZNSYM = T_SYM[J].SYM_ADDR; /*   смещен.втор.операнда */
                for (I = 0; I < 15; I++) /*далее в цикле из всех   */
                { /*рег-ров выберем базовым */
                    if (/*тот, который имеет:     */
                        BASE_REGS_TABLE[I].PRDOST == 'Y' /* призн.активности,      */
                        && /*  и                     */
                        ZNSYM - BASE_REGS_TABLE[I].OFFSET >= 0 /* значенение, меньшее по */
                        && /* величине,но наиболее   */
                        ZNSYM - BASE_REGS_TABLE[I].OFFSET < DELTA /* близкое к смещению вто-*/
                        ) /* рого операнда          */
                    {
                        NBASRG = I + 1;
                        DELTA = ZNSYM - BASE_REGS_TABLE[I].OFFSET;
                    }
                }
                if (NBASRG == 0 || DELTA > 0xfff) /*если баз.рег.не выбр.,то*/
                    return (5); /* заверш.подпр.по ошибке */
                else /*иначе                   */
                { /* сформировыать машинное */
                    B2D2 = NBASRG << 12; /* представление второго  */
                    B2D2 = B2D2 + DELTA; /* операнда в виде B2_D2   */
                    PTR = (char*)&B2D2; /* и в соглашениях ЕС ЭВМ */
                    swab(PTR, PTR, 2); /* с записью в тело ком-ды*/
                    RX.structure.B2_D2 = B2D2;
                }
                goto S_RX2; /*перех.на форм.первого   */
            } /*  опреранда машинной ком*/
        }
        return (2); /*сообщ."необ'явл.идентиф"*/
    }
    else /*иначе, берем в качестве */
    { /*втор.операнда машинн.ком*/
        return (4); /*значен.выбр.   лексемы  */
    }

S_RX2:

    RX.structure.R1_X2 = R1X2; /*дозапись перв.операнда  */

    S_TXT(4); /*формирование txt_map-карты  */
    return (0); /*выйти из подпрограммы   */
}
/*..........................................................................*/
/*подпрогр.формир.об'екн. */
/*файла                   */
int S_OBJFILE()
{ 
    /*набор рабочих           */
    /*переменных              */
    FILE* pFile;
    int RAB2; 
    /*формирование пути и име-*/
    /*ни об'ектного файла     */
    
    /*при неудачн.открыт.ф-ла */
    if ((pFile = fopen(globalArgs.outFileName, "wb")) == NULL)
    /* сообщение об ошибке    */
        return (-7);
    else /*иначе:                  */
        RAB2 = (int) fwrite(OBJTEXT, 80, (size_t) ITCARD, pFile); /* формируем тело об.файла*/

    fclose(pFile); /*закрываем об'ектный файл*/
    return (RAB2); /*завершаем  подпрограмму */
}
/*..........................................................................*/
void INIT()
{

    /*
    ***** и н и ц и а л и з а ц и я   полей буфера формирования записей esd_map-типа
    *****                             для выходного объектного файла
    */

    esd_map.structure.PADDING1 = 0x02;
    memcpy(esd_map.structure.MAP_TYPE, "ESD", 3);
    memset(esd_map.structure.PADDING2, 0x40, 10);
    esd_map.structure.ID_NUM[0] = 0x00;
    esd_map.structure.ID_NUM[1] = 0x01;
    memset(esd_map.structure.SYM_NAME, 0x40, 8);
    esd_map.structure.SYM_TYPE = 0x00;
    memset(esd_map.structure.R_ADDR, 0x00, 3);
    esd_map.structure.PADDING3 = 0x40;
    memset(esd_map.structure.LENGTH, 0x00, 3);
    memset(esd_map.structure.PADDING4, 0x40, 40);
    memset(esd_map.structure.ID_FIELD, 0x40, 8);

    /*
    ***** и н и ц и а л и з а ц и я   полей буфера формирования записей txt_map-типа
    *****                             для выходного объектного файла
    */

    txt_map.structure.PADDING1 = 0x02;
    memcpy(txt_map.structure.MAP_TYPE, "TXT", 3);
    txt_map.structure.PADDING2 = 0x40;
    memset(txt_map.structure.OP_ADDR, 0x00, 3);
    memset(txt_map.structure.PADDING3, 0x40, 2);
    memset(txt_map.structure.OP_LENGTH, 0X00, 2);
    memset(txt_map.structure.PADDING4, 0x40, 2);
    txt_map.structure.ID_NUM[0] = 0x00;
    txt_map.structure.ID_NUM[1] = 0x01;
    memset(txt_map.structure.OP_BODY, 0x40, 56);
    memset(txt_map.structure.ID_FIELD, 0x40, 8);

    /*
    ***** и н и ц и а л и з а ц и я   полей буфера формирования записей end_map-типа
    *****                             для выходного объектного файла
    */

    end_map.structure.PADDING1 = 0x02;
    memcpy(end_map.structure.MAP_TYPE, "END", 3);
    memset(end_map.structure.PADDING2, 0x40, 68);
    memset(end_map.structure.ID_FIELD, 0x40, 8);
}

/*..........................................................................*/



int main(int argc, char* argv[]) 
{
    FILE* fp = NULL;

    unsigned char ASM_TEXT[ASM_TEXT_LEN][80];

    int i, j, RAB; /* переменные цикла      */

    int opt = 0;
     
    globalArgs.inFileName = NULL;
    globalArgs.outFileName = NULL;
    globalArgs.verbosity = 0;

    while( (opt = getopt( argc, argv, optString )) != -1 ) {
        switch( opt ) {
                 
            case 'o':
                globalArgs.outFileName = optarg;
                break;

            case 'i':
                globalArgs.inFileName = optarg;
                break;
                 
            case 'v':
                globalArgs.verbosity = 1;
                break;
                 
            default:
                break;
        }
    }


    /* начальное заполнение  */
    /* буферов формирования  */
    /* записей выходного объ-*/
    /* ектного  файла        */
    INIT();


    if ((fp = fopen(globalArgs.inFileName, "r")) == NULL)
    {
        printf("%s\n", "No source file found");
        return -1;
    }


    /*
    ***** Б Л О К  инициализации массива ASM_TEXT, заменяющий иниц-ю в об'явлении
    *****          (введен как реакция на требования BORLANDC++ 2.0)
    */


    for (i = 0; i <= ASM_TEXT_LEN; i++)
    {
        char trash[2];
        if (fread(ASM_TEXT[i], sizeof(char), 80, fp)!= 80 || fread(trash, sizeof(char), 2, fp) != 2)
        {

            if (feof(fp))
                goto main1;
            else
            {
                printf("%s\n", "Error reading source file");
                return -1;
            }
        }
        printf("%.80s <<\n", ASM_TEXT[i]);
    }

    printf("%s\n", "Read buffer is overflow");
    return -1;

    

main1:

    fclose(fp);
    NFIL[strlen(NFIL) - 3] = '\x0';

    /*
    ***** К О Н Е Ц блока инициализации
    */

    /*
    ******       Н А Ч А Л О   П Е Р В О Г О  П Р О С М О Т Р А      *****
    */

    OP_TABLE[0].CALLBACK = F_RR; /*установить указатели    */
    OP_TABLE[1].CALLBACK = F_RR; /*на подпрограммы обраб-ки*/
    OP_TABLE[2].CALLBACK = F_RX; /*команд АССЕМБЛЕРА при   */
    OP_TABLE[3].CALLBACK = F_RX; /*первом просмотре        */
    OP_TABLE[4].CALLBACK = F_RX;
    OP_TABLE[5].CALLBACK = F_RX;

    PSEUDO_OP_TABLE[0].CALLBACK = F_DC; /*установить указатели    */
    PSEUDO_OP_TABLE[1].CALLBACK = F_DS; /*на подпрограммы обраб-ки*/
    PSEUDO_OP_TABLE[2].CALLBACK = F_END; /*псевдокоманд АССЕМБЛЕРА */
    PSEUDO_OP_TABLE[3].CALLBACK = F_EQU; /*при первом просмотре    */
    PSEUDO_OP_TABLE[4].CALLBACK = F_START;
    PSEUDO_OP_TABLE[5].CALLBACK = F_USING;


    /*для карт с 1 по конечную*/
    for (i = 0; i < ASM_TEXT_LEN; i++)
    { 
        /*ч-ть очередн.карту в буф*/
        memcpy(SRC_CODE.buffer, ASM_TEXT[i], 80);
        /*переход при отсутствии  */
        /*метки                   */
        /*на CONT1,               */
        if (SRC_CODE.structure.LABEL[0] == ' ')
            goto CONT1; 
        /*иначе:                  */
        /* переход к след.стр.TSYM*/
        /* устан.призн.налич.метки*/
        ITSYM++;
        LABEL_FLAG = 'Y';
        /* запомнить имя символа  */
        /* и                      */
        /* его значение(отн.адр.) */
        memcpy(T_SYM[ITSYM].SYM_NAME, SRC_CODE.structure.LABEL, 8);
        T_SYM[ITSYM].SYM_ADDR = ADDR_COUNTER;

    /*
    ***** Б Л О К  поиска текущей операции среди псевдоопераций
    */

    CONT1:

        /*для всех стр.таб.пс.опер*/
        for (j = 0; j < N_PSEUDO_OPS; j++)
        {
            /* если                   */
            /* псевдооперация         */
            /* распознана,            */
            if (!memcmp(SRC_CODE.structure.OPERATION, PSEUDO_OP_TABLE[j].PSEUDO_OP_NAME, 5))
            {
                switch (PSEUDO_OP_TABLE[j].CALLBACK()) /* уйти в подпр.обработки */
                {
                    case 0:
                        goto CONT2; /* и завершить цикл       */
                    case 1:
                        goto ERR1;
                    case 100:
                        goto CONT3;
                    default:break;
                }
            }
        }

        /*для всех стр.таб.м.опер.*/
        for (k = 0; k < N_OPS; k++)
        {
            /* если                   */
            /* машинная операция      */
            /* распознана,            */
            if (!memcmp(SRC_CODE.structure.OPERATION, OP_TABLE[k].OP_NAME, 5))
            {
                OP_TABLE[k].CALLBACK(); /* уйти в подпр.обработки */
                LABEL_FLAG = 'N'; /* снять призн.налич.метки*/
                goto CONT2; /* и завершить цикл       */
            }
        }

        goto ERR3; /*сообщ.'мнемокод нерасп.'*/


    CONT2:
        continue; /*конец цикла обработки   */
    } /*карт исх.текста         */


/*
******       Н А Ч А Л О   В Т О Р О Г О  П Р О С М О Т Р А      *****
*/

CONT3:

    OP_TABLE[0].CALLBACK = S_RR; /*установить указатели    */
    OP_TABLE[1].CALLBACK = S_RR; /*на подпрограммы обраб-ки*/
    OP_TABLE[2].CALLBACK = S_RX; /*команд АССЕМБЛЕРА при   */
    OP_TABLE[3].CALLBACK = S_RX; /*втором просмотре        */
    OP_TABLE[4].CALLBACK = S_RX;
    OP_TABLE[5].CALLBACK = S_RX;

    PSEUDO_OP_TABLE[0].CALLBACK = S_DC; /*установить указатели    */
    PSEUDO_OP_TABLE[1].CALLBACK = S_DS; /*на подпрограммы обраб-ки*/
    PSEUDO_OP_TABLE[2].CALLBACK = S_END; /*псевдокоманд АССЕМБЛЕРА */
    PSEUDO_OP_TABLE[3].CALLBACK = S_EQU; /*при втором просмотре    */
    PSEUDO_OP_TABLE[4].CALLBACK = S_START;
    PSEUDO_OP_TABLE[5].CALLBACK = S_USING;

    /*для карт с 1 по конечную*/
    for (i = 0; i < ASM_TEXT_LEN; i++)
    { /*                        */
        /*ч-ть очередн.карту в буф*/
        memcpy(SRC_CODE.buffer, ASM_TEXT[i], 80);

        /*
        ***** Б Л О К  поиска текущей операции среди псевдоопераций
        */

        for (j = 0; j < N_PSEUDO_OPS; j++) /*для всех стр.таб.пс.опер*/
        {
            /* если                   */
            /* псевдооперация         */
            /* распознана,            */
            if (!memcmp(SRC_CODE.structure.OPERATION, PSEUDO_OP_TABLE[j].PSEUDO_OP_NAME, 5))
            {
                switch (PSEUDO_OP_TABLE[j].CALLBACK()) /* уйти в подпр.обработки */
                {
                    case 0:
                        goto CONT4; /* и завершить цикл       */
                    case 100: /*уйти на формирование    */
                        goto CONT5; /*об'ектного файла        */
                    default:break;
                }
            }
        }

        for (k = 0; k < N_OPS; k++) /*для всех стр.таб.м.опер. выполнить следующее: */
        {
            /* если  машинная операция распознана */
            if (!memcmp(SRC_CODE.structure.OPERATION, OP_TABLE[k].OP_NAME, 5))
            {
                switch (OP_TABLE[k].CALLBACK()) /* уйти в подпр.обработки */
                {
                    case 0:
                        goto CONT4; /* и завершить цикл       */

                    case 2: /*выдать диагностическое  */
                        goto ERR2; /*сообщение               */

                    case 4: /*выдать диагностическое  */
                        goto ERR4; /*сообщение               */

                    case 5: /*выдать диагностическое  */
                        goto ERR5; /*сообщение               */

                    case 6: /*выдать диагностическое  */
                        goto ERR6; /*сообщение               */

                    case 7: /*выдать диагностическое  */
                        goto ERR6; /*сообщение               */
                    default:break;
                }
            }
        }

    CONT4:
        continue; /*конец цикла обработки   */
    } /*карт исх.текста         */

CONT5:
    if (ITCARD == (RAB = S_OBJFILE()))
    {
        printf("%s\n", "успешое завершение трансляции");
    }
    else 
    {
        if (RAB == -7)
            goto ERR7;
        else
            printf("%s\n","ошибка при формировании об'ектного файла");
    }
    return 0;

ERR1:
    printf("%s: %.80s\n", "ошибка формата данных", ASM_TEXT[i]);
    goto CONT6;

ERR2:
    printf("%s: %.80s\n", "необ'явленный идентификатор", ASM_TEXT[i]);
    goto CONT6;

ERR3:
    printf("%s: %.80s\n", "ошибка кода операции", ASM_TEXT[i]);
    goto CONT6;

ERR4:
    printf("%s: %.80s\n", "ошибка второго операнда",   ASM_TEXT[i]); /*выдать диагностич.сообщ.*/
    goto CONT6;

ERR5:
    printf("%s: %.80s\n", "ошибка базирования",  ASM_TEXT[i]); /*выдать диагностич.сообщ.*/
    goto CONT6;

ERR6:
    printf("%s: %.80s\n", "недопустимый номер регистра", ASM_TEXT[i]); /*выдать диагностич.сообщ.*/
    goto CONT6;

ERR7:
    printf("%s: %.80s\n", "ошибка открытия об'ектн.файла", ASM_TEXT[i]); /*выдать диагностич.сообщ.*/
    goto CONT6;


CONT6:
    printf("%s%d\n", "ошибка в карте N ", i + 1); /*выдать диагностич.сообщ.*/

    return 0;
} /*конец main-программы    */

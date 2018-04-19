%{

#include <stdio.h>
#include <string.h>

#define YYERROR_VERBOSE 1
#define YYSTYPE char*

int yylineno;
FILE *yyin;

YYSTYPE yylval;

#define MAX_LENGTH_ASS_PROG 50

#define MAX_LENGTH_DCL_PART 20

#define MAX_LENGTH_IMP_PART 20

char s1[80];

char Prolog[3][80];
char Epilog[80];

char AssProg[MAX_LENGTH_ASS_PROG][80];
int pAssProg;

char DclPart[MAX_LENGTH_DCL_PART][80];
int pDclPart;

char ImpPart[MAX_LENGTH_IMP_PART][80];
int pImpPart;

char AssProgName[9];
char ErrorMessage[100];

int IsDclName( char* pName, int length) 
{
    int i, j;
    char etalon[9];

    for (i = 0; i < pDclPart; i++) 
    {
        j = 0;
        while (DclPart[i][j] != ' ')  
        {
            etalon[j++] = DclPart[i][j];
        }
        etalon[j] = 0;

        if (length != j) continue;

        if (memcmp(&DclPart[i][0], pName, length)) 
        {
            continue;
        }
        else 
        {
            return 0;
        }
    }

    return 1;
}

void yyerror(const char *str) 
{
    fprintf(stderr, "\n error: string N  %u, %s\n", yylineno, str);
}

void pro();
void odi(char *tpe, char *bits, char *lit);
void odr(char *tpe, char *bits);
void opr(char *pr_name);
int  oen(char *pr_name);
int  opa(char *ipe);
void expr_liter(char *liter);
int  expr_name(char *name);
void expr_oper_liter(char *oper, char *lit);
int  expr_oper_name(char *oper, char *name);

%}


%debug
%verbose
%token NAME PROC OPTIONS MAIN END DCL BIN FIXED NUM INIT 
%token FLOAT DECIMAL
%token DATATYPE
%left OPER
%start pro

%%

pro: opr tel oen { pro(); }
     ;

tel: dec imp
     ;

dec: odc
    | dec odc
     ;

odc: odi
    | odr
     ;

odi:  DCL name BIN FIXED '(' bits ')' INIT '(' liter ')' ';' { odi($2, $6, $10); }
     ;

odr:  DCL name BIN FIXED '(' bits ')' ';'                  { odr($2, $6); }
     ;

name: NAME { $$ = $1; }
     ;

bits: NUM { $$ = $1; }
     ;

liter: NUM { $$ = $1; }
     ;

opr: NAME ':' PROC OPTIONS '(' MAIN ')' ';'             { opr($1); }
     ;

oen: END NAME ';'                                       { if ( oen($2) ) YYABORT; }
     ;

imp: opa
    | imp opa
     ;

opa: name '=' expr ';'                                     { if ( opa($1) ) YYABORT; }
     ;

expr: liter                                                 { expr_liter($1); }
    | name                                                { if ( expr_name($1) ) YYABORT;}
    | expr OPER liter                                        { expr_oper_liter($2, $3); }
    | expr OPER name                                        { if ( expr_oper_name($2, $3) ) YYABORT; }
     ;
%%

/*
***************************************************************************************************************
*                   Начало библиотеки семантических программ                                                  *
***************************************************************************************************************
*/

/*
...............................................................................................................
. Программа void pro()                                                                                        .
. Соединяет в массиве AssProg вместе следующие части ассемблеровского эквивалента:                            .
.  - пролог (из массива Prolog),                                                                              ,
.  - императивную часть (из массива ImpPart),                                                                 .
.  - декларативную часть (из массива DeclPart),                                                               .
.  - эпилог (из массива epilog).                                                                              .
. Распечатывает результат                                                                                     .
,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
*/

void pro()
{
    memcpy(&AssProg[pAssProg++][0], &Prolog[0][0], 80);
    memcpy(&AssProg[pAssProg++][0], &Prolog[1][0], 80);
    memcpy(&AssProg[pAssProg++][0], &Prolog[2][0], 80);

    for (int i = 0; i < pImpPart; i++) 
    {
        memcpy(&AssProg[pAssProg++][0], &ImpPart[i][0], 80);
    }

    for (int i = 0; i < pDclPart; i++) 
    {
        memcpy(&AssProg[pAssProg++][0], &DclPart[i][0], 80);
    }

    memcpy(&AssProg[pAssProg++], &Epilog[0], 80);

    if (yydebug) 
    {
        printf("\nResult of Generation\n");
        for (int i = 0; i < pAssProg; i++) 
        {
            printf("\n");
            for (int j = 0; j < 79; j++) 
            {
                printf("%c", AssProg[i][j]);
            }
        }


        printf("\nDclPart");
        for (int i = 0; i < pDclPart; i++) 
        {
            printf("\n");
            for (int j = 0; j < 79; j++) 
            {
                printf("%c", DclPart[i][j]);
            }
        }

        printf("\n");
        printf("\nImpPart");

        for (int i = 0; i < pImpPart; i++) 
        {
            printf("\n");
            for (int j = 0; j < 79; j++) 
            {
                printf("%c", ImpPart[i][j]);
            }
        }

        printf("\n");
    }
}

/*
...............................................................................................................
. Программа void odi(char *name, char *bits, char *lit)                                                         .
.                                                                                                             .
. Формирует и дописывает в свободную строку массива DclPart ассемблеровский эквивалент оператора ЯВУ,         .
. который  объявляет и инициализирует именованный объект (dcl с инициализацией).                              .
.                                                                                                             .
. Параметры: name (имя переменной), bits (разрядность), liter (инициализирующее значение).                        .
,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
*/

void odi(char *name, char *bits, char *liter) 
{
    memset(&s1[0], ' ', 80);
    memcpy(&s1[0], name, strlen(name));
    memcpy(&s1[9], "DC", 2);

    s1[15] = memcmp(bits, "31", 2) ? 'H' : 'F';
    s1[16]='\'';
    
    memcpy(&s1[17], liter, strlen(liter));
 
    s1[17+strlen(liter)]='\'';
 
    memcpy(&s1[30], "Variable declaration with initialization", 40);
    memcpy(&DclPart[pDclPart][0], &s1[0], 80);
 
    pDclPart++;
}

/*
...............................................................................................................
. Программа void odr(char *name, char *bits)                                                                    .
.                                                                                                             .
. Формирует и дописывает в свободную строку массива DclPart ассемблеровский эквивалент оператора ЯВУ,         .
. который  объявляет именованный объект (dcl без инициализации).                                              .
.                                                                                                             .
. Параметры: name (имя переменной), bits (разрядность).                                                         .
.                                                                                                             .
,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
*/

void odr(char *name, char *bits) 
{
    memset(&s1[0], ' ', 80);
    memcpy(&s1[0], name, strlen(name));
    memcpy(&s1[9], "DS", 2);
    
    s1[15] = memcmp(bits, "31", 2) ? 'H' : 'F';

    memcpy(&s1[30], "Variable declaration without initialization", 43);

    memcpy(&DclPart[pDclPart][0], &s1[0], 80);
    pDclPart++;
}

/*
...............................................................................................................
. Программа void opr(char *pr_name)                                                                           .
.                                                                                                             .
. Формирует и дописывает в свободную строку массива Prolog  ассемблеровский эквивалент оператора ЯВУ,         .
. который  объявляет пролог программы на ЯВУ (proc options ...).                                              .
.                                                                                                             .
. Параметры: pr_name (имя программы).                                                                         .
.                                                                                                             .
,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
*/


void opr(char *pr_name) 
{
    memset(&s1[0], ' ', 80);
    memcpy(&s1[0], pr_name, strlen(pr_name));
    memcpy(&s1[9], "START 0", 7);
    memcpy(&s1[30], "Programm start", 14);
    memcpy(&Prolog[0][0], &s1[0], 80);

    memset(&s1[0], ' ', 80);
    memcpy(&s1[9], "BALR  RBASE,0", 13);
    memcpy(&s1[30], "Base initialization", 19);
    memcpy(&Prolog[1][0], &s1[0], 80);


    memset(&s1[0], ' ', 80);
    memcpy(&s1[9], "USING *,RBASE", 13);
    memcpy(&s1[30], "Base declaration", 16);
    memcpy(&Prolog[2][0], &s1[0], 80);

    memcpy(&AssProgName[0], pr_name, strlen(pr_name));
}

/*
...............................................................................................................
. Программа int oen(char *pr_name)                                                                            .
.                                                                                                             .
. Формирует и дописывает в массив Epilog ассемблеровский эквивалент оператора ЯВУ,                            .
. который  объявляет эпилог программы на ЯВУ (end ...).                                                       .
.                                                                                                             .
. Параметры: pr_name (имя программы).                                                                         .
.                                                                                                             .
. Возвращаемый результат 0 (если pr_name совпадает с pr_name пролога) и 1 (в противном случае)                .
,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
*/

int oen(char *pr_name) 
{
    if (!memcmp(&Prolog[0][0], pr_name, strlen(pr_name)))
    {
        memset(&s1[0], ' ', 80);
        memcpy(&s1[9], "END", 3);
        memcpy(&s1[30], "Programm end", 12);
        memcpy(&Epilog[0], &s1[0], 80);

        memset(&s1[0], ' ', 80);
        memcpy(&s1[9], "BCR   15,RVIX", 13);
        memcpy(&s1[30], "Return from programm", 20);

        memcpy(&ImpPart[pImpPart][0], &s1[0], 80);
        pImpPart++;
        return 0; 
    }
    else 
    {
        strcpy(&ErrorMessage[0], " invalid identificator ");
        strcat(&ErrorMessage[0], pr_name);
        strcat(&ErrorMessage[0], " ");
        strcat(&ErrorMessage[0], "in oen\n");
        yyerror(&ErrorMessage[0]);
        return 1;
    }
}

/*
...............................................................................................................
. Программа int opa(char *name)                                                                                .
.                                                                                                             .
. Формирует и дописывает в в свободную строку массива ImpPart ассемблеровский эквивалент оператора ЯВУ,       .
. который  присваивает переменной name значение вычисленного арифметического выражения, находящегося в регистре.
. RRAB  (имя перемеменной =  ...).                                                                            .
.                                                                                                             .
. Параметры: pr_name (имя программы).                                                                         .
.                                                                                                             .
. Возвращаемый результат 0 (если ipe объявлена ранее в операторе dcl) и 1 (в противном случае)                .
,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
*/

int opa(char *name) 
{
    if (IsDclName(name, strlen(name)))
    {
        strcpy(&ErrorMessage[0], " invalid identificator ");
        strcat(&ErrorMessage[0], name);
        strcat(&ErrorMessage[0], " ");
        strcat(&ErrorMessage[0], "in left part of opa\n");
        yyerror(&ErrorMessage[0]);
        return 1;
    }
    memset(&s1[0], ' ', 80);
    memcpy(&s1[9], "ST", 2);
    memcpy(&s1[15], "RRAB,", 5);
    memcpy(&s1[20], name, strlen(name));
    memcpy(&s1[30], "Result storage", 14);

    memcpy(&ImpPart[pImpPart][0], &s1[0], 80);
    pImpPart++;

    return 0;
}

/*
...............................................................................................................
. Программа void expr_liter(char *liter)                                                                           .
.                                                                                                             .
. Формирует и дописывает в в свободную строку массива ImpPart ассемблеровский эквивалент арифметического      .
. выражения - правой части оператора присваивания ЯВУ (имя переменной = аифм.выражение), когда арифметическое .
. выражение представлено литералом.                                                                           .
.                                                                                                             .
. Параметры: liter (литерал).                                                                                   .
.                                                                                                             .
.....,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
*/

void expr_liter(char *liter) 
{
    memset(&s1[0], ' ', 80);
    memcpy(&s1[9], "L", 1);
    memcpy(&s1[15], "RRAB,=F\'", 8);
    memcpy(&s1[23], liter, strlen(liter));
    memcpy(&s1[23+strlen(liter)], "\'", 1);
    memcpy(&s1[30], "Literal loading", 15);

    memcpy(&ImpPart[pImpPart][0], &s1[0], 80);
    pImpPart++;
}

/*
...............................................................................................................
. Программа int expr_name(char *name)                                                                            .
.                                                                                                             .
. Формирует и дописывает в в свободную строку массива ImpPart ассемблеровский эквивалент арифметического      .
. выражения - правой части оператора присваивания ЯВУ (имя переменной = аифм.выражение), когда арифметическое .
. выражение представлено именем переменной name.                                                               .
.                                                                                                             .
. Параметры: ipe (имя переменной).                                                                            .
.                                                                                                             .
. Возвращаемый результат 0 (если ipe объявлена ранее в операторе dcl) и 1 (в противном случае)                .
.                                                                                                             .
.....,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
*/

int  expr_name(char *name) 
{
    if (IsDclName(name, strlen(name)))
    {
        strcpy(&ErrorMessage[0], " invalid identificator ");
        strcat(&ErrorMessage[0], name);
        strcat(&ErrorMessage[0], " ");
        strcat(&ErrorMessage[0], "in avi of opa\n");
        yyerror(&ErrorMessage[0]);
        return 1;
    }
    memset(&s1[0], ' ', 80);
    memcpy(&s1[9], "L", 1);
    memcpy(&s1[15], "RRAB,", 5);
    memcpy(&s1[20], name, strlen(name));
    memcpy(&s1[30], "Variable value loading", 22);

    memcpy(&ImpPart[pImpPart][0], &s1[0], 80);
    pImpPart++;

    return 0;
}

/*
...............................................................................................................
. Программа void expr_oper_liter(char *lit)                                                                   .
.                                                                                                             .
. Формирует и дописывает в в свободную строку массива ImpPart ассемблеровский эквивалент арифметического      .
. выражения - правой части оператора присваивания ЯВУ (имя переменной = аифм.выражение), когда арифметическое .
. выражение представлено как леворекурсивное с литералом справа.                                              .
.                                                                                                             .
. Параметры: sign (знак + или -) liter (литерал).                                                                .
.                                                                                                             .
.                                                                                                             .
.....,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
*/

void expr_oper_liter(char *oper, char *liter) 
{
    memset(&s1[0], ' ', 80);
    if (!memcmp(oper, "+", 1)) 
    {
        memcpy(&s1[9], "A", 1);
        memcpy(&s1[30], "Literal\'s value adding", 22);
    }
    if (!memcmp(oper, "-", 1)) 
    {
        memcpy(&s1[9], "S", 1);
        memcpy(&s1[30], "Literal\'s value substracting", 28);
    }
    memcpy(&s1[15], "RRAB,=F\'", 8);
    memcpy(&s1[23], liter, strlen(liter));
    memcpy(&s1[23+strlen(liter)], "\'", 1);

    memcpy(&ImpPart[pImpPart][0], &s1[0], 80);
    pImpPart++;
}

/*
...............................................................................................................
. Программа int expr_oper_name(char *znk, char *ipe)                                                         .
.                                                                                                             .
. Формирует и дописывает в в свободную строку массива ImpPart ассемблеровский эквивалент арифметического      .
. выражения - правой части оператора присваивания ЯВУ (имя переменной = аифм.выражение), когда арифметическое .
. выражение представлено как леворекурсивное с именем переменной справа.                                      .
.                                                                                                             .
. Параетры: lit (литерал).                                                                                    .
.                                                                                                             .
. Возвращаемый результат 0 (если ipe объявлена ранее в операторе dcl) и 1 (в противном случае)                .
.                                                                                                             .
.....,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
*/

int  expr_oper_name(char *sign, char *name) 
{
    if (IsDclName(name, strlen(name)))
    {
        strcpy(&ErrorMessage[0], " invalid identificator ");
        strcat(&ErrorMessage[0], name);
        strcat(&ErrorMessage[0], " ");
        strcat(&ErrorMessage[0], "in avi of opa\n");
        yyerror(&ErrorMessage[0]);
        return 1;
    }
    memset(&s1[0], ' ', 80);
    if (!memcmp(sign, "+", 1)) 
    {
        memcpy(&s1[9], "A", 1);
        memcpy(&s1[30], "Variable\'s value adding", 23);
    }
    if (!memcmp(sign, "-", 1)) 
    {
        memcpy(&s1[9], "S", 1);
        memcpy(&s1[30], "Variable\'s value substracting", 29);
    }
    memcpy(&s1[15], "RRAB,", 5);
    memcpy(&s1[20], name, strlen(name));

    memcpy(&ImpPart[pImpPart][0], &s1[0], 80);
    pImpPart++;

    return 0;
}

/*
***************************************************************************************************************
*                   Конец  библиотеки семантических программ                                                  *
***************************************************************************************************************
*/

/*
...............................................................................................................
. Программа int yywrap()                                                                                      .
.                                                                                                             .
. Вызывается по достижении EOF входного потока                                                                .
.                                                                                                             .
. Возвращаемый результат 0 (если работа д.б. продолжена) и 1 (в противном случае)                             .
.                                                                                                             .
.....,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
*/

int yywrap() 
{
    return 1;
}

/*
...............................................................................................................
. Программа int main()                                                                                        .
.                                                                                                             .
. Главная программа. Начинает работу, проводит начальную инициализацию, обращается к парсеру (yyparse) и      .
. при отсутствии ошибок разгружает результат компиляции из массива AssProg в выходной файл Results.ass,       .
. а в случае преждевременного завершения парсера - выдает аварийное сообщение                                 .
.                                                                                                             .
. Возвращаемый результат 0 (компиляция успешна) и 1 (в противном случае)                                      .
.                                                                                                             .
.....,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

struct globalArgs_t {
    const char *outFileName;
    const char *inFileName;
    int verbosity;
} globalArgs;
 
static const char *optString = "i:o:v";


int main(int argc, char* argv[]) 
{
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

    pAssProg = 0;
    memset(&DclPart[0][0], ' ', 80);
    memcpy(&DclPart[0][0], "RBASE    EQU   5", 16);
    memset(&DclPart[1][0], ' ', 80);
    memcpy(&DclPart[1][0], "RVIX     EQU   14", 17);
    memset(&DclPart[2][0], ' ', 80);
    memcpy(&DclPart[2][0], "RRAB     EQU   3", 16);
    pDclPart = 3;
    pImpPart = 0;
    yydebug = globalArgs.verbosity;

    yyin = fopen(globalArgs.inFileName, "r");

    if (!yyparse()) 
    {
//        int fd = open(globalArgs.outFileName, O_CREAT|O_WRONLY|O_TRUNC, S_IWUSR);
        FILE* fd = fopen(globalArgs.outFileName, "w");
//        if (fd < 0)
//        {
//            printf("\n*** Error trying to create Result.ass file: check your write permissions");
//            return -1;
//        }


        for (int i = 0; i < pAssProg; i++)
        {
            fprintf(fd, "%.80s|\n", &AssProg[i][0]);
//            fprintf(fd, "%s\n", "dsadsdadasd");
            // if (80 != write(fd, &AssProg[i][0], 80))
            // {
                // printf("\n*** Error during writing Result.ass file\n");
                // break;
            // }
        }
        
        close(fd);

        printf("\n*** Compilation is successfull\n");       
        return 0;
    }
    
    close(yyin);
    printf("\n*** Compilation is not successfull\n");
    return -1;
    
}

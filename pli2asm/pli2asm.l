%{

#include <stdio.h>
#include <string.h>
#define YYSTYPE char *
#include "./pli2asm.parser.h"
extern YYSTYPE yylval;

%}

%option yylineno

%%

":" {
    if (yydebug) printf("COLON TERM: %s\n", yytext);
    return ':';
}

"(" {
    if (yydebug) printf("OPEN_PARENTHESIS TERM: %s\n", yytext);
    return '(';
}

")" {
    if (yydebug) printf("CLOSE_PARENTHESIS TERM: %s\n", yytext);
    return ')';
}

";" {
    if (yydebug) printf("SEMICOLON TERM: %s\n", yytext);
    return ';';
}

"=" {
    if (yydebug) printf("EQUALS TERM: %s\n", yytext);
    return '=';
}

[+-] {
    if (yydebug) printf("OPER TERM: %s\n", yytext);
    yylval = strdup(yytext); 
    return OPER;
}

[a-zA-Z][a-zA-Z0-9]* {
    if (yydebug) printf("WORD TERM: %s\n", yytext);
    if (!memcmp(yytext,"proc", 4)) return PROC;
    if (!memcmp(yytext,"options", 7)) return OPTIONS;
    if (!memcmp(yytext,"main", 4)) return MAIN;
    if (!memcmp(yytext,"end", 3)) return END;
    if (!memcmp(yytext,"dcl", 3)) return DCL;
    if (!memcmp(yytext,"bin", 3)) return BIN;
    if (!memcmp(yytext,"fixed", 5)) return FIXED;
    if (!memcmp(yytext,"init", 4)) return INIT;
    if (!memcmp(yytext,"float", 5)) return FLOAT;

    yylval = strdup(yytext); 
    return NAME;
}

[0-9][0-9]* {
    if (yydebug) printf("INTEGER TERM: %s\n", yytext);
    yylval = strdup(yytext); 
    return NUM;
}

[ \t\n]+ {}

%%

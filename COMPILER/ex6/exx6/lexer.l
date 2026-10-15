%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "y.tab.h"
%}

%option noyywrap
%option yylineno

DIGIT   [0-9]
LETTER  [A-Za-z_]

%%

[ \t\r\n]+                    ;
"//".*                        ;
"/*"([^*]|\*+[^*/])*\*+"/"    ;

"for"                         { return FOR;   }
"while"                       { return WHILE; }
"if"                          { return IF;    }
"else"                        { return ELSE;  }
"int"|"float"|"double"|"char"|"long" { return TYPE; }

"&&"|"and"                    { return AND; }
"||"|"or"                     { return OR;  }
"not"                         { return NOT; }

"=="                          { return EQ; }
"!="                          { return NE; }
"<="                          { return LE; }
">="                          { return GE; }
"++"                          { return INC; }
"--"                          { return DEC; }
"+="                          { return PLUSEQ;  }
"-="                          { return MINUSEQ; }
"*="                          { return MULEQ;   }
"/="                          { return DIVEQ;   }

{LETTER}({LETTER}|{DIGIT})*   { yylval.str = strdup(yytext); return ID;  }
{DIGIT}+(\.{DIGIT}+)?         { yylval.str = strdup(yytext); return NUM; }

[+\-*/%<>=!(){};]             { return yytext[0]; }

.                             { fprintf(stderr, "Line %d: unknown character '%s'\n", yylineno, yytext); }
%%
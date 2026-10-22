%{
#include <stdlib.h>
#include <string.h>
#include "y.tab.h"
%}
%option noyywrap

%%
[ \t\r\n]+                    ;
"for"                         { return FOR; }
"&&"|"and"                    { return AND; }
"||"|"or"                     { return OR;  }
"!"|"not"                      { return NOT; }
"=="                          { return EQ;  }
"!="                          { return NE;  }
"<="                          { return LE;  }
">="                          { return GE;  }
"++"                          { return INC; }
"--"                          { return DEC; }
[A-Za-z_][A-Za-z0-9_]*        { yylval.str = strdup(yytext); return ID;  }
[0-9]+                        { yylval.str = strdup(yytext); return NUM; }
[-+*/%<>=(){};]               { return yytext[0]; }
.                             ;
%%

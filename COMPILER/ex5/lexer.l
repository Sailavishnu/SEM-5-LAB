%option noyywrap

%{
#include <stdio.h>
#include <stdlib.h>
#include "y.tab.h"
%}

DIGIT   [0-9]
NUMBER  ({DIGIT}+(\.{DIGIT}*)?)|(\.{DIGIT}+)

%%

[ \t\r]+        { /* skip spaces and tabs */ }

{NUMBER}        { yylval.val = atof(yytext); return NUMBER; }

"+"             { return '+'; }
"-"             { return '-'; }
"*"             { return '*'; }
"/"             { return '/'; }
"^"             { return '^'; }
"("             { return '('; }
")"             { return ')'; }

\n              { return '\n'; }

.               { printf("Invalid character: %s\n", yytext); return yytext[0]; }

%%

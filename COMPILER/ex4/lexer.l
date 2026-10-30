%{
#include "y.tab.h"
int at_eof = 0;
%}
%option noyywrap yylineno
%s OP
%%
[ \t\n\r]+              { }
"for"                   { BEGIN(INITIAL); return FOR; }
"int"                   { BEGIN(INITIAL); return INT; }
"double"                { BEGIN(INITIAL); return DOUBLE; }
"boolean"               { BEGIN(INITIAL); return BOOLEAN_TYPE; }
"true"|"false"          { BEGIN(OP); return BOOLEAN; }
[0-9]+(\.[0-9]+)?       { BEGIN(OP); return NUMBER; }
[a-zA-Z_][a-zA-Z0-9_]*  { BEGIN(OP); return IDENTIFIER; }
<INITIAL>"++"           { return PRE_INC; }
<INITIAL>"--"           { return PRE_DEC; }
<INITIAL>"+"            { return UPLUS; }
<INITIAL>"-"            { return UMINUS; }
<OP>"++"                { BEGIN(INITIAL); return POST_INC; }
<OP>"--"                { BEGIN(INITIAL); return POST_DEC; }
<OP>"+"                 { BEGIN(INITIAL); return BPLUS; }
<OP>"-"                 { BEGIN(INITIAL); return BMINUS; }
"&&"|"||"               { BEGIN(INITIAL); return LOGICAL; }
"!"                     { BEGIN(INITIAL); return NOT; }
"<="|">="|"<"|">"|"=="|"!=" { BEGIN(INITIAL); return RELOP; }
"="                     { BEGIN(INITIAL); return ASSIGN; }
"("                     { BEGIN(INITIAL); return LP; }
")"                     { BEGIN(OP); return RP; }
"{"                     { BEGIN(INITIAL); return LB; }
"}"                     { BEGIN(INITIAL); return RB; }
";"                     { BEGIN(INITIAL); return SEMI; }
<<EOF>>                 { at_eof = 1; return 0; }
.                       { return UNKNOWN; }
%%
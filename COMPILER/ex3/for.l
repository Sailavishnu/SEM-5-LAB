%{
#include <stdio.h>
%}

%option noyywrap

%s OP

DIGIT   [0-9]
NUMBER  {DIGIT}+(\.{DIGIT}+)?
ID      [a-zA-Z_][a-zA-Z0-9_]*

%%

[ \t\n\r]+          { }

"for"                { BEGIN(INITIAL); printf("< keyword : for >\n"); }
"int"                { BEGIN(INITIAL); printf("< keyword : int >\n"); }
"double"             { BEGIN(INITIAL); printf("< keyword : double >\n"); }
"boolean"            { BEGIN(INITIAL); printf("< keyword : boolean >\n"); }

"true"|"false"       { BEGIN(OP); printf("< boolean : %s >\n", yytext); }

{NUMBER}             { BEGIN(OP); printf("< number : %s >\n", yytext); }
{ID}                 { BEGIN(OP); printf("< identifier : %s >\n", yytext); }

<INITIAL>"++"        { printf("< pre-increment : ++ >\n"); }
<INITIAL>"--"        { printf("< pre-decrement : -- >\n"); }
<INITIAL>"+"         { printf("< unary plus : + >\n"); }
<INITIAL>"-"         { printf("< unary minus : - >\n"); }

<OP>"++"             { BEGIN(INITIAL); printf("< post-increment : ++ >\n"); }
<OP>"--"             { BEGIN(INITIAL); printf("< post-decrement : -- >\n"); }
<OP>"+"              { BEGIN(INITIAL); printf("< binary plus : + >\n"); }
<OP>"-"              { BEGIN(INITIAL); printf("< binary minus : - >\n"); }

"*"                  { BEGIN(INITIAL); printf("< multiplication : * >\n"); }
"/"                  { BEGIN(INITIAL); printf("< division : / >\n"); }

"<="|">="|"=="|"!="|"<"|">" {
                       BEGIN(INITIAL);
                       printf("< relational operator : %s >\n", yytext);
                     }

"&&"|"||"            { BEGIN(INITIAL); printf("< logical operator : %s >\n", yytext); }
"!"                  { BEGIN(INITIAL); printf("< logical NOT : ! >\n"); }
"="                  { BEGIN(INITIAL); printf("< assignment : = >\n"); }

"("                  { BEGIN(INITIAL); printf("< left parenthesis : ( >\n"); }
")"                  { BEGIN(OP); printf("< right parenthesis : ) >\n"); }
"{"                  { BEGIN(INITIAL); printf("< left brace : { >\n"); }
"}"                  { BEGIN(INITIAL); printf("< right brace : } >\n"); }
";"                  { BEGIN(INITIAL); printf("< semicolon : ; >\n"); }

.                    { printf("< unknown : %s >\n", yytext); }

%%

int main(void)
{
    printf("Enter Java for loop:\n");
    BEGIN(INITIAL);
    yylex();
    return 0;
}
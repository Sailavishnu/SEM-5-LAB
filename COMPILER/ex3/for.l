%{
#include <stdio.h>
#include <stdlib.h>
FILE *yyin;

/* Tracks whether the previous token behaves like a "value"
   (identifier, number, ')', ']') so we can tell:
     ++x  vs  x++   (prefix vs postfix)
     -x   vs  x-y   (unary vs binary)
   1 = previous token was a value/operand
   0 = previous token was not (start of expr, operator, '(', ',', ';', keyword, etc.) */
int last_was_operand = 0;
%}

%%

"for"|"if"|"else"|"while"|"do"|"class"|"return"|"void"|"static"|"public"|"private"   { printf("<KEYWORD, %s>\n", yytext); last_was_operand = 0; }

"int"|"float"|"double"|"char"|"boolean"|"String"   { printf("<DATATYPE, %s>\n", yytext); last_was_operand = 0; }

"true"|"false"   { printf("<BOOLEAN, %s>\n", yytext); last_was_operand = 1; }

"++"   {
    if (last_was_operand)
        printf("<POST_INCREMENT, %s>\n", yytext);
    else
        printf("<PRE_INCREMENT, %s>\n", yytext);
    last_was_operand = 1;
}

"--"   {
    if (last_was_operand)
        printf("<POST_DECREMENT, %s>\n", yytext);
    else
        printf("<PRE_DECREMENT, %s>\n", yytext);
    last_was_operand = 1;
}

"=="|"!="|"<="|">="   { printf("<RELATIONAL_OPERATOR, %s>\n", yytext); last_was_operand = 0; }

"<"|">"   { printf("<RELATIONAL_OPERATOR, %s>\n", yytext); last_was_operand = 0; }

"&&"|"||"   { printf("<LOGICAL_OPERATOR, %s>\n", yytext); last_was_operand = 0; }

"!"   { printf("<LOGICAL_OPERATOR, %s>\n", yytext); last_was_operand = 0; }

"="   { printf("<ASSIGNMENT_OPERATOR, %s>\n", yytext); last_was_operand = 0; }

"+="|"-="|"*="|"/="|"%="   { printf("<COMPOUND_ASSIGNMENT, %s>\n", yytext); last_was_operand = 0; }

"+"   {
    if (last_was_operand)
        printf("<BINARY_PLUS, %s>\n", yytext);
    else
        printf("<UNARY_PLUS, %s>\n", yytext);
    last_was_operand = 0;
}

"-"   {
    if (last_was_operand)
        printf("<BINARY_MINUS, %s>\n", yytext);
    else
        printf("<UNARY_MINUS, %s>\n", yytext);
    last_was_operand = 0;
}

"*"|"/"|"%"   { printf("<ARITHMETIC_OPERATOR, %s>\n", yytext); last_was_operand = 0; }

[a-zA-Z_][a-zA-Z0-9_]*   { printf("<IDENTIFIER, %s>\n", yytext); last_was_operand = 1; }

[0-9]+(\.[0-9]+)?   { printf("<NUMBER, %s>\n", yytext); last_was_operand = 1; }

"("   { printf("<PUNCTUATION, %s>\n", yytext); last_was_operand = 0; }
")"   { printf("<PUNCTUATION, %s>\n", yytext); last_was_operand = 1; }
"{"   { printf("<PUNCTUATION, %s>\n", yytext); last_was_operand = 0; }
"}"   { printf("<PUNCTUATION, %s>\n", yytext); last_was_operand = 0; }
"["   { printf("<PUNCTUATION, %s>\n", yytext); last_was_operand = 0; }
"]"   { printf("<PUNCTUATION, %s>\n", yytext); last_was_operand = 1; }
";"   { printf("<PUNCTUATION, %s>\n", yytext); last_was_operand = 0; }
","   { printf("<PUNCTUATION, %s>\n", yytext); last_was_operand = 0; }
"."   { printf("<PUNCTUATION, %s>\n", yytext); last_was_operand = 0; }

[ \t\n]+   { /* whitespace: ignore, don't touch last_was_operand */ }

.   { printf("<UNKNOWN, %s>\n", yytext); last_was_operand = 0; }

%%

int yywrap()
{
    return 1;
}

int main()
{
    yyin = fopen("input.txt", "r");
    if (!yyin)
    
    {
        printf("Error: Could not open input.txt!\n");
        return 1;
    }
    printf("Reading and tokenizing input.txt...\n\n");
    yylex();
    fclose(yyin);
    return 0;
}

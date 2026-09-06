%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *yyin;

char prev_token_type[50] = "NONE";
char prev_lexeme[100] = "";

#define CHECK_AND_PRINT(current_type) \
    do { \
        if (strcmp(prev_token_type, "IDENTIFIER") != 0 && strcmp(prev_token_type, "NUMBER") != 0) { \
            printf("<%s, %s>\n", current_type, yytext); \
        } else { \
            printf("<%s, %s>\n", current_type, yytext); \
        } \
        strcpy(prev_token_type, current_type); \
        strcpy(prev_lexeme, yytext); \
    } while(0)
%}

%%
"for"|"in"|"print"|"range"|"break"|"continue"|"pass"|"if"|"elif"|"else" { CHECK_AND_PRINT("KEYWORD"); }
"True"|"False"                                                          { CHECK_AND_PRINT("BOOLEAN"); }
"AND"|"and"|"OR"|"or"|"NOT"|"not"                                       { CHECK_AND_PRINT("LOGICAL_OPERATOR"); }
[a-zA-Z_][a-zA-Z0-9_]*                                                  { CHECK_AND_PRINT("IDENTIFIER"); }
[0-9]+(\.[0-9]+)?                                                      { CHECK_AND_PRINT("NUMBER"); }
"=="|"!="|"<="|">="|"<"|">"                                             { CHECK_AND_PRINT("RELATIONAL_OPERATOR"); }
"="|"+="|"-="|"*="|"/="                                                 { CHECK_AND_PRINT("ASSIGNMENT_OPERATOR"); }

"-" {
    /* Special Context-Sensitive Rule for Minus Signs */
    if (strcmp(prev_token_type, "IDENTIFIER") != 0 && strcmp(prev_token_type, "NUMBER") != 0) {
        printf("<UNARY_MINUS, %s>\n", yytext);
        strcpy(prev_token_type, "UNARY_MINUS");
    } else {
        printf("<OPERATOR, %s>\n", yytext);
        strcpy(prev_token_type, "OPERATOR");
    }
    strcpy(prev_lexeme, yytext);
}

"+"|"*"|"/"                                                             { CHECK_AND_PRINT("OPERATOR"); }
"("|")"|":"|","|"["|"]"                                                 { CHECK_AND_PRINT("PUNCTUATION"); }

[ \t\n]+ { ; } /* Skipped: Whitespaces don't change state history */

. { CHECK_AND_PRINT("UNKNOWN"); }
%%

int yywrap()
{
    return 1;
}

int main()
{
    yyin = fopen("in.txt", "r");
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

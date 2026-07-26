%{
#include<stdio.h>
#include<string.h>
#include<ctype.h>
%}

%%

[a-zA-Z][a-zA-Z0-9&@$\-]* {

    int len = strlen(yytext);
    int upper = 0, lower = 0, digit = 0, special = 0;
    int length = 1;

    /* Length Check */
    if(len < 8 || len > 64)
        length = 0;

    /* Character Checks */
    for(int i = 0; i < len; i++)
    {
        if(isupper(yytext[i]))
            upper = 1;
        else if(islower(yytext[i]))
            lower = 1;
        else if(isdigit(yytext[i]))
            digit = 1;
        else if(yytext[i] == '&' || yytext[i] == '@' || yytext[i] == '$' || yytext[i] == '-')
            special = 1;
    }

    if(!(length && upper && lower && digit && special))
        printf("Invalid Password:\n");

    if(!length)
        printf("Length should be between 8 and 64 characters.\n");

    if(!upper)
        printf("Missing uppercase letter.\n");

    if(!lower)
        printf("Missing lowercase letter.\n");

    if(!digit)
        printf("Missing digit.\n");

    if(!special)
        printf("Missing special character.\n");

    if(length && upper && lower && digit && special)
        printf("Valid Password.\n");

    return 0;
}

. {
    printf("Invalid Password:\n");
    printf("Should start with an alphabet and contain only valid characters.\n");
    return 0;
}

%%

int yywrap()
{
    return 1;
}

int main()
{
    printf("Enter Password: ");
    yylex();
    return 0;
}
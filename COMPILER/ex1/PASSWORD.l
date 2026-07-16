%{
#include<stdio.h>
#include<string.h>
#include<ctype.h>

char password[100];
%}

%%

[a-zA-Z][a-zA-Z0-9&@$\-]*    {
    int len = strlen(yytext);
    int upper=0, lower=0, digit=0, special=0;

    if(len<8)
    {
        printf("Invalid Password: Length should be at least 8.\n");
        return 0;
    }

    if(len>64)
    {
        printf("Invalid Password: Length should not exceed 64.\n");
        return 0;
    }

    for(int i=0;i<len;i++)
    {
        if(isupper(yytext[i])) upper=1;
        if(islower(yytext[i])) lower=1;
        if(isdigit(yytext[i])) digit=1;
        if(yytext[i]=='&' || yytext[i]=='@' || yytext[i]=='$' || yytext[i]=='-')
            special=1;
    }

    if(!upper)
        printf("Invalid: No uppercase letter.\n");
    if(!lower)
        printf("Invalid: No lowercase letter.\n");
    if(!digit)
        printf("Invalid: No digit.\n");
    if(!special)
        printf("Invalid: No special character.\n");

    if(upper && lower && digit && special)
        printf("Valid Password.\n");
}

.|\n {
    printf("Invalid Password: Should start with an alphabet and contain only valid characters.\n");
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
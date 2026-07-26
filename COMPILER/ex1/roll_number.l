%{
#include<stdio.h>

FILE *in, *out;
int count = 0;
%}

%%

[0-9]{2}(BCS|BIT|BEE|BEC|BAD|BME|BBT|bcs|bit|bee|bec|bad|bme|bbt)[0-9]{3} {
    fprintf(out, "%s\n", yytext);
    count++;
}

[ \t\n]+ ;

. ;

%%

int yywrap()
{
    return 1;
}

int main()
{
    in = fopen("rollno_input.txt", "r");

    if(in == NULL)
    {
        printf("Cannot open input file.\n");
        return 1;
    }

    out = fopen("rollno_output.txt", "w");

    if(out == NULL)
    {
        printf("Cannot open output file.\n");
        fclose(in);
        return 1;
    }

    yyin = in;
    yylex();

    printf("Valid roll numbers are stored in rollnumbers.txt\n");
    printf("Total Valid Roll Numbers = %d\n", count);

    fclose(in);
    fclose(out);

    return 0;
}
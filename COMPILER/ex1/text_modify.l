%{
#include<stdio.h>
#include<stdlib.h>

FILE *in, *out;
int is_count = 0;

void incrementNumber(char *num_str)
{
    int num = atoi(num_str);
    fprintf(out, "%d", num + 10);
}
%}

%%

" is " {
    fprintf(out, " was ");
    is_count++;
}

[0-9]+ {
    incrementNumber(yytext);
}

.|\n {
    fprintf(out, "%s", yytext);
}

%%

int yywrap()
{
    return 1;
}

int main()
{
    in = fopen("text_input.txt", "r");

    if(in == NULL)
    {
        printf("Cannot open input file.\n");
        return 1;
    }

    out = fopen("text_output.txt", "w");

    if(out == NULL)
    {
        printf("Cannot open output file.\n");
        fclose(in);
        return 1;
    }

    yyin = in;
    yylex();

    printf("Text processing completed.\n");
    printf("Number of 'is' replaced = %d\n", is_count);

    fclose(in);
    fclose(out);

    return 0;
}
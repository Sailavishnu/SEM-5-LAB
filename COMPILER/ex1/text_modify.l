%{
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

FILE *out;
int is_count = 0;

/* Function to increment number by 10 */
void incrementNumber(char *num_str)
{
    int num = atoi(num_str);
    num += 10;
    fprintf(out, "%d", num);
}
%}

%%

"is"  {
    fprintf(out, "was");
    is_count++;
}

[0-9]+  {
    incrementNumber(yytext);
}

.  {
    fprintf(out, "%s", yytext);
}

\n  {
    fprintf(out, "\n");
}

%%

int yywrap()
{
    return 1;
}

int main()
{
    FILE *in;
    
    in = fopen("input_text.txt", "r");
    if(in == NULL)
    {
        printf("Cannot open input file.\n");
        return 1;
    }
    
    out = fopen("output_text.txt", "w");
    if(out == NULL)
    {
        printf("Cannot open output file.\n");
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

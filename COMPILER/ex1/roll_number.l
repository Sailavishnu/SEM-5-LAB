%{
#include<stdio.h>
#include<string.h>
#include<ctype.h>

FILE *out;
int count = 0;

/* Function to validate branch code */
int isValidBranch(char *branch)
{
    if(strcmp(branch, "BCS") == 0) return 1;
    if(strcmp(branch, "BIT") == 0) return 1;
    if(strcmp(branch, "BAI") == 0) return 1;
    if(strcmp(branch, "BEE") == 0) return 1;
    if(strcmp(branch, "BEC") == 0) return 1;
    if(strcmp(branch, "BME") == 0) return 1;
    if(strcmp(branch, "BBT") == 0) return 1;
    return 0;
}

/* Function to check roll number format yybbnnn */
int isValidRollNumber(char *roll)
{
    int len = strlen(roll);
    
    if(len != 7)
        return 0;
    
    /* Check first 2 chars are digits (year) */
    if(!isdigit(roll[0]) || !isdigit(roll[1]))
        return 0;
    
    /* Extract branch code (3 chars) */
    char branch[4];
    strncpy(branch, roll+2, 3);
    branch[3] = '\0';
    
    if(!isValidBranch(branch))
        return 0;
    
    /* Check last 3 chars are digits */
    if(!isdigit(roll[5]) || !isdigit(roll[6]))
        return 0;
    
    return 1;
}
%}

%%

[0-9]{2}[A-Z]{3}[0-9]{3}  {
    fprintf(out, "%s\n", yytext);
    count++;
}

[ \t]+ ;

\n      ;

.       ;

%%

int yywrap()
{
    return 1;
}

int main()
{
    FILE *in;
    
    in = fopen("student.txt", "r");
    if(in == NULL)
    {
        printf("Cannot open input file.\n");
        return 1;
    }
    
    out = fopen("rollnumbers.txt", "w");
    if(out == NULL)
    {
        printf("Cannot open output file.\n");
        return 1;
    }
    
    yyin = in;
    yylex();
    
    printf("Roll Numbers found:\n");
    printf("Total number of students = %d\n", count);
    
    fclose(in);
    fclose(out);
    
    return 0;
}

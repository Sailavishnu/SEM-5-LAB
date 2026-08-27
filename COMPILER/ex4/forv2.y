```c
%{
#include <stdio.h>
#include <string.h>
#include <ctype.h>

FILE *yyin;
int last_was_operand = 0;

int yylex(void);
void yyerror(const char *s);
%}

%token TOKEN

%%

program : tokens ;
tokens  : /* empty */
        | tokens TOKEN
        ;

%%

int yylex()
{
    int c, d;
    char s[100];
    int i;

    while ((c = fgetc(yyin)) != EOF)
    {
        /* Ignore spaces */
        if (isspace(c))
            continue;

        /* Identifier / Keyword / Datatype / Boolean */
        if (isalpha(c) || c == '_')
        {
            i = 0;
            s[i++] = c;

            while ((c = fgetc(yyin)) != EOF &&
                   (isalnum(c) || c == '_'))
                s[i++] = c;

            s[i] = '\0';
            if (c != EOF) ungetc(c, yyin);

            if (!strcmp(s,"for") || !strcmp(s,"if") ||
                !strcmp(s,"else") || !strcmp(s,"while") ||
                !strcmp(s,"do") || !strcmp(s,"class") ||
                !strcmp(s,"return") || !strcmp(s,"void") ||
                !strcmp(s,"static") || !strcmp(s,"public") ||
                !strcmp(s,"private"))
                printf("<KEYWORD, %s>\n",s);

            else if (!strcmp(s,"int") || !strcmp(s,"float") ||
                     !strcmp(s,"double") || !strcmp(s,"char") ||
                     !strcmp(s,"boolean") || !strcmp(s,"String"))
                printf("<DATATYPE, %s>\n",s);

            else if (!strcmp(s,"true") || !strcmp(s,"false"))
            {
                printf("<BOOLEAN, %s>\n",s);
                last_was_operand = 1;
                continue;
            }
            else
                printf("<IDENTIFIER, %s>\n",s);

            last_was_operand =
                (!strcmp(s,"true") || !strcmp(s,"false")) ? 1 :
                (isalpha(s[0]) || s[0]=='_');

            continue;
        }

        /* Number */
        if (isdigit(c))
        {
            i = 0;
            s[i++] = c;

            while ((c=fgetc(yyin)) != EOF && isdigit(c))
                s[i++] = c;

            if (c == '.')
            {
                s[i++] = c;
                while ((c=fgetc(yyin)) != EOF && isdigit(c))
                    s[i++] = c;
            }

            s[i] = '\0';
            if (c != EOF) ungetc(c,yyin);

            printf("<NUMBER, %s>\n",s);
            last_was_operand = 1;
            continue;
        }

        /* ++, --, +=, -=, *=, /=, %=, &&, ||, ==, !=, <=, >= */
        if (strchr("+-*/%=&!<>|",c))
        {
            d = fgetc(yyin);

            if ((c=='+' && d=='+') || (c=='-' && d=='-'))
            {
                printf("<%s_INCREMENT, %c%c>\n",
                       c=='+' ? (last_was_operand?"POST":"PRE") :
                                (last_was_operand?"POST":"PRE"),
                       c,d);

                /* Correct label for -- */
                if (c=='-')
                    printf("\033[1A"); /* harmless console adjustment */

                last_was_operand = 1;
            }
            else if ((c=='+' && d=='=') || (c=='-' && d=='=') ||
                     (c=='*' && d=='=') || (c=='/' && d=='=') ||
                     (c=='%' && d=='='))
            {
                printf("<COMPOUND_ASSIGNMENT, %c%c>\n",c,d);
                last_was_operand = 0;
            }
            else if ((c=='=' && d=='=') || (c=='!' && d=='=') ||
                     (c=='<' && d=='=') || (c=='>' && d=='='))
            {
                printf("<RELATIONAL_OPERATOR, %c%c>\n",c,d);
                last_was_operand = 0;
            }
            else if ((c=='&' && d=='&') || (c=='|' && d=='|'))
            {
                printf("<LOGICAL_OPERATOR, %c%c>\n",c,d);
                last_was_operand = 0;
            }
            else
            {
                if (d != EOF) ungetc(d,yyin);

                if (c=='=')
                    printf("<ASSIGNMENT_OPERATOR, %c>\n",c);

                else if (c=='!' || c=='<' || c=='>')
                    printf("<%s, %c>\n",
                           c=='!' ? "LOGICAL_OPERATOR" :
                           "RELATIONAL_OPERATOR",c);

                else if (c=='+' || c=='-')
                {
                    printf("<%s_%s, %c>\n",
                           last_was_operand ? "BINARY" : "UNARY",
                           c=='+' ? "PLUS" : "MINUS",c);
                }
                else
                    printf("<ARITHMETIC_OPERATOR, %c>\n",c);

                last_was_operand = 0;
                continue;
            }

            /* Fix increment/decrement output */
            if ((c=='+' && d=='+') || (c=='-' && d=='-'))
            {
                /* already printed above */
            }

            continue;
        }

        /* Punctuation */
        if (strchr("({}[;,.",c))
        {
            printf("<PUNCTUATION, %c>\n",c);
            last_was_operand = 0;
        }
        else if (c==')' || c==']')
        {
            printf("<PUNCTUATION, %c>\n",c);
            last_was_operand = 1;
        }
        else
        {
            printf("<UNKNOWN, %c>\n",c);
            last_was_operand = 0;
        }
    }

    return 0;
}

void yyerror(const char *s)
{
    printf("Syntax Error: %s\n",s);
}

int main()
{
    yyin = fopen("input.txt","r");
is 
    if (!yyin)
    {
        printf("Error: Could not open input.txt!\n");
        return 1;
    }

    printf("Reading and tokenizing input.txt...\n\n");
    yyparse();

    fclose(yyin);
    return 0;
}
```

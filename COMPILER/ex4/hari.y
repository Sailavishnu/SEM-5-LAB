%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

extern FILE *yyin;

int yylex(void);
void yyerror(const char *s);
%}

%token NUMBER KEYWORD BOOLEAN RELATIONAL INCREMENT OPERATOR
%token IDENTIFIER INVALID BRACKETS UNKNOWN

%%

program:
    tokens
    ;

tokens:
      tokens token
    | token
    ;

token:
      NUMBER
    | KEYWORD
    | BOOLEAN
    | RELATIONAL
    | INCREMENT
    | OPERATOR
    | IDENTIFIER
    | INVALID
    | BRACKETS
    | UNKNOWN
    ;

%%

int yylex(void)
{
    int c;
    char buffer[100];
    int i;

    while ((c = fgetc(yyin)) != EOF)
    {
        if (isspace(c))
            continue;

        if (isdigit(c))
        {
            i = 0;
            buffer[i++] = c;

            while ((c = fgetc(yyin)) != EOF && isdigit(c))
                buffer[i++] = c;

            if (c == '.')
            {
                buffer[i++] = c;

                while ((c = fgetc(yyin)) != EOF && isdigit(c))
                    buffer[i++] = c;

                buffer[i] = '\0';
                printf("<NUMBER,%s>\n", buffer);
                return NUMBER;
            }

            if (isalpha(c))
            {
                buffer[i++] = c;

                while ((c = fgetc(yyin)) != EOF &&
                       isalnum(c))
                    buffer[i++] = c;

                buffer[i] = '\0';
                printf("<INVALID,%s>\n", buffer);
                return INVALID;
            }

            buffer[i] = '\0';

            if (c != EOF)
                ungetc(c, yyin);

            printf("<NUMBER,%s>\n", buffer);
            return NUMBER;
        }

        if (isalpha(c))
        {
            i = 0;
            buffer[i++] = c;

            while ((c = fgetc(yyin)) != EOF && isalnum(c))
                buffer[i++] = c;

            buffer[i] = '\0';

            if (c != EOF)
                ungetc(c, yyin);

            if (!strcmp(buffer, "auto") ||
                !strcmp(buffer, "break") ||
                !strcmp(buffer, "case") ||
                !strcmp(buffer, "char") ||
                !strcmp(buffer, "const") ||
                !strcmp(buffer, "continue") ||
                !strcmp(buffer, "default") ||
                !strcmp(buffer, "do") ||
                !strcmp(buffer, "double") ||
                !strcmp(buffer, "else") ||
                !strcmp(buffer, "enum") ||
                !strcmp(buffer, "extern") ||
                !strcmp(buffer, "float") ||
                !strcmp(buffer, "for") ||
                !strcmp(buffer, "goto") ||
                !strcmp(buffer, "if") ||
                !strcmp(buffer, "int") ||
                !strcmp(buffer, "long") ||
                !strcmp(buffer, "register") ||
                !strcmp(buffer, "return") ||
                !strcmp(buffer, "short") ||
                !strcmp(buffer, "signed") ||
                !strcmp(buffer, "sizeof") ||
                !strcmp(buffer, "static") ||
                !strcmp(buffer, "struct") ||
                !strcmp(buffer, "switch") ||
                !strcmp(buffer, "typedef") ||
                !strcmp(buffer, "union") ||
                !strcmp(buffer, "unsigned") ||
                !strcmp(buffer, "void") ||
                !strcmp(buffer, "volatile") ||
                !strcmp(buffer, "while"))
            {
                printf("<KEYWORD,%s>\n", buffer);
                return KEYWORD;
            }

            if (!strcmp(buffer, "true") ||
                !strcmp(buffer, "false"))
            {
                printf("<BOOLEAN,%s>\n", buffer);
                return BOOLEAN;
            }

            printf("<IDENTIFIER,%s>\n", buffer);
            return IDENTIFIER;
        }

        if (c == '=')
        {
            c = fgetc(yyin);

            if (c == '=')
            {
                printf("<RELATIONAL OPERATOR,==>\n");
                return RELATIONAL;
            }

            if (c != EOF)
                ungetc(c, yyin);

            printf("<OPERATOR,=>\n");
            return OPERATOR;
        }

        if (c == '!')
        {
            c = fgetc(yyin);

            if (c == '=')
            {
                printf("<RELATIONAL OPERATOR,!=>\n");
                return RELATIONAL;
            }

            if (c != EOF)
                ungetc(c, yyin);

            printf("<UNKNOWN,!>\n");
            return UNKNOWN;
        }

        if (c == '<' || c == '>')
        {
            int first = c;
            c = fgetc(yyin);

            if (c == '=')
            {
                printf("<RELATIONAL OPERATOR,%c=>\n", first);
                return RELATIONAL;
            }

            if (c != EOF)
                ungetc(c, yyin);

            printf("<RELATIONAL OPERATOR,%c>\n", first);
            return RELATIONAL;
        }

        if (c == '+' || c == '-')
        {
            int first = c;
            c = fgetc(yyin);

            if (c == first)
            {
                printf("<INCREMENT OPERATOR,%c%c>\n",
                       first, first);
                return INCREMENT;
            }

            if (c != EOF)
                ungetc(c, yyin);

            printf("<OPERATOR,%c>\n", first);
            return OPERATOR;
        }

        if (c == '*' || c == '/' || c == '%')
        {
            printf("<OPERATOR,%c>\n", c);
            return OPERATOR;
        }

        if (c == '{' || c == '}' ||
            c == '(' || c == ')' ||
            c == '[' || c == ']')
        {
            printf("<BRACKETS,%c>\n", c);
            return BRACKETS;
        }

        printf("<UNKNOWN,%c>\n", c);
        return UNKNOWN;
    }

    return 0;
}

int main()
{
    yyin = fopen("input.c", "r");

    if (!yyin)
    {
        printf("Error in opening file");
        return 1;
    }

    printf("Reading and tokenizing\n");

    yyparse();

    fclose(yyin);

    return 0;
}

void yyerror(const char *s)
{
    printf("Syntax Error\n");
}
```c
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

FILE *yyin;

int last_was_operand = 0;

int yylex(void);
void yyerror(const char *s);
%}

/* Tokens */
%token KEYWORD
%token DATATYPE
%token BOOLEAN
%token PRE_INCREMENT
%token POST_INCREMENT
%token PRE_DECREMENT
%token POST_DECREMENT
%token RELATIONAL_OPERATOR
%token LOGICAL_OPERATOR
%token ASSIGNMENT_OPERATOR
%token COMPOUND_ASSIGNMENT
%token BINARY_PLUS
%token UNARY_PLUS
%token BINARY_MINUS
%token UNARY_MINUS
%token ARITHMETIC_OPERATOR
%token IDENTIFIER
%token NUMBER
%token PUNCTUATION

%%

program
    : tokens
    ;

tokens
    : token
    | tokens token
    ;

token
    : KEYWORD
    | DATATYPE
    | BOOLEAN
    | PRE_INCREMENT
    | POST_INCREMENT
    | PRE_DECREMENT
    | POST_DECREMENT
    | RELATIONAL_OPERATOR
    | LOGICAL_OPERATOR
    | ASSIGNMENT_OPERATOR
    | COMPOUND_ASSIGNMENT
    | BINARY_PLUS
    | UNARY_PLUS
    | BINARY_MINUS
    | UNARY_MINUS
    | ARITHMETIC_OPERATOR
    | IDENTIFIER
    | NUMBER
    | PUNCTUATION
    ;

%%

/* -------------------------------------------------
   LEXICAL ANALYZER
   ------------------------------------------------- */

int yylex(void)
{
    int c;
    char buffer[256];
    int i;

    while ((c = fgetc(yyin)) != EOF)
    {
        /* Whitespace */
        if (c == ' ' || c == '\t' || c == '\n')
        {
            continue;
        }

        /* -----------------------------------------
           Identifier / Keyword / Datatype / Boolean
           ----------------------------------------- */
        if (isalpha(c) || c == '_')
        {
            i = 0;
            buffer[i++] = c;

            while ((c = fgetc(yyin)) != EOF &&
                   (isalnum(c) || c == '_'))
            {
                buffer[i++] = c;
            }

            buffer[i] = '\0';

            if (c != EOF)
                ungetc(c, yyin);

            /* Keywords */
            if (!strcmp(buffer, "for") ||
                !strcmp(buffer, "if") ||
                !strcmp(buffer, "else") ||
                !strcmp(buffer, "while") ||
                !strcmp(buffer, "do") ||
                !strcmp(buffer, "class") ||
                !strcmp(buffer, "return") ||
                !strcmp(buffer, "void") ||
                !strcmp(buffer, "static") ||
                !strcmp(buffer, "public") ||
                !strcmp(buffer, "private"))
            {
                printf("<KEYWORD, %s>\n", buffer);
                last_was_operand = 0;
                return KEYWORD;
            }

            /* Datatypes */
            if (!strcmp(buffer, "int") ||
                !strcmp(buffer, "float") ||
                !strcmp(buffer, "double") ||
                !strcmp(buffer, "char") ||
                !strcmp(buffer, "boolean") ||
                !strcmp(buffer, "String"))
            {
                printf("<DATATYPE, %s>\n", buffer);
                last_was_operand = 0;
                return DATATYPE;
            }

            /* Boolean */
            if (!strcmp(buffer, "true") ||
                !strcmp(buffer, "false"))
            {
                printf("<BOOLEAN, %s>\n", buffer);
                last_was_operand = 1;
                return BOOLEAN;
            }

            /* Identifier */
            printf("<IDENTIFIER, %s>\n", buffer);
            last_was_operand = 1;
            return IDENTIFIER;
        }

        /* -----------------------------------------
           Number
           ----------------------------------------- */
        if (isdigit(c))
        {
            i = 0;
            buffer[i++] = c;

            while ((c = fgetc(yyin)) != EOF && isdigit(c))
            {
                buffer[i++] = c;
            }

            /* Decimal number */
            if (c == '.')
            {
                buffer[i++] = c;

                c = fgetc(yyin);

                if (isdigit(c))
                {
                    buffer[i++] = c;

                    while ((c = fgetc(yyin)) != EOF &&
                           isdigit(c))
                    {
                        buffer[i++] = c;
                    }
                }
                else
                {
                    if (c != EOF)
                        ungetc(c, yyin);
                }
            }

            buffer[i] = '\0';

            if (c != EOF)
                ungetc(c, yyin);

            printf("<NUMBER, %s>\n", buffer);
            last_was_operand = 1;
            return NUMBER;
        }

        /* -----------------------------------------
           Increment ++
           ----------------------------------------- */
        if (c == '+')
        {
            int next = fgetc(yyin);

            if (next == '+')
            {
                if (last_was_operand)
                {
                    printf("<POST_INCREMENT, ++>\n");
                    last_was_operand = 1;
                    return POST_INCREMENT;
                }
                else
                {
                    printf("<PRE_INCREMENT, ++>\n");
                    last_was_operand = 1;
                    return PRE_INCREMENT;
                }
            }

            if (next == '=')
            {
                printf("<COMPOUND_ASSIGNMENT, +=>\n");
                last_was_operand = 0;
                return COMPOUND_ASSIGNMENT;
            }

            if (next != EOF)
                ungetc(next, yyin);

            /* Plus */
            if (last_was_operand)
            {
                printf("<BINARY_PLUS, +>\n");
                last_was_operand = 0;
                return BINARY_PLUS;
            }
            else
            {
                printf("<UNARY_PLUS, +>\n");
                last_was_operand = 0;
                return UNARY_PLUS;
            }
        }

        /* -----------------------------------------
           Decrement --
           ----------------------------------------- */
        if (c == '-')
        {
            int next = fgetc(yyin);

            if (next == '-')
            {
                if (last_was_operand)
                {
                    printf("<POST_DECREMENT, -->\n");
                    last_was_operand = 1;
                    return POST_DECREMENT;
                }
                else
                {
                    printf("<PRE_DECREMENT, -->\n");
                    last_was_operand = 1;
                    return PRE_DECREMENT;
                }
            }

            if (next == '=')
            {
                printf("<COMPOUND_ASSIGNMENT, -=>\n");
                last_was_operand = 0;
                return COMPOUND_ASSIGNMENT;
            }

            if (next != EOF)
                ungetc(next, yyin);

            /* Minus */
            if (last_was_operand)
            {
                printf("<BINARY_MINUS, ->\n");
                last_was_operand = 0;
                return BINARY_MINUS;
            }
            else
            {
                printf("<UNARY_MINUS, ->\n");
                last_was_operand = 0;
                return UNARY_MINUS;
            }
        }

        /* -----------------------------------------
           Multiplication / Compound multiplication
           ----------------------------------------- */
        if (c == '*')
        {
            int next = fgetc(yyin);

            if (next == '=')
            {
                printf("<COMPOUND_ASSIGNMENT, *=>\n");
                last_was_operand = 0;
                return COMPOUND_ASSIGNMENT;
            }

            if (next != EOF)
                ungetc(next, yyin);

            printf("<ARITHMETIC_OPERATOR, *>\n");
            last_was_operand = 0;
            return ARITHMETIC_OPERATOR;
        }

        /* -----------------------------------------
           Division / Compound division
           ----------------------------------------- */
        if (c == '/')
        {
            int next = fgetc(yyin);

            if (next == '=')
            {
                printf("<COMPOUND_ASSIGNMENT, /=>\n");
                last_was_operand = 0;
                return COMPOUND_ASSIGNMENT;
            }

            if (next != EOF)
                ungetc(next, yyin);

            printf("<ARITHMETIC_OPERATOR, />\n");
            last_was_operand = 0;
            return ARITHMETIC_OPERATOR;
        }

        /* -----------------------------------------
           Modulus / Compound modulus
           ----------------------------------------- */
        if (c == '%')
        {
            int next = fgetc(yyin);

            if (next == '=')
            {
                printf("<COMPOUND_ASSIGNMENT, %=>\n");
                last_was_operand = 0;
                return COMPOUND_ASSIGNMENT;
            }

            if (next != EOF)
                ungetc(next, yyin);

            printf("<ARITHMETIC_OPERATOR, %>\n");
            last_was_operand = 0;
            return ARITHMETIC_OPERATOR;
        }

        /* -----------------------------------------
           Relational operators
           == != <= >=
           ----------------------------------------- */
        if (c == '=' || c == '!' ||
            c == '<' || c == '>')
        {
            int next = fgetc(yyin);

            if (c == '=' && next == '=')
            {
                printf("<RELATIONAL_OPERATOR, ==>\n");
                last_was_operand = 0;
                return RELATIONAL_OPERATOR;
            }

            if (c == '!' && next == '=')
            {
                printf("<RELATIONAL_OPERATOR, !=>\n");
                last_was_operand = 0;
                return RELATIONAL_OPERATOR;
            }

            if (c == '<' && next == '=')
            {
                printf("<RELATIONAL_OPERATOR, <=>\n");
                last_was_operand = 0;
                return RELATIONAL_OPERATOR;
            }

            if (c == '>' && next == '=')
            {
                printf("<RELATIONAL_OPERATOR, >=>\n");
                last_was_operand = 0;
                return RELATIONAL_OPERATOR;
            }

            if (next != EOF)
                ungetc(next, yyin);

            if (c == '=')
            {
                printf("<ASSIGNMENT_OPERATOR, =>\n");
                last_was_operand = 0;
                return ASSIGNMENT_OPERATOR;
            }

            if (c == '!')
            {
                printf("<LOGICAL_OPERATOR, !>\n");
                last_was_operand = 0;
                return LOGICAL_OPERATOR;
            }

            printf("<RELATIONAL_OPERATOR, %c>\n", c);
            last_was_operand = 0;
            return RELATIONAL_OPERATOR;
        }

        /* -----------------------------------------
           Logical operators && ||
           ----------------------------------------- */
        if (c == '&' || c == '|')
        {
            int next = fgetc(yyin);

            if (c == '&' && next == '&')
            {
                printf("<LOGICAL_OPERATOR, &&>\n");
                last_was_operand = 0;
                return LOGICAL_OPERATOR;
            }

            if (c == '|' && next == '|')
            {
                printf("<LOGICAL_OPERATOR, ||>\n");
                last_was_operand = 0;
                return LOGICAL_OPERATOR;
            }

            if (next != EOF)
                ungetc(next, yyin);

            printf("<UNKNOWN, %c>\n", c);
            last_was_operand = 0;
            return PUNCTUATION;
        }

        /* -----------------------------------------
           Punctuation
           ----------------------------------------- */
        if (c == '(' || c == '{' || c == '}' ||
            c == '[' || c == ';' || c == ',' ||
            c == '.')
        {
            printf("<PUNCTUATION, %c>\n", c);
            last_was_operand = 0;
            return PUNCTUATION;
        }

        if (c == ')' || c == ']')
        {
            printf("<PUNCTUATION, %c>\n", c);
            last_was_operand = 1;
            return PUNCTUATION;
        }

        /* -----------------------------------------
           Unknown character
           ----------------------------------------- */
        printf("<UNKNOWN, %c>\n", c);
        last_was_operand = 0;
    }

    return 0;
}


/* -------------------------------------------------
   ERROR HANDLER
   ------------------------------------------------- */

void yyerror(const char *s)
{
    printf("Syntax Error: %s\n", s);
}


/* -------------------------------------------------
   MAIN
   ------------------------------------------------- */

int main(void)
{
    yyin = fopen("input.txt", "r");

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

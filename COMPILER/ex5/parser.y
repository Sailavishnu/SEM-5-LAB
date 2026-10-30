%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int  yylex(void);
void yyerror(const char *msg);

typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char *str);
extern void yy_delete_buffer(YY_BUFFER_STATE buf);

int math_error = 0;    
int syntax_error = 0;  
%}

%union {
    double val;
}

%token <val> NUMBER
%type  <val> expr

%left  '+' '-'
%left  '*' '/'
%right UMINUS
%right '^'

%%

line
    : expr '\n'
        {
            if (!math_error && !syntax_error)
                printf("Result = %g\n", $1);
        }
    | '\n'
        { printf("Nothing entered.\n"); }
    | error '\n'
        { yyerrok; }
    ;

expr
    : NUMBER                { $$ = $1; }
    | expr '+' expr         { $$ = $1 + $3; }
    | expr '-' expr         { $$ = $1 - $3; }
    | expr '*' expr         { $$ = $1 * $3; }
    | expr '/' expr
        {
            if ($3 == 0) {
                printf("Error: division by zero\n");
                math_error = 1;
                $$ = 0;
            } else {
                $$ = $1 / $3;
            }
        }
    | expr '^' expr
        {
            $$ = pow($1, $3);
            if (isnan($$)) {
                printf("Error: result is not a real number\n");
                math_error = 1;
                $$ = 0;
            }
        }
    | '-' expr %prec UMINUS { $$ = -$2; }
    | '(' expr ')'          { $$ = $2; }
    ;

%%

void yyerror(const char *msg)
{
    printf("Error: invalid expression (%s)\n", msg);
    syntax_error = 1;
}

int main(void)
{
    char line[256];
    char again[16];

    do {
        printf("Enter expression: ");
        if (fgets(line, sizeof line, stdin) == NULL)
            break;

        if (strchr(line, '\n') == NULL)
            strcat(line, "\n");

        math_error = 0;
        syntax_error = 0;

        YY_BUFFER_STATE buf = yy_scan_string(line);
        yyparse();
        yy_delete_buffer(buf);

        printf("Run again? (y/n): ");
        if (fgets(again, sizeof again, stdin) == NULL)
            break;
    } while (again[0] == 'y' || again[0] == 'Y');

    return 0;
}

%{
#include <stdio.h>
int yylex(void);
void yyerror(const char *s);
extern FILE *yyin;
extern char *yytext;
extern int yylineno, at_eof;
int err_line = 0;
%}

%token FOR INT DOUBLE BOOLEAN_TYPE IDENTIFIER NUMBER BOOLEAN
%token LP RP LB RB SEMI ASSIGN UPLUS UMINUS BPLUS BMINUS
%token PRE_INC POST_INC PRE_DEC POST_DEC RELOP LOGICAL NOT UNKNOWN

%%
program : for_stmt ;

for_stmt : FOR LP init SEMI cond SEMI upd RP stmt ;

init : | type IDENTIFIER ASSIGN expr | IDENTIFIER ASSIGN expr ;
cond : | expr ;
upd  : | update ;

update : PRE_INC IDENTIFIER | PRE_DEC IDENTIFIER
       | IDENTIFIER POST_INC | IDENTIFIER POST_DEC
       | IDENTIFIER ASSIGN expr ;

stmt : for_stmt | LB stmts RB | simple SEMI | SEMI ;
stmts : | stmts stmt ;
simple : update | type IDENTIFIER | type IDENTIFIER ASSIGN expr ;
type : INT | DOUBLE | BOOLEAN_TYPE ;

expr  : logic ;
logic : rel | logic LOGICAL rel ;
rel   : arith | arith RELOP arith ;
arith : term | arith BPLUS term | arith BMINUS term ;
term  : IDENTIFIER | NUMBER | BOOLEAN
      | UPLUS term | UMINUS term | NOT term | LP expr RP ;
%%

void yyerror(const char *s) {
    err_line = yylineno;
}

int main(int argc, char *argv[]) {
    const char *file = argc > 1 ? argv[1] : "input.txt";
    yyin = fopen(file, "r");
    if (!yyin) { printf("Cannot open %s\n", file); return 1; }
    if (yyparse() == 0)
        printf("Correct: the for loop is syntactically correct.\n");
    else
        printf("Not correct: syntax error at line %d.\n", err_line);
    return 0;
}
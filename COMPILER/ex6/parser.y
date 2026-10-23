%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#define MAX 500
#define LEN 32

typedef struct { char label[64], op[LEN], a1[LEN], a2[LEN], res[LEN]; } Quad;
typedef struct { Quad *q; int n; } Chunk;

static Quad quads[MAX];
static int  nq = 0, tc = 0, lc = 0;
static char pending[64] = "";            

int yylex(void);
void yyerror(const char *s) { fprintf(stderr, "Syntax error\n"); }
extern FILE *yyin;

static char *name(char c, int *n) { char b[LEN]; sprintf(b, "%c%d", c, ++*n); return strdup(b); }

static void emit(const char *op, const char *a1, const char *a2, const char *fmt, ...)
{
    Quad *q = &quads[nq++];
    va_list v; va_start(v, fmt); vsnprintf(q->res, LEN, fmt, v); va_end(v);
    strcpy(q->label, *pending ? pending : "-"); *pending = 0;
    strcpy(q->op, op); strcpy(q->a1, a1); strcpy(q->a2, a2);
}

static char *bin(const char *op, const char *a, const char *b)
{
    char *t = name('t', &tc); emit(op, a, b, "%s", t); return t;
}

static Chunk *cut(int start)
{
    Chunk *c = malloc(sizeof *c);
    c->n = nq - start;
    c->q = memcpy(malloc((c->n + 1) * sizeof(Quad)), &quads[start], c->n * sizeof(Quad));
    nq = start;
    if (c->n) { strcpy(pending, c->q[0].label); strcpy(c->q[0].label, "-"); }   
    return c;
}

static void paste(Chunk *c)
{
    if (c->n && *pending) { strcpy(c->q[0].label, pending); *pending = 0; }   
    memcpy(&quads[nq], c->q, c->n * sizeof(Quad)); nq += c->n; free(c->q); free(c);
}
%}

%union { char *str; int pos; void *chunk; }

%token <str> ID NUM
%token FOR AND OR NOT LE GE EQ NE INC DEC
%type <str> expr

%left OR
%left AND
%left EQ NE
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/' '%'
%right NOT

%%

program   : stmt_list ;
stmt_list : stmt_list stmt | ;
stmt      : assign ';' | for_stmt | '{' stmt_list '}' | ';' ;

assign
    : ID '=' expr { emit("=", $3, "-", "%s", $1); }
    | ID INC      { emit("=", bin("+", $1, "1"), "-", "%s", $1); }
    | ID DEC      { emit("=", bin("-", $1, "1"), "-", "%s", $1); }
    ;

opt_assign : assign | ;

/* init; Lcond: cond; if c goto Lbody; goto Lend; Lbody: body; incr; goto Lcond; Lend: */
for_stmt
    : FOR '(' opt_assign ';'
      { $<str>$ = name('L', &lc); strcpy(pending, $<str>$); }                 /* 5: Lcond */
      expr ';'
      { char *body = name('L', &lc);  $<str>$ = name('L', &lc);                /* 8: Lend  */
        emit("if",   $6, "-", "goto %s", body);
        emit("goto", "-", "-", "goto %s", $<str>$);
        strcpy(pending, body); }
      { $<pos>$ = nq; }                                                       /* 9 */
      opt_assign ')'
      { $<chunk>$ = cut($<pos>9); }                                           /* 12 */
      stmt
      { paste($<chunk>12); emit("goto", "-", "-", "goto %s", $<str>5); strcpy(pending, $<str>8); }
    ;

expr
    : expr '+' expr { $$ = bin("+",  $1, $3); }
    | expr '-' expr { $$ = bin("-",  $1, $3); }
    | expr '*' expr { $$ = bin("*",  $1, $3); }
    | expr '/' expr { $$ = bin("/",  $1, $3); }
    | expr '%' expr { $$ = bin("%",  $1, $3); }
    | expr '<' expr { $$ = bin("<",  $1, $3); }
    | expr '>' expr { $$ = bin(">",  $1, $3); }
    | expr LE  expr { $$ = bin("<=", $1, $3); }
    | expr GE  expr { $$ = bin(">=", $1, $3); }
    | expr EQ  expr { $$ = bin("==", $1, $3); }
    | expr NE  expr { $$ = bin("!=", $1, $3); }
    | expr AND expr { $$ = bin("&&", $1, $3); }
    | expr OR  expr { $$ = bin("||", $1, $3); }
    | NOT expr      { $$ = bin("!",  $2, "-"); }
    | '(' expr ')'  { $$ = $2; }
    | ID | NUM
    ;

%%

int main(void)
{
    if (!(yyin = fopen("input.txt", "r"))) { puts("Could not open input.txt"); return 1; }
    if (yyparse()) return 1;
    if (*pending) emit("-", "-", "-", "-");            
    
    char seen[MAX * 3][LEN]; int n = 0;
    for (int i = 0; i < nq; i++)
        for (int k = 0; k < 3; k++) {
            char *s = k == 0 ? quads[i].a1 : k == 1 ? quads[i].a2 : quads[i].res;
            if (s[0] != 't' || s[1] < '0' || s[1] > '9' || s[1 + strspn(s + 1, "0123456789")]) continue;
            int j = 0;
            while (j < n && strcmp(seen[j], s)) j++;
            if (j == n) strcpy(seen[n++], s);
            sprintf(s, "t%d", j + 1);
        }

    printf("\n%-8s%-10s%-10s%-10s%-10s%-12s\n-----------------------------------------------------------\n",
           "Index", "Label", "Operator", "Operand1", "Operand2", "Result");
    for (int i = 0; i < nq; i++)
        printf("%-8d%-10s%-10s%-10s%-10s%-12s\n", i, quads[i].label, quads[i].op, quads[i].a1, quads[i].a2, quads[i].res);
    return 0;
}

%{
/*
 * parser.y - HLL to Quadruples (with Goto column)
 *
 * Reads input.txt (high level code) and prints the quadruple table.
 *   ./hll2quad        Goto column shows labels (L1, L2)
 *   ./hll2quad -n     Goto column shows row numbers
 *
 * For loop flow:
 *   init
 *   Lcond:                      <- label before condition
 *   cond -> temps
 *   ifFalse t goto Lend
 *   body
 *   increment
 *   goto Lcond                  <- pure goto, nothing else
 *   Lend:
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_ROWS 500
#define LEN      32

typedef struct { char op[LEN], arg1[LEN], arg2[LEN], result[LEN], go[LEN]; } Quad;
typedef struct { Quad *q; int n; } Chunk;

static Quad quads[MAX_ROWS];
static int  nquads = 0, tcount = 0, lcount = 0;

int  yylex(void);
void yyerror(const char *s);
extern int   yylineno;
extern FILE *yyin;

static char *newtemp(void)  { char b[LEN]; snprintf(b, LEN, "t%d", ++tcount); return strdup(b); }
static char *newlabel(void) { char b[LEN]; snprintf(b, LEN, "L%d", ++lcount); return strdup(b); }

static void emit(const char *op, const char *a1, const char *a2,
                 const char *res, const char *go)
{
    if (nquads >= MAX_ROWS) { fprintf(stderr, "Too many quadruples.\n"); exit(1); }
    Quad *q = &quads[nquads++];
    snprintf(q->op, LEN, "%s", op);
    snprintf(q->arg1, LEN, "%s", a1);
    snprintf(q->arg2, LEN, "%s", a2);
    snprintf(q->result, LEN, "%s", res);
    snprintf(q->go, LEN, "%s", go);
}

static void emit_label(const char *l) { emit("label", "-", "-", l, "-"); }

static char *binop(const char *op, const char *a, const char *b)
{
    char *t = newtemp();
    emit(op, a, b, t, "-");
    return t;
}

static char *unop(const char *op, const char *a)
{
    char *t = newtemp();
    emit(op, a, "-", t, "-");
    return t;
}

/* x op= v   ->   t = x op v ; x = t */
static void compound(const char *op, const char *x, const char *v)
{
    char *t = binop(op, x, v);
    emit("=", t, "-", x, "-");
}

/* Remove quads from 'start' onward and keep them (for the for-increment). */
static Chunk *cut_from(int start)
{
    Chunk *c = malloc(sizeof *c);
    c->n = nquads - start;
    c->q = malloc((c->n ? c->n : 1) * sizeof(Quad));
    memcpy(c->q, &quads[start], c->n * sizeof(Quad));
    nquads = start;
    return c;
}

static void paste(Chunk *c)
{
    for (int i = 0; i < c->n; i++) {
        if (nquads >= MAX_ROWS) { fprintf(stderr, "Too many quadruples.\n"); exit(1); }
        quads[nquads++] = c->q[i];
    }
    free(c->q);
    free(c);
}
%}


%union {
    char  *str;
    int    pos;
    void  *chunk;
}

%token <str> ID NUM
%token FOR WHILE IF ELSE TYPE
%token AND OR NOT EQ NE LE GE INC DEC PLUSEQ MINUSEQ MULEQ DIVEQ

%type <str> expr if_head

%left OR
%left AND
%left EQ NE
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/' '%'
%right NOT UMINUS

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%

program
    : stmt_list
    ;

stmt_list
    : stmt_list stmt
    |
    ;

stmt
    : assign ';'
    | TYPE ID ';'                       /* plain declaration, no quad */
    | for_stmt
    | while_stmt
    | if_stmt
    | '{' stmt_list '}'
    | ';'
    ;

/* ---------- assignments ---------- */
assign
    : ID '=' expr       { emit("=", $3, "-", $1, "-"); }
    | TYPE ID '=' expr  { emit("=", $4, "-", $2, "-"); }
    | ID PLUSEQ  expr   { compound("+", $1, $3); }
    | ID MINUSEQ expr   { compound("-", $1, $3); }
    | ID MULEQ   expr   { compound("*", $1, $3); }
    | ID DIVEQ   expr   { compound("/", $1, $3); }
    | ID INC            { compound("+", $1, "1"); }
    | ID DEC            { compound("-", $1, "1"); }
    ;

opt_assign
    : assign
    |
    ;

/* ---------- for loop ---------- */
for_stmt
    : FOR '(' opt_assign ';'                              /* 1-4: init quads emitted */
      { $<str>$ = newlabel(); emit_label($<str>$); }      /* 5: Lcond                */
      expr ';'                                            /* 6,7: condition quads    */
      { $<str>$ = newlabel();
        emit("ifFalse", $6, "-", "-", $<str>$); }         /* 8: Lend + ifFalse       */
      { $<pos>$ = nquads; }                               /* 9: where increment starts */
      opt_assign ')'                                      /* 10,11: increment        */
      { $<chunk>$ = cut_from($<pos>9); }                  /* 12: hold increment aside */
      stmt                                                /* 13: body                */
      { paste((Chunk *)$<chunk>12);                                /* increment after body    */
        emit("goto", "-", "-", "-", $<str>5);             /* back to condition       */
        emit_label($<str>8); }                            /* exit label              */
    ;

/* ---------- while loop ---------- */
while_stmt
    : WHILE '('
      { $<str>$ = newlabel(); emit_label($<str>$); }      /* 3: Lcond */
      expr ')'                                            /* 4,5      */
      { $<str>$ = newlabel();
        emit("ifFalse", $4, "-", "-", $<str>$); }         /* 6: Lend  */
      stmt                                                /* 7        */
      { emit("goto", "-", "-", "-", $<str>3);
        emit_label($<str>6); }
    ;

/* ---------- if / else ---------- */
if_head
    : IF '(' expr ')'
      { $$ = newlabel(); emit("ifFalse", $3, "-", "-", $$); }
    ;

if_stmt
    : if_head stmt %prec LOWER_THAN_ELSE
      { emit_label($1); }
    | if_head stmt ELSE
      { $<str>$ = newlabel();
        emit("goto", "-", "-", "-", $<str>$);
        emit_label($1); }
      stmt
      { emit_label($<str>4); }
    ;

/* ---------- expressions: arithmetic, relational, logical ---------- */
expr
    : expr '+' expr     { $$ = binop("+",  $1, $3); }
    | expr '-' expr     { $$ = binop("-",  $1, $3); }
    | expr '*' expr     { $$ = binop("*",  $1, $3); }
    | expr '/' expr     { $$ = binop("/",  $1, $3); }
    | expr '%' expr     { $$ = binop("%",  $1, $3); }
    | expr '<' expr     { $$ = binop("<",  $1, $3); }
    | expr '>' expr     { $$ = binop(">",  $1, $3); }
    | expr LE  expr     { $$ = binop("<=", $1, $3); }
    | expr GE  expr     { $$ = binop(">=", $1, $3); }
    | expr EQ  expr     { $$ = binop("==", $1, $3); }
    | expr NE  expr     { $$ = binop("!=", $1, $3); }
    | expr AND expr     { $$ = binop("&&", $1, $3); }
    | expr OR  expr     { $$ = binop("||", $1, $3); }
    | NOT expr          { $$ = unop("!",     $2); }
    | '!' expr %prec NOT { $$ = unop("!",    $2); }
    | '-' expr %prec UMINUS { $$ = unop("uminus", $2); }
    | '(' expr ')'      { $$ = $2; }
    | ID                { $$ = $1; }
    | NUM               { $$ = $1; }
    ;

%%

void yyerror(const char *s)
{
    fprintf(stderr, "Line %d: %s\n", yylineno, s);
}

int main(int argc, char *argv[])
{
    int use_numbers = (argc > 1 && strcmp(argv[1], "-n") == 0);

    yyin = fopen("input.txt", "r");
    if (!yyin) {
        printf("Could not open input.txt. Place it in the same folder.\n");
        return 1;
    }
    if (yyparse() != 0) return 1;
    fclose(yyin);

    if (use_numbers) {
        for (int i = 0; i < nquads; i++) {
            if (strcmp(quads[i].go, "-") == 0) continue;
            int row = -1;
            for (int j = 0; j < nquads; j++)
                if (strcmp(quads[j].op, "label") == 0 &&
                    strcmp(quads[j].result, quads[i].go) == 0) { row = j; break; }
            if (row >= 0) snprintf(quads[i].go, LEN, "%d", row);
            else printf("Warning: label %s not defined\n", quads[i].go);
        }
    }

    printf("\n%-8s%-10s%-10s%-10s%-10s%-6s\n",
           "Index", "Operator", "Operand1", "Operand2", "Result", "Goto");
    printf("-----------------------------------------------------\n");
    for (int i = 0; i < nquads; i++)
        printf("%-8d%-10s%-10s%-10s%-10s%-6s\n", i, quads[i].op, quads[i].arg1,
               quads[i].arg2, quads[i].result, quads[i].go);
    return 0;
}

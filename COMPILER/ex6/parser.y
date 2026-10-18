%{
/*
 * parser.y - HLL to Quadruples (Label column, jumps in Result)
 *
 * Reads input.txt (high level code) and prints the quadruple table.
 *   ./hll2quad        jumps shown as  goto L1
 *   ./hll2quad -n     jumps shown as  goto <row number>
 *
 * For loop flow:
 *   init
 *   Lcond: cond                 <- label sits on the condition row
 *   if t goto Lbody
 *   goto Lend
 *   Lbody: body
 *   increment
 *   goto Lcond                  <- pure goto, nothing else
 *   Lend:                       <- label sits on the next row
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_ROWS 500
#define LEN      32

typedef struct { char label[64], op[LEN], arg1[LEN], arg2[LEN], result[LEN]; } Quad;
typedef struct { Quad *q; int n; } Chunk;

static Quad quads[MAX_ROWS];
static int  nquads = 0, tcount = 0, lcount = 0;
static char pending[64] = "";   /* label(s) waiting to attach to the next quad */

int  yylex(void);
void yyerror(const char *s);
extern int   yylineno;
extern FILE *yyin;

static char *newtemp(void)  { char b[LEN]; snprintf(b, LEN, "t%d", ++tcount); return strdup(b); }
static char *newlabel(void) { char b[LEN]; snprintf(b, LEN, "L%d", ++lcount); return strdup(b); }

/* If 'go' is given, the jump target goes into the Result column as "goto Lx". */
static void emit(const char *op, const char *a1, const char *a2,
                 const char *res, const char *go)
{
    if (nquads >= MAX_ROWS) { fprintf(stderr, "Too many quadruples.\n"); exit(1); }
    Quad *q = &quads[nquads++];
    snprintf(q->label, sizeof q->label, "%s", pending[0] ? pending : "-");
    pending[0] = '\0';
    snprintf(q->op, LEN, "%s", op);
    snprintf(q->arg1, LEN, "%s", a1);
    snprintf(q->arg2, LEN, "%s", a2);
    if (strcmp(go, "-") != 0) snprintf(q->result, LEN, "goto %s", go);
    else                      snprintf(q->result, LEN, "%s", res);
}

/* A label does not make a row; it sticks to the next quad emitted. */
static void emit_label(const char *l)
{
    size_t n = strlen(pending);
    if (n) snprintf(pending + n, sizeof pending - n, ",%s", l);
    else   snprintf(pending, sizeof pending, "%s", l);
}

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
    /* a label waiting for the body may have landed on the first increment quad: give it back */
    if (c->n > 0 && strcmp(c->q[0].label, "-") != 0) {
        snprintf(pending, sizeof pending, "%s", c->q[0].label);
        strcpy(c->q[0].label, "-");
    }
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
      { char *body = newlabel();
        $<str>$ = newlabel();
        emit("if",   $6, "-", "-", body);                 /* if cond goto Lbody      */
        emit("goto", "-", "-", "-", $<str>$);             /* goto Lend               */
        emit_label(body); }                               /* 8: Lbody on first body row; value = Lend */
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
      { char *body = newlabel();
        $<str>$ = newlabel();
        emit("if",   $4, "-", "-", body);
        emit("goto", "-", "-", "-", $<str>$);
        emit_label(body); }                               /* 6: value = Lend */
      stmt                                                /* 7        */
      { emit("goto", "-", "-", "-", $<str>3);
        emit_label($<str>6); }
    ;

/* ---------- if / else ---------- */
if_head
    : IF '(' expr ')'
      { char *body = newlabel();
        $$ = newlabel();
        emit("if",   $3, "-", "-", body);
        emit("goto", "-", "-", "-", $$);
        emit_label(body); }
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

static int has_label(const Quad *q, const char *name)
{
    char buf[64], *t;
    snprintf(buf, sizeof buf, "%s", q->label);
    for (t = strtok(buf, ","); t; t = strtok(NULL, ","))
        if (strcmp(t, name) == 0) return 1;
    return 0;
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

    if (pending[0])                       /* label at the very end of the program */
        emit("-", "-", "-", "-", "-");

    if (use_numbers) {                    /* goto Lx  ->  goto <row number> */
        for (int i = 0; i < nquads; i++) {
            if (strncmp(quads[i].result, "goto ", 5) != 0) continue;
            char name[LEN];
            snprintf(name, LEN, "%s", quads[i].result + 5);
            int row = -1;
            for (int j = 0; j < nquads; j++)
                if (has_label(&quads[j], name)) { row = j; break; }
            if (row >= 0) snprintf(quads[i].result, LEN, "goto %d", row);
            else printf("Warning: label %s not defined\n", name);
        }
    }

    printf("\n%-8s%-10s%-10s%-10s%-10s%-12s\n",
           "Index", "Label", "Operator", "Operand1", "Operand2", "Result");
    printf("-----------------------------------------------------------\n");
    for (int i = 0; i < nquads; i++)
        printf("%-8d%-10s%-10s%-10s%-10s%-12s\n", i, quads[i].label, quads[i].op,
               quads[i].arg1, quads[i].arg2, quads[i].result);
    return 0;
}

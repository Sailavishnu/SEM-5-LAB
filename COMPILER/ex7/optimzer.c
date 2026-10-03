/*
 * Code optimizer for three address code (TAC)
 *
 * Applies these 4 optimizations again and again until nothing changes:
 *   1. Common subexpression elimination
 *   2. Constant propagation
 *   3. Copy propagation
 *   4. Constant folding
 *
 * Reads the TAC from input.txt (or from the file name given as the first
 * argument) and prints every change plus the final optimized code.
 *
 *   gcc optimizer.c -o optimizer -lm
 *   ./optimizer
 *
 * Input rules:
 *   - one statement per line, spaces are optional:  t1=a+b   or   t1 = a + b
 *   - a full stop or semicolon at the end of a line is ignored
 *   - supported operators: + - * / % ^
 *   - labels and goto / if-goto lines are copied as they are; at a label
 *     everything learned so far is forgotten, because control can arrive
 *     there from somewhere else.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAXL 100
#define LEN  64

typedef struct {
    int  assign;          /* 1 = assignment, 0 = label / jump line      */
    int  label;           /* 1 = line is a label such as  L1:           */
    char lhs[LEN];
    char op;              /* 0 means a plain copy:  x = y               */
    char a1[LEN];
    char a2[LEN];
    char raw[LEN * 2];    /* text of a non-assignment line              */
} Stmt;

typedef struct { char var[LEN]; char val[LEN]; } Pair;
typedef struct { char op; char a1[LEN]; char a2[LEN]; char res[LEN]; } Expr;

static Stmt prog[MAXL];
static int  n = 0;

/* what we currently know while walking down the code */
static Pair consts[MAXL];  static int nconst = 0;   /* x is the constant 5   */
static Pair copies[MAXL];  static int ncopy  = 0;   /* x is a copy of y      */
static Expr exprs[MAXL];   static int nexpr  = 0;   /* x already holds y op z */

/* ---------- small helpers ---------- */

static int is_num(const char *s)
{
    char *end;
    if (s[0] == '\0') return 0;
    if (!(isdigit((unsigned char)s[0]) || s[0] == '.' ||
          ((s[0] == '-' || s[0] == '+') &&
           (isdigit((unsigned char)s[1]) || s[1] == '.'))))
        return 0;
    strtod(s, &end);
    return *end == '\0';
}

static int is_var(const char *s)
{
    if (!(isalpha((unsigned char)s[0]) || s[0] == '_')) return 0;
    for (int i = 1; s[i]; i++)
        if (!(isalnum((unsigned char)s[i]) || s[i] == '_')) return 0;
    return 1;
}

static void fmt_stmt(const Stmt *s, char *out)
{
    if (!s->assign)  sprintf(out, "%s", s->raw);
    else if (s->op)  sprintf(out, "%s = %s %c %s", s->lhs, s->a1, s->op, s->a2);
    else             sprintf(out, "%s = %s", s->lhs, s->a1);
}

static void fmt_num(double v, char *out)
{
    snprintf(out, LEN, "%.10g", v);
}

static int fold(char op, double x, double y, double *r)
{
    switch (op) {
    case '+': *r = x + y; break;
    case '-': *r = x - y; break;
    case '*': *r = x * y; break;
    case '/': if (y == 0) return 0; *r = x / y; break;
    case '%': if (y == 0) return 0; *r = fmod(x, y); break;
    case '^': *r = pow(x, y); if (isnan(*r)) return 0; break;
    default:  return 0;
    }
    return 1;
}

/* ---------- reading the input ---------- */

static void add_raw(const char *text, int is_label)
{
    Stmt *s = &prog[n++];
    memset(s, 0, sizeof *s);
    s->assign = 0;
    s->label  = is_label;
    snprintf(s->raw, sizeof s->raw, "%s", text);
}

static void parse_line(char *line)
{
    char *p = line;
    while (isspace((unsigned char)*p)) p++;
    for (int i = (int)strlen(p) - 1; i >= 0 && isspace((unsigned char)p[i]); i--)
        p[i] = '\0';
    if (*p == '\0') return;

    /* a label in front of the statement:  L1: t1 = i + 1 */
    char *colon = strchr(p, ':');
    if (colon) {
        char lab[LEN];
        int len = (int)(colon - p) + 1;
        if (len >= LEN) len = LEN - 1;
        strncpy(lab, p, len);
        lab[len] = '\0';
        add_raw(lab, 1);
        p = colon + 1;
        while (isspace((unsigned char)*p)) p++;
        if (*p == '\0') return;
    }

    /* jumps are copied unchanged */
    if (strstr(p, "goto") || strchr(p, '=') == NULL) {
        add_raw(p, 0);
        return;
    }

    /* remove all spaces, and a full stop / semicolon at the end */
    char buf[256];
    int k = 0;
    for (int i = 0; p[i] && k < 250; i++)
        if (!isspace((unsigned char)p[i])) buf[k++] = p[i];
    buf[k] = '\0';
    if (k > 0 && (buf[k - 1] == '.' || buf[k - 1] == ';')) buf[--k] = '\0';

    char *eq = strchr(buf, '=');
    if (eq == NULL || eq == buf || eq[1] == '\0') { add_raw(p, 0); return; }
    *eq = '\0';
    char *rhs = eq + 1;

    Stmt *s = &prog[n++];
    memset(s, 0, sizeof *s);
    s->assign = 1;
    snprintf(s->lhs, LEN, "%.63s", buf);

    /* find the operator; start at index 1 so a leading minus sign stays
       part of the first operand */
    int pos = -1;
    for (int i = 1; rhs[i]; i++)
        if (strchr("+-*/%^", rhs[i])) { pos = i; break; }

    if (pos < 0) {
        s->op = 0;
        snprintf(s->a1, LEN, "%s", rhs);
    } else {
        s->op = rhs[pos];
        rhs[pos] = '\0';
        snprintf(s->a1, LEN, "%s", rhs);
        snprintf(s->a2, LEN, "%s", rhs + pos + 1);
    }
}

/* ---------- knowledge tables ---------- */

static int find_pair(Pair *arr, int cnt, const char *var)
{
    for (int i = 0; i < cnt; i++)
        if (strcmp(arr[i].var, var) == 0) return i;
    return -1;
}

/* variable x is being assigned a new value: forget everything about x */
static void kill(const char *x)
{
    for (int i = 0; i < nconst; ) {
        if (strcmp(consts[i].var, x) == 0) consts[i] = consts[--nconst];
        else i++;
    }
    for (int i = 0; i < ncopy; ) {
        if (strcmp(copies[i].var, x) == 0 || strcmp(copies[i].val, x) == 0)
            copies[i] = copies[--ncopy];
        else i++;
    }
    for (int i = 0; i < nexpr; ) {
        if (strcmp(exprs[i].a1, x) == 0 || strcmp(exprs[i].a2, x) == 0 ||
            strcmp(exprs[i].res, x) == 0)
            exprs[i] = exprs[--nexpr];
        else i++;
    }
}

static void forget_all(void) { nconst = ncopy = nexpr = 0; }

/* constant propagation first, then copy propagation, on one operand */
static int substitute(char *opd, int line)
{
    if (!is_var(opd)) return 0;

    int i = find_pair(consts, nconst, opd);
    if (i >= 0) {
        printf("  Line %d: constant propagation, %s replaced by %s\n",
               line, opd, consts[i].val);
        snprintf(opd, LEN, "%s", consts[i].val);
        return 1;
    }
    i = find_pair(copies, ncopy, opd);
    if (i >= 0) {
        printf("  Line %d: copy propagation, %s replaced by %s\n",
               line, opd, copies[i].val);
        snprintf(opd, LEN, "%s", copies[i].val);
        return 1;
    }
    return 0;
}

/* ---------- one full pass over the code ---------- */

static int run_pass(void)
{
    int changed = 0;
    forget_all();

    for (int i = 0; i < n; i++) {
        Stmt *s = &prog[i];
        int line = i + 1;

        if (!s->assign) {
            if (s->label) forget_all();
            continue;
        }

        /* constant propagation / copy propagation */
        changed |= substitute(s->a1, line);
        if (s->op) changed |= substitute(s->a2, line);

        if (s->op) {
            /* constant folding */
            if (is_num(s->a1) && is_num(s->a2)) {
                double r;
                if (fold(s->op, atof(s->a1), atof(s->a2), &r)) {
                    char val[LEN];
                    fmt_num(r, val);
                    printf("  Line %d: constant folding, %s %c %s = %s\n",
                           line, s->a1, s->op, s->a2, val);
                    s->op = 0;
                    snprintf(s->a1, LEN, "%s", val);
                    s->a2[0] = '\0';
                    changed = 1;
                }
            }
        }

        if (s->op) {
            /* common subexpression elimination */
            for (int j = 0; j < nexpr; j++) {
                Expr *e = &exprs[j];
                int same = (e->op == s->op &&
                            strcmp(e->a1, s->a1) == 0 &&
                            strcmp(e->a2, s->a2) == 0);
                int swapped = ((s->op == '+' || s->op == '*') &&
                               e->op == s->op &&
                               strcmp(e->a1, s->a2) == 0 &&
                               strcmp(e->a2, s->a1) == 0);
                if ((same || swapped) && strcmp(e->res, s->lhs) != 0) {
                    char before[256], after[256];
                    fmt_stmt(s, before);
                    s->op = 0;
                    snprintf(s->a1, LEN, "%s", e->res);
                    s->a2[0] = '\0';
                    fmt_stmt(s, after);
                    printf("  Line %d: common subexpression, %s  becomes  %s\n",
                           line, before, after);
                    changed = 1;
                    break;
                }
            }
        }

        /* remember what this statement tells us */
        kill(s->lhs);
        if (s->op) {
            if (strcmp(s->a1, s->lhs) != 0 && strcmp(s->a2, s->lhs) != 0) {
                Expr *e = &exprs[nexpr++];
                e->op = s->op;
                snprintf(e->a1, LEN, "%s", s->a1);
                snprintf(e->a2, LEN, "%s", s->a2);
                snprintf(e->res, LEN, "%s", s->lhs);
            }
        } else if (is_num(s->a1)) {
            snprintf(consts[nconst].var, LEN, "%s", s->lhs);
            snprintf(consts[nconst].val, LEN, "%s", s->a1);
            nconst++;
        } else if (is_var(s->a1) && strcmp(s->a1, s->lhs) != 0) {
            snprintf(copies[ncopy].var, LEN, "%s", s->lhs);
            snprintf(copies[ncopy].val, LEN, "%s", s->a1);
            ncopy++;
        }
    }
    return changed;
}

static void print_code(void)
{
    char text[256];
    for (int i = 0; i < n; i++) {
        fmt_stmt(&prog[i], text);
        printf("%3d.  %s\n", i + 1, text);
    }
}

int main(int argc, char *argv[])
{
    const char *name = (argc > 1) ? argv[1] : "input.txt";
    FILE *fp = fopen(name, "r");
    char line[256];

    if (fp == NULL) {
        printf("Could not open %s. Place it in the same folder.\n", name);
        return 1;
    }
    while (fgets(line, sizeof line, fp) && n < MAXL - 2)
        parse_line(line);
    fclose(fp);

    printf("Original TAC:\n");
    print_code();

    for (int pass = 1; pass <= 50; pass++) {
        printf("\nPass %d\n", pass);
        if (!run_pass()) {
            printf("  no further change, stopping\n");
            break;
        }
    }

    printf("\nOptimized TAC:\n");
    print_code();
    return 0;
}
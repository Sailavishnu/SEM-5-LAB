/* Short TAC optimizer: constant propagation, copy propagation,
 * constant folding and common subexpression elimination, repeated
 * until nothing changes.   gcc optimizer.c -o optimizer -lm
 *                          ./optimizer input.txt            */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX 200
#define LEN 64

enum { LABEL, ASSIGN, JUMP };

/* LABEL: lhs = name.  ASSIGN: lhs = a [op b].  JUMP: [if a] goto b */
typedef struct { int kind; char lhs[LEN], a[LEN], b[LEN], op; } Stmt;
typedef struct { char key[LEN], val[LEN]; } Pair;

static Stmt prog[MAX]; static int n;
static Pair cons[MAX]; static int nc;   /* x is constant val  */
static Pair cps[MAX];  static int ncp;  /* x is a copy of val */
static Stmt exps[MAX]; static int ne;   /* lhs = a op b       */

static int is_num(const char *s)
{
    char *end;
    if (!*s || !strchr("0123456789+-.", *s)) return 0;
    strtod(s, &end);
    return *end == '\0';
}

static int is_var(const char *s)
{
    if (!(isalpha((unsigned char)*s) || *s == '_')) return 0;
    for (; *s; s++)
        if (!(isalnum((unsigned char)*s) || *s == '_')) return 0;
    return 1;
}

static int is_temp(const char *s)
{
    return s[0] == 't' && s[1] && strspn(s + 1, "0123456789") == strlen(s + 1);
}

static int fold(char op, double x, double y, double *r)
{
    switch (op) {
    case '+': *r = x + y; break;
    case '-': *r = x - y; break;
    case '*': *r = x * y; break;
    case '/': if (y == 0) return 0; *r = x / y; break;
    case '%': if (y == 0) return 0; *r = fmod(x, y); break;
    case '^': *r = pow(x, y); break;
    case '<': *r = x < y; break;
    case '>': *r = x > y; break;
    default: return 0;
    }
    return 1;
}

/* ---------- reading one line ---------- */

static void add(int kind, const char *lhs, const char *a, const char *b, char op)
{
    Stmt *s = &prog[n++];
    snprintf(s->lhs, LEN, "%s", lhs);
    snprintf(s->a, LEN, "%s", a);
    snprintf(s->b, LEN, "%s", b);
    s->kind = kind;
    s->op = op;
}

static void parse_line(char *p)
{
    char buf[256], lab[LEN], c[LEN], t[LEN];
    int k = 0;

    while (isspace((unsigned char)*p)) p++;

    char *colon = strchr(p, ':');                 /* "L1: ..." */
    if (colon && colon - p < LEN) {
        snprintf(lab, sizeof lab, "%.*s", (int)(colon - p), p);
        if (is_var(lab)) {
            add(LABEL, lab, "", "", 0);
            p = colon + 1;
            while (isspace((unsigned char)*p)) p++;
        }
    }
    if (*p == '\0') return;

    if (sscanf(p, "if %63s goto %63s", c, t) == 2) { add(JUMP, "", c, t, 0); return; }
    if (sscanf(p, "goto %63s", t) == 1)            { add(JUMP, "", "", t, 0); return; }

    for (; *p; p++)                               /* drop spaces */
        if (!isspace((unsigned char)*p)) buf[k++] = *p;
    buf[k] = '\0';
    if (k && (buf[k - 1] == '.' || buf[k - 1] == ';')) buf[--k] = '\0';

    char *eq = strchr(buf, '=');
    if (!eq || eq == buf || !eq[1]) return;
    *eq = '\0';

    char *rhs = eq + 1;
    int pos = 0;
    for (int i = 1; rhs[i] && !pos; i++)          /* start at 1: keep a leading '-' */
        if (strchr("+-*/%^<>", rhs[i])) pos = i;

    if (!pos)
        add(ASSIGN, buf, rhs, "", 0);
    else {
        char a[LEN];
        snprintf(a, LEN, "%.*s", pos, rhs);
        add(ASSIGN, buf, a, rhs + pos + 1, rhs[pos]);
    }
}

/* ---------- what we know ---------- */

static int find(Pair *t, int cnt, const char *x)
{
    for (int i = 0; i < cnt; i++)
        if (!strcmp(t[i].key, x)) return i;
    return -1;
}

/* x is being assigned: forget every fact that mentions x */
static void kill(const char *x)
{
    for (int i = 0; i < nc; )
        if (!strcmp(cons[i].key, x)) cons[i] = cons[--nc]; else i++;
    for (int i = 0; i < ncp; )
        if (!strcmp(cps[i].key, x) || !strcmp(cps[i].val, x)) cps[i] = cps[--ncp]; else i++;
    for (int i = 0; i < ne; )
        if (!strcmp(exps[i].a, x) || !strcmp(exps[i].b, x) || !strcmp(exps[i].lhs, x))
            exps[i] = exps[--ne]; else i++;
}

/* replace x by a known constant, or else by a known copy */
static int propagate(char *x, int line)
{
    int i;
    if (!is_var(x)) return 0;
    if ((i = find(cons, nc, x)) >= 0) {
        printf("  Line %d: constant propagation, %s replaced by %s\n", line, x, cons[i].val);
        strcpy(x, cons[i].val);
        return 1;
    }
    if ((i = find(cps, ncp, x)) >= 0) {
        printf("  Line %d: copy propagation, %s replaced by %s\n", line, x, cps[i].val);
        strcpy(x, cps[i].val);
        return 1;
    }
    return 0;
}

/* ---------- one pass over the code ---------- */

static int run_pass(void)
{
    int changed = 0;
    nc = ncp = ne = 0;

    for (int i = 0; i < n; i++) {
        Stmt *s = &prog[i];
        int line = i + 1;

        if (s->kind == LABEL) { nc = ncp = ne = 0; continue; }   /* control can enter here */
        if (s->kind != ASSIGN) continue;

        changed |= propagate(s->a, line);
        if (s->op) changed |= propagate(s->b, line);

        if (s->op && is_num(s->a) && is_num(s->b)) {              /* folding */
            double r;
            if (fold(s->op, atof(s->a), atof(s->b), &r)) {
                char v[LEN];
                snprintf(v, LEN, "%.10g", r);
                printf("  Line %d: constant folding, %s %c %s = %s\n", line, s->a, s->op, s->b, v);
                strcpy(s->a, v);
                s->op = 0;
                s->b[0] = '\0';
                changed = 1;
            }
        }

        if (s->op) {                                              /* CSE */
            for (int j = 0; j < ne; j++) {
                Stmt *e = &exps[j];
                int same = !strcmp(e->a, s->a) && !strcmp(e->b, s->b);
                int swap = (s->op == '+' || s->op == '*') && !strcmp(e->a, s->b) && !strcmp(e->b, s->a);
                if (e->op == s->op && (same || swap) && strcmp(e->lhs, s->lhs)) {
                    printf("  Line %d: common subexpression, %s = %s %c %s  becomes  %s = %s\n",
                           line, s->lhs, s->a, s->op, s->b, s->lhs, e->lhs);
                    strcpy(s->a, e->lhs);
                    s->op = 0;
                    s->b[0] = '\0';
                    changed = 1;
                    break;
                }
            }
        }

        kill(s->lhs);                                             /* remember */
        if (s->op) {
            if (strcmp(s->a, s->lhs) && strcmp(s->b, s->lhs)) exps[ne++] = *s;
        } else if (is_num(s->a)) {
            snprintf(cons[nc].key, LEN, "%s", s->lhs);
            snprintf(cons[nc++].val, LEN, "%s", s->a);
        } else if (is_var(s->a) && strcmp(s->a, s->lhs)) {
            snprintf(cps[ncp].key, LEN, "%s", s->lhs);
            snprintf(cps[ncp++].val, LEN, "%s", s->a);
        }
    }
    return changed;
}

/* ---------- rename temps to t1, t2, ... in order of first use ---------- */

static void renumber(void)
{
    static char from[3 * MAX][LEN];
    int m = 0;

    for (int i = 0; i < n; i++) {
        char *f[3] = { prog[i].lhs, prog[i].a, prog[i].b };
        for (int k = 0; k < 3; k++) {
            int seen = 0;
            for (int j = 0; j < m; j++)
                if (!strcmp(from[j], f[k])) seen = 1;
            if (is_temp(f[k]) && !seen) strcpy(from[m++], f[k]);
        }
    }
    for (int i = 0; i < n; i++) {
        char *f[3] = { prog[i].lhs, prog[i].a, prog[i].b };
        for (int k = 0; k < 3; k++)
            for (int j = 0; j < m; j++)
                if (is_temp(f[k]) && !strcmp(from[j], f[k])) {
                    sprintf(f[k], "t%d", j + 1);
                    break;
                }
    }
}

static void print_code(void)
{
    for (int i = 0; i < n; i++) {
        Stmt *s = &prog[i];
        if (s->kind == LABEL)      printf("%3d.  %s:\n", i + 1, s->lhs);
        else if (s->kind == JUMP && s->a[0])
            printf("%3d.  if %s goto %s\n", i + 1, s->a, s->b);
        else if (s->kind == JUMP)
            printf("%3d.  goto %s\n", i + 1, s->b);
        else if (s->op)            printf("%3d.  %s = %s %c %s\n", i + 1, s->lhs, s->a, s->op, s->b);
        else                       printf("%3d.  %s = %s\n", i + 1, s->lhs, s->a);
    }
}

int main(int argc, char *argv[])
{
    const char *name = argc > 1 ? argv[1] : "input.txt";
    FILE *fp = fopen(name, "r");
    char line[256];

    if (!fp) { printf("Could not open %s\n", name); return 1; }
    while (fgets(line, sizeof line, fp) && n < MAX - 2)
        parse_line(line);
    fclose(fp);

    printf("Original TAC:\n");
    print_code();

    for (int pass = 1; pass <= 50; pass++) {
        printf("\nPass %d\n", pass);
        if (!run_pass()) { printf("  no further change, stopping\n"); break; }
    }

    renumber();
    printf("\nOptimized TAC:\n");
    print_code();
    return 0;
}
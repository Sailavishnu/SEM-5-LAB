
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX 1000
#define BUFSZ 256

typedef enum { NOP, ASGN, ADD, SUB, MUL, DIV, MOD, IFG, GOTO, LAB } Op;
typedef struct { Op op; char a1[BUFSZ], a2[BUFSZ], res[BUFSZ]; } Quad;

Quad code[MAX]; int n = 0;

void trim(char *s) {
    int i = 0, j = 0;
    while (s[i] && isspace((unsigned char)s[i])) i++;
    while (s[i]) s[j++] = s[i++];
    s[j] = '\0';
    while (j > 0 && isspace((unsigned char)s[j-1])) s[--j] = '\0';
}

bool is_num(const char *s) {
    if (!s || !*s) return false;
    int i = (*s == '-' || *s == '+') ? 1 : 0;
    for (; s[i]; i++) if (!isdigit((unsigned char)s[i])) return false;
    return i > (*s == '-' || *s == '+' ? 1 : 0);
}

bool is_underscore(const char *s) { return s && s[0] == '_' && s[1] == '\0'; }

const char* op_str(Op o) {
    switch (o) { case ADD: return "+"; case SUB: return "-"; case MUL: return "*"; case DIV: return "/"; case MOD: return "%"; default: return ""; }
}

void print_header() {
    printf("+--------+----------+----------------+----------------+----------------+\n");
    printf("| Op     | Arg1     | Arg2           | Result         |                |\n");
    printf("+--------+----------+----------------+----------------+----------------+\n");
}

void print_footer() {
    printf("+--------+----------+----------------+----------------+----------------+\n");
}

void qprint(Quad *q) {
    switch (q->op) {
        case NOP: break;
        case ASGN: printf("| %-6s | %-8s | %-14s | %-14s |\n", "=", q->a1, q->a2, q->res); break;
        case ADD:  printf("| %-6s | %-8s | %-14s | %-14s |\n", "+", q->a1, q->a2, q->res); break;
        case SUB:  printf("| %-6s | %-8s | %-14s | %-14s |\n", "-", q->a1, q->a2, q->res); break;
        case MUL:  printf("| %-6s | %-8s | %-14s | %-14s |\n", "*", q->a1, q->a2, q->res); break;
        case DIV:  printf("| %-6s | %-8s | %-14s | %-14s |\n", "/", q->a1, q->a2, q->res); break;
        case MOD:  printf("| %-6s | %-8s | %-14s | %-14s |\n", "%", q->a1, q->a2, q->res); break;
        case IFG:  printf("| %-6s | %-8s | %-14s | %-14s |\n", "if", q->a1, q->a2, q->res); break;
        case GOTO: printf("| %-6s | %-8s | %-14s | %-14s |\n", "goto", q->a1, q->a2, q->res); break;
        case LAB:  printf("| %-6s | %-8s | %-14s | %-14s |\n", "label", q->a1, q->a2, q->res); break;
    }
}

void print_all() {
    print_header();
    for (int i = 0; i < n; i++) if (code[i].op != NOP) qprint(&code[i]);
    print_footer();
}

void parse(const char *fname) {
    FILE *fp = fopen(fname, "r");
    if (!fp) { printf("Cannot open %s\n", fname); exit(1); }
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = 0; trim(line); if (!*line) continue;
        Quad q = {0}; char op[16], a1[BUFSZ], a2[BUFSZ], res[BUFSZ];
        if (sscanf(line, "%15s %255s %255s %255s", op, a1, a2, res) != 4) { printf("Bad line: %s\n", line); continue; }
        strcpy(q.a1, is_underscore(a1) ? "" : a1);
        strcpy(q.a2, is_underscore(a2) ? "" : a2);
        strcpy(q.res, is_underscore(res) ? "" : res);
        if (!strcmp(op, "="))  q.op = ASGN;
        else if (!strcmp(op, "+"))  q.op = ADD;
        else if (!strcmp(op, "-"))  q.op = SUB;
        else if (!strcmp(op, "*"))  q.op = MUL;
        else if (!strcmp(op, "/"))  q.op = DIV;
        else if (!strcmp(op, "%"))  q.op = MOD;
        else if (!strcmp(op, "if"))  q.op = IFG;
        else if (!strcmp(op, "goto")) q.op = GOTO;
        else if (!strcmp(op, "label")) q.op = LAB;
        else { printf("Unknown op: %s\n", op); continue; }
        code[n++] = q;
    }
    fclose(fp);
}

int find(char map[][BUFSZ], int cnt, const char *v) {
    for (int i = 0; i < cnt; i++) if (!strcmp(map[i], v)) return i;
    return -1;
}

bool expr_has(const char *e, const char *v) {
    char b[BUFSZ]; strcpy(b, e); char *t = strtok(b, " ");
    while (t) { if (!strcmp(t, v)) return true; t = strtok(NULL, " "); }
    return false;
}

void build_expr(Quad *q, char *buf) { sprintf(buf, "%s %s %s", q->a1, op_str(q->op), q->a2); }

bool pass_cse() {
    bool changed = false;
    char exprs[MAX][BUFSZ], res[MAX][BUFSZ]; bool valid[MAX] = {0}; int cnt = 0;
    for (int i = 0; i < n; i++) {
        Quad *q = &code[i]; if (q->op == NOP) continue;
        if (*q->res)
            for (int j = 0; j < cnt; j++)
                if (valid[j] && (expr_has(exprs[j], q->res) || !strcmp(res[j], q->res))) valid[j] = false;
        if (q->op >= ADD && q->op <= MOD) {
            char ex[BUFSZ]; build_expr(q, ex);
            int f = -1;
            for (int j = 0; j < cnt; j++) if (valid[j] && !strcmp(exprs[j], ex)) { f = j; break; }
            if (f != -1) {
                printf("  CSE: (%s, %s, %s, %s) -> (=, %s, _, %s)\n", op_str(q->op), q->a1, q->a2, q->res, res[f], q->res);
                q->op = ASGN; strcpy(q->a1, res[f]); strcpy(q->a2, "");
                changed = true;
            } else { strcpy(exprs[cnt], ex); strcpy(res[cnt], q->res); valid[cnt++] = true; }
        }
    }
    return changed;
}

bool pass_const_prop() {
    bool changed = false;
    char vars[256][BUFSZ], vals[256][BUFSZ]; int cnt = 0;
    for (int i = 0; i < n; i++) {
        Quad *q = &code[i]; if (q->op == NOP) continue;
        if (*q->a1 && !is_num(q->a1)) {
            int j = find(vars, cnt, q->a1);
            if (j != -1) { printf("  Const Prop: replace arg1 '%s' -> '%s'\n", q->a1, vals[j]); strcpy(q->a1, vals[j]); changed = true; }
        }
        if (*q->a2 && !is_num(q->a2)) {
            int j = find(vars, cnt, q->a2);
            if (j != -1) { printf("  Const Prop: replace arg2 '%s' -> '%s'\n", q->a2, vals[j]); strcpy(q->a2, vals[j]); changed = true; }
        }
        if (q->op == ASGN && is_num(q->a1)) {
            int j = find(vars, cnt, q->res);
            if (j == -1) { strcpy(vars[cnt], q->res); strcpy(vals[cnt], q->a1); cnt++; }
            else strcpy(vals[j], q->a1);
        } else if (*q->res) {
            int j = find(vars, cnt, q->res);
            if (j != -1) { memmove(&vars[j], &vars[j+1], (cnt-j-1)*BUFSZ); memmove(&vals[j], &vals[j+1], (cnt-j-1)*BUFSZ); cnt--; }
        }
    }
    return changed;
}

bool pass_const_fold() {
    bool changed = false;
    for (int i = 0; i < n; i++) {
        Quad *q = &code[i]; if (q->op < ADD || q->op > MOD) continue;
        if (is_num(q->a1) && is_num(q->a2)) {
            int v1 = atoi(q->a1), v2 = atoi(q->a2), r = 0;
            switch (q->op) { case ADD: r = v1+v2; break; case SUB: r = v1-v2; break; case MUL: r = v1*v2; break; case DIV: r = v1/v2; break; case MOD: r = v1%v2; break; default: break; }
            printf("  Const Fold: (%s, %s, %s, %s) -> (=, %d, _, %s)\n", op_str(q->op), q->a1, q->a2, q->res, r, q->res);
            sprintf(q->a1, "%d", r); q->op = ASGN; strcpy(q->a2, "");
            changed = true;
        }
    }
    return changed;
}

bool pass_copy_prop() {
    bool changed = false;
    char dst[256][BUFSZ], src[256][BUFSZ]; int cnt = 0;
    for (int i = 0; i < n; i++) {
        Quad *q = &code[i]; if (q->op == NOP) continue;
        if (*q->a1 && !is_num(q->a1)) {
            int j = find(dst, cnt, q->a1);
            if (j != -1) { printf("  Copy Prop: replace arg1 '%s' -> '%s'\n", q->a1, src[j]); strcpy(q->a1, src[j]); changed = true; }
        }
        if (*q->a2 && !is_num(q->a2)) {
            int j = find(dst, cnt, q->a2);
            if (j != -1) { printf("  Copy Prop: replace arg2 '%s' -> '%s'\n", q->a2, src[j]); strcpy(q->a2, src[j]); changed = true; }
        }
        if (q->op == ASGN && !is_num(q->a1) && *q->a1) {
            int j = find(dst, cnt, q->res);
            if (j == -1) { strcpy(dst[cnt], q->res); strcpy(src[cnt], q->a1); cnt++; }
            else strcpy(src[j], q->a1);
        } else if (*q->res) {
            int j = find(dst, cnt, q->res);
            if (j != -1) { memmove(&dst[j], &dst[j+1], (cnt-j-1)*BUFSZ); memmove(&src[j], &src[j+1], (cnt-j-1)*BUFSZ); cnt--; }
        }
        for (int j = 0; j < cnt; j++)
            if (!strcmp(q->res, src[j])) { memmove(&dst[j], &dst[j+1], (cnt-j-1)*BUFSZ); memmove(&src[j], &src[j+1], (cnt-j-1)*BUFSZ); cnt--; j--; }
    }
    return changed;
}

void add_live(char live[][BUFSZ], int *lc, const char *v) {
    if (is_num(v)) return;
    for (int i = 0; i < *lc; i++) if (!strcmp(live[i], v)) return;
    strcpy(live[*lc], v); (*lc)++;
}
void rm_live(char live[][BUFSZ], int *lc, const char *v) {
    for (int i = 0; i < *lc; i++) if (!strcmp(live[i], v)) {
        memmove(&live[i], &live[i+1], (*lc-i-1)*BUFSZ); (*lc)--; return;
    }
}
bool live_var(char live[][BUFSZ], int lc, const char *v) {
    for (int i = 0; i < lc; i++) if (!strcmp(live[i], v)) return true;
    return false;
}

bool pass_dce() {
    bool changed = false;
    char live[256][BUFSZ]; int lc = 0;
    int last = -1;
    for (int i = n-1; i >= 0; i--) if (code[i].op != NOP) { last = i; break; }
    if (last != -1 && *code[last].res) add_live(live, &lc, code[last].res);
    for (int i = n-1; i >= 0; i--) {
        Quad *q = &code[i]; if (q->op == NOP) continue;
        bool se = (q->op == IFG || q->op == GOTO || q->op == LAB);
        bool dead = !se && *q->res && !live_var(live, lc, q->res) && !strstr(q->res, "result") && !strstr(q->res, "output");
        if (dead) { printf("  DCE: removed "); qprint(q); q->op = NOP; changed = true; }
        else { if (*q->a1) add_live(live, &lc, q->a1); if (*q->a2) add_live(live, &lc, q->a2); if (*q->res) rm_live(live, &lc, q->res); }
    }
    return changed;
}

int main() {
    parse("input.txt");
    printf("=== Original Quadruples ===\n"); print_all();

    printf("\n=== Pass 1: Common Subexpression Elimination ===\n");
    if (pass_cse()) { printf("\nAfter Pass 1:\n"); print_all(); }
    else printf("  (no changes)\n");

    printf("\n=== Pass 2: Constant Propagation ===\n");
    if (pass_const_prop()) { printf("\nAfter Pass 2:\n"); print_all(); }
    else printf("  (no changes)\n");

    printf("\n=== Pass 3: Constant Folding ===\n");
    if (pass_const_fold()) { printf("\nAfter Pass 3:\n"); print_all(); }
    else printf("  (no changes)\n");

    printf("\n=== Pass 4: Copy Propagation ===\n");
    if (pass_copy_prop()) { printf("\nAfter Pass 4:\n"); print_all(); }
    else printf("  (no changes)\n");

    printf("\n=== Pass 5: Dead Code Elimination ===\n");
    if (pass_dce()) { printf("\nAfter Pass 5:\n"); print_all(); }
    else printf("  (no changes)\n");

    printf("\n=== Final Optimized Quadruples ===\n"); print_all();
    return 0;
}

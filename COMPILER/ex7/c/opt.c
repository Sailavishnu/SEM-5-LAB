#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_QUAD 100

struct quad { char op[10], a1[20], a2[20], res[20]; bool active; };
struct quad q[MAX_QUAD];
int n = 0;

bool is_number(const char *s) {
    if (!s || !*s) return false;
    if (*s == '-' || *s == '+') s++;
    if (!*s) return false;
    while (*s) { if (!isdigit((unsigned char)*s)) return false; s++; }
    return true;
}
// temp = 't' followed by digits only (t, t1, t25)
bool is_temp(const char *s) {
    if (s[0] != 't' && s[0] != 'T') return false;
    for (int i = 1; s[i]; i++) if (!isdigit((unsigned char)s[i])) return false;
    return true;
}
bool is_arg(const char *s) { return s && *s && strcmp(s, "-") != 0; }
bool is_arith(int i) { return strlen(q[i].op) == 1 && strchr("+-*/", q[i].op[0]); }
bool is_ctrl(int i) {
    return !strcmp(q[i].op, "label") || !strcmp(q[i].op, "goto") || !strcmp(q[i].op, "ifFalse");
}

void clean_token(char *str) {
    int j = 0; char temp[50];
    for (int i = 0; str[i]; i++)
        if (str[i]!='(' && str[i]!=')' && str[i]!=',' && str[i]!='"' && !isspace((unsigned char)str[i]))
            temp[j++] = str[i];
    temp[j] = '\0';
    strcpy(str, j == 0 ? "-" : temp);
}

void add_quad(const char *op, const char *a1, const char *a2, const char *res) {
    snprintf(q[n].op, 10, "%s", op);   snprintf(q[n].a1, 20, "%s", a1);
    snprintf(q[n].a2, 20, "%s", a2);   snprintf(q[n].res, 20, "%s", res);
    q[n].active = true; n++;
}

// Accepts:  t1=a+b   |   x=t2   |   = a - x   |   + a b t1
void parse_line(char *line) {
    char s[256]; int k = 0;
    for (int i = 0; line[i]; i++) if (!isspace((unsigned char)line[i])) s[k++] = line[i];
    s[k] = '\0';
    while (k > 0 && (s[k-1] == '.' || s[k-1] == ';')) s[--k] = '\0';   // stray "5." or ";"

    char *eq = strchr(s, '=');
    if (eq && eq != s && !strchr(line, ' ')) {          // infix form
        *eq = '\0';
        char *res = s, *rhs = eq + 1;
        int p = -1;
        for (int i = 1; rhs[i]; i++) if (strchr("+-*/", rhs[i])) { p = i; break; }
        if (p > 0) {
            char a1[20], a2[20], op[2] = { rhs[p], '\0' };
            snprintf(a1, 20, "%.*s", p, rhs);
            snprintf(a2, 20, "%s", rhs + p + 1);
            add_quad(op, a1, a2, res);
        } else add_quad("=", rhs, "-", res);
        return;
    }
    char t1[50]="-", t2[50]="-", t3[50]="-", t4[50]="-";   // quadruple form
    if (sscanf(line, "%s %s %s %s", t1, t2, t3, t4) >= 1) {
        clean_token(t1); clean_token(t2); clean_token(t3); clean_token(t4);
        add_quad(t1, t2, t3, t4);
    }
}

void read_quadruples(FILE *stream) {
    char line[256]; n = 0;
    while (fgets(line, sizeof(line), stream)) {
        line[strcspn(line, "\r\n")] = 0;
        if (!strlen(line)) continue;
        parse_line(line);
    }
}

void print_quads(const char *title) {
    printf("\n%s:\n", title);
    printf("No.\tOp\tArg1\tArg2\tResult\t\tCode\n");
    printf("------------------------------------------------------------\n");
    for (int i = 0; i < n; i++) {
        if (!q[i].active) continue;
        char code[80];
        if (!strcmp(q[i].op, "="))      snprintf(code, 80, "%s = %s", q[i].res, q[i].a1);
        else if (is_arith(i))           snprintf(code, 80, "%s = %s %s %s", q[i].res, q[i].a1, q[i].op, q[i].a2);
        else                            snprintf(code, 80, "%s %s %s %s", q[i].op, q[i].a1, q[i].a2, q[i].res);
        printf("%d\t%s\t%s\t%s\t%s\t\t%s\n", i + 1, q[i].op, q[i].a1, q[i].a2, q[i].res, code);
    }
}

/* ---------- 1. Common Subexpression Elimination ---------- */
void cse() {
    printf("\n>>> STEP 1: Common Subexpression Elimination\n");
    bool any = false;
    for (int i = 0; i < n; i++) {
        if (!q[i].active || !is_arith(i)) continue;
        for (int j = i - 1; j >= 0; j--) {
            if (!q[j].active) continue;
            if (is_ctrl(j)) break;
            if (!strcmp(q[j].op, q[i].op) && !strcmp(q[j].a1, q[i].a1) && !strcmp(q[j].a2, q[i].a2)) {
                printf("  Quad %d same as Quad %d -> %s = %s\n", i+1, j+1, q[i].res, q[j].res);
                strcpy(q[i].op, "="); strcpy(q[i].a1, q[j].res); strcpy(q[i].a2, "-");
                any = true; break;
            }
            if (!strcmp(q[j].res, q[i].a1) || !strcmp(q[j].res, q[i].a2)) break;
        }
    }
    if (!any) printf("  (no change)\n");
    print_quads("After Common Subexpression Elimination");
}

/* constants=true -> propagate numeric values; false -> propagate variables (copies) */
bool propagate(bool constants) {
    bool changed = false;
    for (int i = 0; i < n; i++) {
        if (!q[i].active || strcmp(q[i].op, "=")) continue;
        char *var = q[i].res, *val = q[i].a1;
        if (is_number(val) != constants || !strcmp(var, val)) continue;
        for (int j = i + 1; j < n; j++) {
            if (!q[j].active) continue;
            if (!strcmp(q[j].op, "label")) break;
            if (is_arg(q[j].a1) && !strcmp(q[j].a1, var)) {
                printf("  Quad %d: arg1 %s -> %s\n", j+1, var, val);
                strcpy(q[j].a1, val); changed = true;
            }
            if (is_arg(q[j].a2) && !strcmp(q[j].a2, var)) {
                printf("  Quad %d: arg2 %s -> %s\n", j+1, var, val);
                strcpy(q[j].a2, val); changed = true;
            }
            if (is_ctrl(j)) break;
            if (!strcmp(q[j].res, var) || !strcmp(q[j].res, val)) break;
        }
    }
    return changed;
}

/* ---------- 2. Constant Propagation ---------- */
void constant_propagation() {
    printf("\n>>> STEP 2: Constant Propagation\n");
    if (!propagate(true)) printf("  (no change)\n");
    print_quads("After Constant Propagation");
}

bool fold_once() {
    bool changed = false;
    for (int i = 0; i < n; i++) {
        if (!q[i].active || !is_arith(i)) continue;
        char o = q[i].op[0], a[20], b[20];
        strcpy(a, q[i].a1); strcpy(b, q[i].a2);
        if (is_number(a) && is_number(b)) {
            int x = atoi(a), y = atoi(b), r;
            if (o=='+') r = x+y; else if (o=='-') r = x-y; else if (o=='*') r = x*y;
            else { if (!y) continue; r = x/y; }
            printf("  Quad %d: %s %c %s = %d\n", i+1, a, o, b, r);
            sprintf(q[i].a1, "%d", r);
        } else if ((o=='*' && !strcmp(b,"1")) || (o=='+' && !strcmp(b,"0")) ||
                   (o=='-' && !strcmp(b,"0")) || (o=='/' && !strcmp(b,"1"))) {
            printf("  Quad %d: %s %c %s -> %s\n", i+1, a, o, b, a);
        } else if ((o=='*' && !strcmp(a,"1")) || (o=='+' && !strcmp(a,"0"))) {
            printf("  Quad %d: %s %c %s -> %s\n", i+1, a, o, b, b);
            strcpy(q[i].a1, b);
        } else if (o=='*' && (!strcmp(a,"0") || !strcmp(b,"0"))) {
            printf("  Quad %d: %s * %s -> 0\n", i+1, a, b);
            strcpy(q[i].a1, "0");
        } else continue;
        strcpy(q[i].op, "="); strcpy(q[i].a2, "-");
        changed = true;
    }
    return changed;
}

/* ---------- 3. Constant Folding ---------- */
void constant_folding() {
    printf("\n>>> STEP 3: Constant Folding\n");
    bool any = false;
    while (fold_once()) { any = true; propagate(true); }
    if (!any) printf("  (no change)\n");
    print_quads("After Constant Folding");
}

/* ---------- 4. Copy Propagation ---------- */
void copy_propagation() {
    printf("\n>>> STEP 4: Copy Propagation\n");
    if (!propagate(false)) printf("  (no change)\n");
    print_quads("After Copy Propagation");
}

/* ---------- 5. Dead Code Elimination ---------- */
void dead_code_elimination() {
    printf("\n>>> STEP 5: Dead Code Elimination\n");
    bool any = false, changed;
    do {
        changed = false;
        for (int i = 0; i < n; i++) {                       // overwritten stores
            if (!q[i].active || strcmp(q[i].op, "=")) continue;
            bool read = false, redef = false;
            for (int j = i + 1; j < n; j++) {
                if (!q[j].active) continue;
                if (is_ctrl(j)) { read = true; break; }
                if ((is_arg(q[j].a1) && !strcmp(q[j].a1, q[i].res)) ||
                    (is_arg(q[j].a2) && !strcmp(q[j].a2, q[i].res))) { read = true; break; }
                if (!strcmp(q[j].res, q[i].res)) { redef = true; break; }
            }
            if (!read && redef) {
                printf("  Quad %d removed: %s overwritten before use\n", i+1, q[i].res);
                q[i].active = false; changed = true;
            }
        }
        for (int i = 0; i < n; i++) {                       // unused temporaries
            if (!q[i].active || !is_temp(q[i].res)) continue;
            bool used = false;
            for (int j = 0; j < n && !used; j++) {
                if (!q[j].active || j == i) continue;
                used = (is_arg(q[j].a1) && !strcmp(q[j].a1, q[i].res)) ||
                       (is_arg(q[j].a2) && !strcmp(q[j].a2, q[i].res));
            }
            if (!used) {
                printf("  Quad %d removed: %s never used\n", i+1, q[i].res);
                q[i].active = false; changed = true;
            }
        }
        any |= changed;
    } while (changed);
    if (!any) printf("  (no change)\n");
    print_quads("Final Optimized Quadruples");
}

int main(int argc, char *argv[]) {
    FILE *fp = NULL;
    if (argc > 1) {
        if (!strcmp(argv[1], "-")) fp = stdin;
        else if (!(fp = fopen(argv[1], "r"))) { printf("Error opening file %s\n", argv[1]); return 1; }
    } else {
        fp = fopen("input.txt", "r");
        if (!fp) fp = stdin;
    }
    read_quadruples(fp);
    if (fp != stdin) fclose(fp);

    print_quads("Original Quadruples");
    cse();
    constant_propagation();
    constant_folding();
    copy_propagation();
    dead_code_elimination();
    return 0;
}
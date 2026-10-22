#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

typedef struct { char lhs[16], a[16], b[16]; char op; int id; char why[96]; } I;
I c[100];
int n = 0, chg = 0;

int isnum(char *s) { return isdigit(s[0]) || s[0] == '.' || (s[0] == '-' && isdigit(s[1])); }
int istemp(char *s) { return s[0] == 't' && isdigit(s[1]); }

void fmt(I *p, char *buf) {
    if (p->op) sprintf(buf, "%s = %s %c %s", p->lhs, p->a, p->op, p->b);
    else       sprintf(buf, "%s = %s", p->lhs, p->a);
}
void note(I *p, const char *f, ...) {
    size_t l = strlen(p->why);
    if (l) { strcat(p->why, "; "); l += 2; }
    va_list ap; va_start(ap, f);
    vsnprintf(p->why + l, sizeof p->why - l, f, ap);
    va_end(ap);
}
void logchg(I *p, char *old) {
    if (!p->why[0]) return;
    char nw[64]; fmt(p, nw);
    printf("  Line %d: %s  =>  %s\n", p->id, old, nw);
    p->why[0] = 0; chg++;
}
void show(const char *title) {
    if (!chg) printf("  (no change)\n");
    printf("-- code after %s --\n", title);
    for (int i = 0; i < n; i++) { char s[64]; fmt(&c[i], s); printf("  %2d: %s\n", c[i].id, s); }
    chg = 0;
}

void parse(char *s) {
    char t[64]; int k = 0;
    for (int i = 0; s[i]; i++) if (!isspace(s[i])) t[k++] = s[i];
    t[k] = 0;
    char *eq = strchr(t, '=');
    if (!k || !eq) return;
    *eq = 0;
    I *p = &c[n]; memset(p, 0, sizeof(I));
    p->id = ++n;
    strcpy(p->lhs, t);
    char *r = eq + 1;
    for (int i = 1; r[i]; i++)
        if (strchr("+-*/", r[i])) {
            p->op = r[i]; r[i] = 0;
            strcpy(p->a, r); strcpy(p->b, r + i + 1);
            return;
        }
    strcpy(p->a, r);
}

void cse() {
    for (int i = 0; i < n; i++) {
        if (!c[i].op) continue;
        char old[64]; fmt(&c[i], old);
        for (int j = i - 1; j >= 0; j--) {
            if (!strcmp(c[j].lhs, c[i].a) || !strcmp(c[j].lhs, c[i].b)) break;
            if (!c[j].op || c[j].op != c[i].op) continue;
            int same = !strcmp(c[j].a, c[i].a) && !strcmp(c[j].b, c[i].b);
            if (!same && (c[i].op == '+' || c[i].op == '*'))
                same = !strcmp(c[j].a, c[i].b) && !strcmp(c[j].b, c[i].a);
            if (!same) continue;
            int ok = 1;
            for (int k = j + 1; k < i; k++) if (!strcmp(c[k].lhs, c[j].lhs)) ok = 0;
            if (ok) {
                note(&c[i], "'%s %c %s' already computed in line %d (%s)", c[i].a, c[i].op, c[i].b, c[j].id, c[j].lhs);
                c[i].op = 0; strcpy(c[i].a, c[j].lhs); c[i].b[0] = 0;
            }
            break;
        }
        logchg(&c[i], old);
    }
}

/* mode 0: const propagation, 1: const folding, 2: copy propagation */
void prop(int mode) {
    char key[50][16], val[50][16]; int m = 0;
    for (int i = 0; i < n; i++) {
        I *p = &c[i]; char old[64]; fmt(p, old);
        for (int k = 0; k < m; k++) {
            if (!strcmp(p->a, key[k])) { note(p, "%s is %s %s", key[k], mode == 2 ? "a copy of" : "constant", val[k]); strcpy(p->a, val[k]); }
            if (p->op && !strcmp(p->b, key[k])) { note(p, "%s is %s %s", key[k], mode == 2 ? "a copy of" : "constant", val[k]); strcpy(p->b, val[k]); }
        }
        if (mode == 1 && p->op && isnum(p->a) && isnum(p->b)) {
            double x = atof(p->a), y = atof(p->b), r = 0; int ok = 1;
            switch (p->op) {
                case '+': r = x + y; break;
                case '-': r = x - y; break;
                case '*': r = x * y; break;
                case '/': if (y) r = x / y; else ok = 0; break;
            }
            if (ok) {
                char tmp[32]; sprintf(tmp, "%g", r);
                note(p, "folded %s %c %s = %s", p->a, p->op, p->b, tmp);
                strcpy(p->a, tmp); p->op = 0; p->b[0] = 0;
            }
        }
        for (int k = 0; k < m; k++)
            if (!strcmp(key[k], p->lhs) || !strcmp(val[k], p->lhs)) {
                strcpy(key[k], key[m - 1]); strcpy(val[k], val[m - 1]); m--; k--;
            }
        if (!p->op && strcmp(p->a, p->lhs) &&
            ((mode != 2 && isnum(p->a)) || (mode == 2 && !isnum(p->a)))) {
            strcpy(key[m], p->lhs); strcpy(val[m], p->a); m++;
        }
        logchg(p, old);
    }
}

void dce() {
    char live[100][16]; int m = 0, dead[100] = {0};
    for (int i = 0; i < n; i++)
        if (!istemp(c[i].lhs)) {
            int f = 0;
            for (int k = 0; k < m; k++) if (!strcmp(live[k], c[i].lhs)) f = 1;
            if (!f) strcpy(live[m++], c[i].lhs);
        }
    for (int i = n - 1; i >= 0; i--) {
        int idx = -1;
        for (int k = 0; k < m; k++) if (!strcmp(live[k], c[i].lhs)) idx = k;
        if (idx < 0) {
            char s[64]; fmt(&c[i], s);
            printf("  Line %d removed: %s\n", c[i].id, s);
            dead[i] = 1; chg++; continue;
        }
        strcpy(live[idx], live[--m]);
        char *u[2] = { c[i].a, c[i].op ? c[i].b : "" };
        for (int q = 0; q < 2; q++) {
            if (!u[q][0] || isnum(u[q])) continue;
            int f = 0;
            for (int k = 0; k < m; k++) if (!strcmp(live[k], u[q])) f = 1;
            if (!f) strcpy(live[m++], u[q]);
        }
    }
    int k = 0;
    for (int i = 0; i < n; i++) if (!dead[i]) c[k++] = c[i];
    n = k;
}

int main() {
    FILE *f = fopen("input.txt", "r");
    if (!f) { printf("input.txt not found\n"); return 1; }
    char s[64];
    while (fgets(s, sizeof s, f)) parse(s);
    fclose(f);

    printf("===== INPUT =====\n"); show("reading input");
    printf("\n===== 1. Common Subexpression Elimination =====\n"); cse();   show("CSE");
    printf("\n===== 2. Constant Propagation =====\n");             prop(0); show("constant propagation");
    printf("\n===== 3. Constant Folding =====\n");                 prop(1); show("constant folding");
    printf("\n===== 4. Copy Propagation =====\n");                 prop(2); show("copy propagation");
    printf("\n===== 5. Dead Code Elimination =====\n");            dce();   show("DCE");
    return 0;
}   
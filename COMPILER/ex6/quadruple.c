/*
 * TAC to Quadruples generator (with a Goto column)
 *
 * Reads the TAC from the file  input.txt  (same folder) and prints the
 * quadruple table on the screen.
 *
 * Usage:
 *   ./quadruples          Goto column shows labels  (L1, L2)
 *   ./quadruples -n       Goto column shows row numbers instead
 *
 * Put one TAC statement per line in input.txt.
 * Tokens must be separated by spaces:  t1 = i + 1
 *
 * Supported TAC forms:
 *   x = y op z              (op can be + - * / % etc.)
 *   x = y                   (copy)
 *   if a relop b goto L     (relop can be < > <= >= == !=)
 *   goto L
 *   L:                      (label alone, or in front of a statement)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_ROWS   100
#define MAX_TOKENS 10
#define LEN        32

typedef struct {
    char op[LEN];
    char arg1[LEN];
    char arg2[LEN];
    char result[LEN];
    char go[LEN];
} Quad;

typedef struct {
    char name[LEN];
    int  row;
} Label;

static Quad  quads[MAX_ROWS];
static Label labels[MAX_ROWS];
static int   nquads = 0;
static int   nlabels = 0;

static void init_quad(Quad *q)
{
    strcpy(q->op, "-");
    strcpy(q->arg1, "-");
    strcpy(q->arg2, "-");
    strcpy(q->result, "-");
    strcpy(q->go, "-");
}

static int find_label(const char *name)
{
    for (int i = 0; i < nlabels; i++)
        if (strcmp(labels[i].name, name) == 0)
            return labels[i].row;
    return -1;
}

/* Fill one quadruple from the statement tokens. Returns 1 on success. */
static int parse_statement(char tok[][LEN], int n, Quad *q)
{
    if (n == 2 && strcmp(tok[0], "goto") == 0) {
        strcpy(q->op, "goto");
        strcpy(q->go, tok[1]);
        return 1;
    }
    if (n == 6 && strcmp(tok[0], "if") == 0 && strcmp(tok[4], "goto") == 0) {
        snprintf(q->op, LEN, "if%s", tok[2]);   /* e.g. if< */
        strcpy(q->arg1, tok[1]);
        strcpy(q->arg2, tok[3]);
        strcpy(q->go, tok[5]);
        return 1;
    }
    if (n == 3 && strcmp(tok[1], "=") == 0) {   /* x = y */
        strcpy(q->op, "=");
        strcpy(q->arg1, tok[2]);
        strcpy(q->result, tok[0]);
        return 1;
    }
    if (n == 5 && strcmp(tok[1], "=") == 0) {   /* x = y op z */
        strcpy(q->op, tok[3]);
        strcpy(q->arg1, tok[2]);
        strcpy(q->arg2, tok[4]);
        strcpy(q->result, tok[0]);
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int use_numbers = (argc > 1 && strcmp(argv[1], "-n") == 0);
    char line[256];

    FILE *fp = fopen("input.txt", "r");
    if (fp == NULL) {
        printf("Could not open input.txt. Place it in the same folder.\n");
        return 1;
    }

    while (fgets(line, sizeof line, fp)) {
        char tok[MAX_TOKENS][LEN];
        int n = 0;

        for (char *p = strtok(line, " \t\r\n"); p && n < MAX_TOKENS;
             p = strtok(NULL, " \t\r\n")) {
            strncpy(tok[n], p, LEN - 1);
            tok[n][LEN - 1] = '\0';
            n++;
        }

        if (n == 0) continue;                       /* blank line */
        if (strcmp(tok[0], "end") == 0) break;
        if (nquads >= MAX_ROWS) { printf("Too many lines.\n"); break; }

        /* A leading token ending in ':' is a label on this line. */
        int start = 0;
        size_t len = strlen(tok[0]);
        if (tok[0][len - 1] == ':') {
            tok[0][len - 1] = '\0';
            strcpy(labels[nlabels].name, tok[0]);
            labels[nlabels].row = nquads;           /* row about to be added */
            nlabels++;
            start = 1;
        }

        Quad *q = &quads[nquads];
        init_quad(q);

        if (start < n) {                            /* label + statement */
            if (!parse_statement(tok + start, n - start, q)) {
                printf("Skipping unrecognised line: %s\n", tok[start]);
                continue;
            }
        }
        /* else: label alone, row stays all "-" */
        nquads++;
    }

    fclose(fp);

    /* Optionally replace labels in the Goto column by row numbers. */
    if (use_numbers) {
        for (int i = 0; i < nquads; i++) {
            if (strcmp(quads[i].go, "-") == 0) continue;
            int row = find_label(quads[i].go);
            if (row >= 0)
                snprintf(quads[i].go, LEN, "%d", row);
            else
                printf("Warning: label %s not defined\n", quads[i].go);
        }
    }

    printf("\n%-8s%-10s%-10s%-10s%-10s%-6s\n",
           "Index", "Operator", "Operand1", "Operand2", "Result", "Goto");
    printf("-----------------------------------------------------\n");
    for (int i = 0; i < nquads; i++) {
        printf("%-8d%-10s%-10s%-10s%-10s%-6s\n", i,
               quads[i].op, quads[i].arg1, quads[i].arg2,
               quads[i].result, quads[i].go);
    }
    return 0;
}
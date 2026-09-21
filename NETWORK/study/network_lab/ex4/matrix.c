#include <stdio.h>
#include "router.h"

/* prints the prompt (with the last valid letter filled in) and reads one router letter */
char readRouter(const char *prompt, int n) {
    char c;
    printf(prompt, 'A' + n - 1);
    scanf(" %c", &c);
    return c;
}

void printMatrix(int n, int m[MAX][MAX]) {
    printf("\n\t");
    for (int i = 0; i < n; i++)
        printf("%c\t", 'A' + i);
    printf("\n");
    for (int i = 0; i < n; i++) {
        printf("%c\t", 'A' + i);
        for (int j = 0; j < n; j++) {
            if (m[i][j] == INF)
                printf("INF\t");
            else
                printf("%d\t", m[i][j]);
        }
        printf("\n");
    }
}

void displayCostMatrix(int n, int graph[MAX][MAX]) {
    printf("\n========================================\n");
    printf("            COST MATRIX\n");
    printf("========================================\n");
    printMatrix(n, graph);
}

void updateLinkCost(int n, int graph[MAX][MAX]) {
    char c1, c2;
    int r1, r2, newCost;

    c1 = readRouter(SRC_PROMPT, n);
    c2 = readRouter(DEST_PROMPT, n);
    r1 = c1 - 'A';
    r2 = c2 - 'A';
    if (r1 < 0 || r1 >= n || r2 < 0 || r2 >= n) {
        printf("\nInvalid router letters!\n");
        return;
    }
    if (r1 == r2) {
        printf("\nCost from a router to itself is always 0.\n");
        return;
    }

    if (graph[r1][r2] == INF)
        printf("\nPrevious cost between %c and %c: INF\n", c1, c2);
    else
        printf("\nPrevious cost between %c and %c: %d\n", c1, c2, graph[r1][r2]);

    printf("Enter new cost (Enter %d for INF/no connection): ", INF);
    scanf("%d", &newCost);
    graph[r1][r2] = graph[r2][r1] = newCost;          /* links are two-way */
    printf("\nCost matrix updated successfully between %c and %c!\n", c1, c2);
}

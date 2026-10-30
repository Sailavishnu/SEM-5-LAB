#include <stdio.h>
#include "router.h"

void runDistanceVector(int n, int cost[MAX][MAX]) {
    int dist[MAX][MAX], next[MAX][MAX];
    int updated, rounds = 0;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            dist[i][j] = cost[i][j];
            next[i][j] = (cost[i][j] != INF && i != j) ? j : -1;   /* first hop towards j */
        }
    }

    /* keep relaxing until no distance improves any more */
    do {
        updated = 0;
        rounds++;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                for (int k = 0; k < n; k++) {
                    if (dist[i][k] != INF && dist[k][j] != INF && dist[i][k] + dist[k][j] < dist[i][j]) {
                        dist[i][j] = dist[i][k] + dist[k][j];
                        next[i][j] = next[i][k];
                        updated = 1;
                    }
                }
            }
        }
    } while (updated);

    printf("\n========================================\n");
    printf("        DISTANCE VECTOR ROUTING\n");
    printf("========================================\n");
    printf("\nIterations = %d\n", rounds);
    for (int i = 0; i < n; i++) {
        printf("\nRouting Table - %c\n", 'A' + i);
        printf("----------------------------------------\n");
        printf("Destination\tNext Hop\tCost\n");
        for (int j = 0; j < n; j++) {
            printf("%c\t\t", 'A' + j);
            if (i == j)
                printf("-\t\t0\n");
            else if (dist[i][j] == INF)
                printf("-\t\tINF\n");
            else
                printf("%c\t\t%d\n", 'A' + next[i][j], dist[i][j]);
        }
    }

    printf("\n========================================\n");
    printf("      FINAL UPDATED ROUTING TABLE\n");
    printf("          (Distance Vector)\n");
    printf("========================================\n");
    printf("\n--- Final Shortest Path Cost Matrix ---\n");
    printMatrix(n, dist);
}

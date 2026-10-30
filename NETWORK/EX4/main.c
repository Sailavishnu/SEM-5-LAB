#include <stdio.h>
#include "router.h"

/* reads n and the n x n cost matrix; returns 1 on success */
static int readMatrix(int *n, int graph[MAX][MAX]) {
    printf("\nEnter number of routers (maximum %d): ", MAX);
    scanf("%d", n);
    if (*n <= 0 || *n > MAX) {
        printf("\nInvalid number of routers!\n");
        *n = 0;
        return 0;
    }
    printf("\nEnter the cost matrix row by row. Enter %d for no connection.\n\n", INF);
    for (int i = 0; i < *n; i++) {
        for (int j = 0; j < *n; j++) {
            scanf("%d", &graph[i][j]);
            if (i == j)
                graph[i][j] = 0;
        }
    }
    return 1;
}

int main(void) {
    int n = 0, graph[MAX][MAX], menuChoice, initialized = 0;

    do {
        printf("\n========================================\n");
        printf("      ROUTING ALGORITHM SIMULATOR\n");
        printf("========================================\n");
        printf("1. Enter/Change Full Network Cost Matrix\n2. Display Cost Matrix\n3. Distance Vector Routing\n"
               "4. Link State Routing\n5. Change Cost of Specific Router Pair\n"
               "6. Find Shortest Distance/Path for Specific Pair\n7. Exit\n");
        printf("\nEnter your choice: ");
        scanf("%d", &menuChoice);

        if (menuChoice >= 2 && menuChoice <= 6 && !initialized) {
            printf("\nPlease enter the cost matrix first (Option 1)!\n");
            continue;
        }
        switch (menuChoice) {
            case 1: if (readMatrix(&n, graph)) initialized = 1; break;
            case 2: displayCostMatrix(n, graph);  break;
            case 3: runDistanceVector(n, graph);  break;
            case 4: runLinkState(n, graph);       break;
            case 5: updateLinkCost(n, graph);     break;
            case 6: findShortestRoute(n, graph);  break;
            case 7: printf("\nProgram terminated.\n"); break;
            default: printf("\nInvalid choice! Please try again.\n");
        }
    } while (menuChoice != 7);
    return 0;
}

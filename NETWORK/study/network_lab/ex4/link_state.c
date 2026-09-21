#include <stdio.h>
#include "router.h"

/* unvisited router with the smallest distance (-1 if none is reachable) */
static int getMinimumNode(const int dist[MAX], const int visited[MAX], int n) {
    int min = INF, pos = -1;
    for (int i = 0; i < n; i++) {
        if (!visited[i] && dist[i] < min) {
            min = dist[i];
            pos = i;
        }
    }
    return pos;
}

static void printShortestPath(const int parent[MAX], int node) {
    if (parent[node] != -1) {
        printShortestPath(parent, parent[node]);
        printf(" -> ");
    }
    printf("%c", 'A' + node);
}

/* Dijkstra: fills dist[] and parent[] for the given source */
static void dijkstra(int n, int graph[MAX][MAX], int source, int dist[MAX], int parent[MAX]) {
    int visited[MAX];
    for (int i = 0; i < n; i++) {
        dist[i] = INF;
        visited[i] = 0;
        parent[i] = -1;
    }
    dist[source] = 0;

    for (int count = 0; count < n - 1; count++) {
        int current = getMinimumNode(dist, visited, n);
        if (current == -1)
            break;
        visited[current] = 1;
        for (int i = 0; i < n; i++) {
            if (!visited[i] && graph[current][i] != INF && dist[current] + graph[current][i] < dist[i]) {
                dist[i] = dist[current] + graph[current][i];
                parent[i] = current;
            }
        }
    }
}

void runLinkState(int n, int graph[MAX][MAX]) {
    int dist[MAX], parent[MAX];
    int source = readRouter(SRC_PROMPT, n) - 'A';

    if (source < 0 || source >= n) {
        printf("\nInvalid source router!\n");
        return;
    }
    dijkstra(n, graph, source, dist, parent);

    printf("\n========================================\n");
    printf("          LINK STATE ROUTING\n");
    printf("========================================\n");
    printf("\nSource Router: %c\n", 'A' + source);
    printf("\nDestination\tCost\tShortest Path\n");
    printf("----------------------------------------\n");
    for (int i = 0; i < n; i++) {
        printf("%c\t\t", 'A' + i);
        if (dist[i] == INF)
            printf("INF\tNo Path\n");
        else {
            printf("%d\t", dist[i]);
            printShortestPath(parent, i);
            printf("\n");
        }
    }
}

void findShortestRoute(int n, int graph[MAX][MAX]) {
    int dist[MAX], parent[MAX];
    int source = readRouter(SRC_PROMPT, n) - 'A';
    int dest = readRouter(DEST_PROMPT, n) - 'A';

    if (source < 0 || source >= n || dest < 0 || dest >= n) {
        printf("\nInvalid router letters!\n");
        return;
    }
    dijkstra(n, graph, source, dist, parent);

    printf("\n========================================\n");
    printf("     SHORTEST PATH: %c -> %c\n", 'A' + source, 'A' + dest);
    printf("========================================\n");
    if (dist[dest] == INF)
        printf("No path exists between %c and %c.\n", 'A' + source, 'A' + dest);
    else {
        printf("Shortest Cost: %d\n", dist[dest]);
        printf("Path: ");
        printShortestPath(parent, dest);
        printf("\n");
    }
}

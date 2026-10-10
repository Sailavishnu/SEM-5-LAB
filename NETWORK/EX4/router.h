#ifndef ROUTER_H
#define ROUTER_H

#define MAX 10
#define INF 999

#define SRC_PROMPT  "\nEnter source router (A-%c): "
#define DEST_PROMPT "Enter destination router (A-%c): "

/* matrix.c */
char readRouter(const char *prompt, int n);
void printMatrix(int n, int m[MAX][MAX]);
void displayCostMatrix(int n, int graph[MAX][MAX]);
void updateLinkCost(int n, int graph[MAX][MAX]);

/* distance_vector.c */
void runDistanceVector(int n, int cost[MAX][MAX]);

/* link_state.c */
void runLinkState(int n, int graph[MAX][MAX]);
void findShortestRoute(int n, int graph[MAX][MAX]);

#endif

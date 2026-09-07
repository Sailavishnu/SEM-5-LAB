#include <stdio.h>
#define MAX 10
#define INF 999

void displayCostMatrix(int n, int graph[MAX][MAX])
{
    int i, j;
    printf("\n========================================\n");
    printf("            COST MATRIX\n");
    printf("========================================\n");
    printf("\n\t");
    for (i = 0; i < n; i++)
        printf("%c\t", 'A' + i);
    printf("\n");
    for (i = 0; i < n; i++)
    {
        printf("%c\t", 'A' + i);
        for (j = 0; j < n; j++)
        {
            if (graph[i][j] == INF)
                printf("INF\t");
            else
                printf("%d\t", graph[i][j]);
        }
        printf("\n");
    }
}

void displayFinalRoutingMatrix(int n, int dist[MAX][MAX], int next[MAX][MAX])
{
    int i, j;
    printf("\n========================================\n");
    printf("      FINAL UPDATED ROUTING TABLE\n");
    printf("          (Distance Vector)\n");
    printf("========================================\n");

    printf("\n--- Final Shortest Path Cost Matrix ---\n");
    printf("\n\t");
    for (i = 0; i < n; i++)
        printf("%c\t", 'A' + i);
    printf("\n");
    for (i = 0; i < n; i++)
    {
        printf("%c\t", 'A' + i);
        for (j = 0; j < n; j++)
        {
            if (dist[i][j] == INF)
                printf("INF\t");
            else
                printf("%d\t", dist[i][j]);
        }
        printf("\n");
    }
}

void runDistanceVector(int n, int cost[MAX][MAX])
{
    int dist[MAX][MAX];
    int next[MAX][MAX];
    int i, j, k;
    int updated;
    int round = 0;
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            dist[i][j] = cost[i][j];
            if (cost[i][j] != INF && i != j)
                next[i][j] = j;
            else
                next[i][j] = -1;
        }
    }
    do
    {
        updated = 0;
        round++;
        for (i = 0; i < n; i++)
        {
            for (j = 0; j < n; j++)
            {
                for (k = 0; k < n; k++)
                {
                    if (dist[i][k] != INF && dist[k][j] != INF)
                    {
                        if (dist[i][k] + dist[k][j] < dist[i][j])
                        {
                            dist[i][j] = dist[i][k] + dist[k][j];
                            next[i][j] = next[i][k];
                            updated = 1;
                        }
                    }
                }
            }
        }
    } while (updated);
    printf("\n========================================\n");
    printf("        DISTANCE VECTOR ROUTING\n");
    printf("========================================\n");
    printf("\nIterations = %d\n", round);
    for (i = 0; i < n; i++)
    {
        printf("\nRouting Table - %c\n", 'A' + i);
        printf("----------------------------------------\n");
        printf("Destination\tNext Hop\tCost\n");
        for (j = 0; j < n; j++)
        {
            printf("%c\t\t", 'A' + j);
            if (i == j)
            {
                printf("-\t\t0\n");
            }
            else if (dist[i][j] == INF)
            {
                printf("-\t\tINF\n");
            }
            else
            {
                printf("%c\t\t%d\n", 'A' + next[i][j], dist[i][j]);
            }
        }
    }
    displayFinalRoutingMatrix(n, dist, next);
}

int getMinimumNode(int dist[MAX], int visited[MAX], int n)
{
    int min = INF;
    int pos = -1;
    int i;
    for (i = 0; i < n; i++)
    {
        if (!visited[i] && dist[i] < min)
        {
            min = dist[i];
            pos = i;
        }
    }
    return pos;
}

void printShortestPath(int parent[MAX], int node)
{
    if (parent[node] == -1)
    {
        printf("%c", 'A' + node);
        return;
    }
    printShortestPath(parent, parent[node]);
    printf(" -> %c", 'A' + node);
}

void runLinkState(int n, int graph[MAX][MAX])
{
    int dist[MAX];
    int visited[MAX];
    int parent[MAX];
    int source;
    int i, count, current;
    printf("\nEnter source router (A-%c): ", 'A' + n - 1);
    char srcChar;
    scanf(" %c", &srcChar);
    source = srcChar - 'A';
    if (source < 0 || source >= n)
    {
        printf("\nInvalid source router!\n");
        return;
    }
    for (i = 0; i < n; i++)
    {
        dist[i] = INF;
        visited[i] = 0;
        parent[i] = -1;
    }
    dist[source] = 0;
    for (count = 0; count < n - 1; count++)
    {
        current = getMinimumNode(dist, visited, n);
        if (current == -1)
            break;
        visited[current] = 1;
        for (i = 0; i < n; i++)
        {
            if (!visited[i] && graph[current][i] != INF)
            {
                int newCost = dist[current] + graph[current][i];
                if (newCost < dist[i])
                {
                    dist[i] = newCost;
                    parent[i] = current;
                }
            }
        }
    }
    printf("\n========================================\n");
    printf("          LINK STATE ROUTING\n");
    printf("========================================\n");
    printf("\nSource Router: %c\n", 'A' + source);
    printf("\nDestination\tCost\tShortest Path\n");
    printf("----------------------------------------\n");
    for (i = 0; i < n; i++)
    {
        printf("%c\t\t", 'A' + i);
        if (dist[i] == INF)
        {
            printf("INF\tNo Path\n");
        }
        else
        {
            printf("%d\t", dist[i]);
            printShortestPath(parent, i);
            printf("\n");
        }
    }
}

void updateLinkCost(int n, int graph[MAX][MAX])
{
    char r1Char, r2Char;
    int r1, r2, newCost;
    printf("\nEnter source router (A-%c): ", 'A' + n - 1);
    scanf(" %c", &r1Char);
    printf("Enter destination router (A-%c): ", 'A' + n - 1);
    scanf(" %c", &r2Char);
    r1 = r1Char - 'A';
    r2 = r2Char - 'A';
    if (r1 < 0 || r1 >= n || r2 < 0 || r2 >= n)
    {
        printf("\nInvalid router letters!\n");
        return;
    }
    if (r1 == r2)
    {
        printf("\nCost from a router to itself is always 0.\n");
        return;
    }

    if (graph[r1][r2] == INF)
        printf("\nPrevious cost between %c and %c: INF\n", r1Char, r2Char);
    else
        printf("\nPrevious cost between %c and %c: %d\n", r1Char, r2Char, graph[r1][r2]);

    printf("Enter new cost (Enter %d for INF/no connection): ", INF);
    scanf("%d", &newCost);
    graph[r1][r2] = newCost;
    graph[r2][r1] = newCost;
    printf("\nCost matrix updated successfully between %c and %c!\n", r1Char, r2Char);
}

void findShortestRoute(int n, int graph[MAX][MAX])
{
    char srcChar, destChar;
    int source, dest;
    printf("\nEnter source router (A-%c): ", 'A' + n - 1);
    scanf(" %c", &srcChar);
    printf("Enter destination router (A-%c): ", 'A' + n - 1);
    scanf(" %c", &destChar);
    source = srcChar - 'A';
    dest = destChar - 'A';
    if (source < 0 || source >= n || dest < 0 || dest >= n)
    {
        printf("\nInvalid router letters!\n");
        return;
    }
    int dist[MAX], visited[MAX], parent[MAX];
    int i, count, current;
    for (i = 0; i < n; i++)
    {
        dist[i] = INF;
        visited[i] = 0;
        parent[i] = -1;
    }
    dist[source] = 0;
    for (count = 0; count < n - 1; count++)
    {
        current = getMinimumNode(dist, visited, n);
        if (current == -1)
            break;
        visited[current] = 1;
        for (i = 0; i < n; i++)
        {
            if (!visited[i] && graph[current][i] != INF)
            {
                int newCost = dist[current] + graph[current][i];
                if (newCost < dist[i])
                {
                    dist[i] = newCost;
                    parent[i] = current;
                }
            }
        }
    }
    printf("\n========================================\n");
    printf("     SHORTEST PATH: %c -> %c\n", 'A' + source, 'A' + dest);
    printf("========================================\n");
    if (dist[dest] == INF)
    {
        printf("No path exists between %c and %c.\n", 'A' + source, 'A' + dest);
    }
    else
    {
        printf("Shortest Cost: %d\n", dist[dest]);
        printf("Path: ");
        printShortestPath(parent, dest);
        printf("\n");
    }
}

int main()
{
    int n = 0;
    int graph[MAX][MAX];
    int i, j;
    int menuChoice;
    int initialized = 0;
    do
    {
        printf("\n========================================\n");
        printf("      ROUTING ALGORITHM SIMULATOR\n");
        printf("========================================\n");
        printf("1. Enter/Change Full Network Cost Matrix\n");
        printf("2. Display Cost Matrix\n");
        printf("3. Distance Vector Routing\n");
        printf("4. Link State Routing\n");
        printf("6. Change Cost of Specific Router Pair\n");
        printf("7. Find Shortest Distance/Path for Specific Pair\n");
        printf("9. Exit\n");
        printf("\nEnter your choice: ");
        scanf("%d", &menuChoice);
        switch (menuChoice)
        {
        case 1:
        case 1:
            printf("\nEnter number of routers (maximum %d): ", MAX);
            scanf("%d", &n);

            if (n <= 0 || n > MAX)
            {
                printf("\nInvalid number of routers!\n");
                n = 0;
                break;
            }

            printf("\nEnter the cost matrix. Enter %d for no connection.\n\n", INF);

            for (i = 0; i < n; i++)
            {
                for (j = 0; j < n; j++)
                {
                    printf("Cost %c -> %c: ", 'A' + i, 'A' + j);
                    scanf("%d", &graph[i][j]);

                    if (i == j)
                        graph[i][j] = 0;
                }
            }

            initialized = 1;
            break;
        case 2:
            if (!initialized)
                printf("\nPlease enter the cost matrix first (Option 1)!\n");
            else
                displayCostMatrix(n, graph);
            break;
        case 3:
            if (!initialized)
                printf("\nPlease enter the cost matrix first (Option 1)!\n");
            else
                runDistanceVector(n, graph);
            break;
        case 4:
            if (!initialized)
                printf("\nPlease enter the cost matrix first (Option 1)!\n");
            else
                runLinkState(n, graph);
            break;
        case 6:
            if (!initialized)
                printf("\nPlease enter the cost matrix first (Option 1)!\n");
            else
                updateLinkCost(n, graph);
            break;
        case 7:
            if (!initialized)
                printf("\nPlease enter the cost matrix first (Option 1)!\n");
            else
                findShortestRoute(n, graph);
            break;
        case 9:
            printf("\nProgram terminated.\n");
            break;
        default:
            printf("\nInvalid choice! Please try again.\n");
        }
    } while (menuChoice != 9);
    return 0;
}

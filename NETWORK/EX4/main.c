#include <stdio.h>
#include <string.h>

#define MAX_NODES 10
#define INF_COST 999

int nodeCount;
char labelArray[MAX_NODES][20];
int matrixTopology[MAX_NODES][MAX_NODES];

int distVectorMatrix[MAX_NODES][MAX_NODES];
int distVectorHops[MAX_NODES][MAX_NODES];

int linkStateMatrix[MAX_NODES][MAX_NODES];
int linkStateTrackers[MAX_NODES][MAX_NODES];

void loadNetworkSchema()
{
    printf("Enter number of nodes (Max %d): ", MAX_NODES);
    if (scanf("%d", &nodeCount) != 1 || nodeCount <= 0 || nodeCount > MAX_NODES)
    {
        printf("Invalid node count! Defaulting to 5.\n");
        nodeCount = 5;
    }

    printf("Enter node names (space-separated): ");
    for (int i = 0; i < nodeCount; i++)
    {
        scanf("%19s", labelArray[i]);
    }

    printf("Enter the Adjacency Matrix (%d x %d):\n", nodeCount, nodeCount);
    printf("(Use %d for INF/No Edge)\n", INF_COST);

    for (int i = 0; i < nodeCount; i++)
    {
        for (int j = 0; j < nodeCount; j++)
        {
            scanf("%d", &matrixTopology[i][j]);
        }
    }
}

int findNodeIndex(const char *name)
{
    for (int i = 0; i < nodeCount; i++)
    {
        if (strcmp(labelArray[i], name) == 0)
        {
            return i;
        }
    }
    return -1;
}

void modifyLinkCost()
{
    char srcName[20], destName[20];
    int newCost;

    printf("\nEnter Source Node Name: ");
    scanf("%19s", srcName);
    printf("Enter Destination Node Name: ");
    scanf("%19s", destName);

    int srcIdx = findNodeIndex(srcName);
    int destIdx = findNodeIndex(destName);

    if (srcIdx == -1 || destIdx == -1)
    {
        printf("Error: One or both node names are invalid!\n");
        return;
    }

    if (srcIdx == destIdx)
    {
        printf("Error: Cannot modify self-loop cost!\n");
        return;
    }

    printf("Enter new connection cost (Use %d for breaking the link): ", INF_COST);
    if (scanf("%d", &newCost) != 1)
    {
        printf("Invalid cost input!\n");
        return;
    }

    matrixTopology[srcIdx][destIdx] = newCost;
    // Assuming an undirected graph topology. Remove the line below if the graph is strictly directed.
    matrixTopology[destIdx][srcIdx] = newCost;

    printf("Success: Connection between %s and %s updated to %d.\n", srcName, destName, newCost);
}

void traceVectorRoute(int start, int end)
{
    if (distVectorMatrix[start][end] >= INF_COST)
    {
        printf("-");
        return;
    }

    int currentPos = start;
    printf("%s", labelArray[currentPos]);

    int visited[MAX_NODES] = {0};
    while (currentPos != end)
    {
        visited[currentPos] = 1;
        currentPos = distVectorHops[currentPos][end];

        if (currentPos == -1 || visited[currentPos])
        {
            printf(" -> Loop/Broken Path");
            return;
        }
        printf(" -> %s", labelArray[currentPos]);
    }
}

void triggerDistanceVectorCalculation()
{
    for (int i = 0; i < nodeCount; i++)
    {
        for (int j = 0; j < nodeCount; j++)
        {
            distVectorMatrix[i][j] = matrixTopology[i][j];

            if (i == j)
                distVectorHops[i][j] = i;
            else if (matrixTopology[i][j] < INF_COST)
                distVectorHops[i][j] = j;
            else
                distVectorHops[i][j] = -1;
        }
    }

    int flagModified = 1;
    while (flagModified)
    {
        flagModified = 0;

        for (int i = 0; i < nodeCount; i++)
        {
            for (int j = 0; j < nodeCount; j++)
            {
                if (i == j || matrixTopology[i][j] >= INF_COST)
                    continue;

                for (int k = 0; k < nodeCount; k++)
                {
                    if (i == k)
                        continue;

                    if (distVectorMatrix[j][k] < INF_COST &&
                        matrixTopology[i][j] + distVectorMatrix[j][k] < distVectorMatrix[i][k])
                    {
                        distVectorMatrix[i][k] = matrixTopology[i][j] + distVectorMatrix[j][k];
                        distVectorHops[i][k] = j;
                        flagModified = 1;
                    }
                }
            }
        }
    }

    printf("\n========================================================================\n");
    printf("                       DISTANCE VECTOR ROUTING\n");
    printf("========================================================================\n");

    for (int i = 0; i < nodeCount; i++)
    {
        printf("\nRouting Table of Router %s\n", labelArray[i]);
        printf("%-15s %-10s %-15s %-30s\n", "Destination", "Cost", "NextHop", "Full Path Route");
        printf("------------------------------------------------------------------------\n");

        for (int j = 0; j < nodeCount; j++)
        {
            if (i == j)
                continue;

            printf("%-15s", labelArray[j]);

            if (distVectorMatrix[i][j] >= INF_COST)
            {
                printf("%-10s %-15s %-30s\n", "INF", "-", "No Path Available");
            }
            else
            {
                printf("%-10d %-15s ", distVectorMatrix[i][j], labelArray[distVectorHops[i][j]]);
                traceVectorRoute(i, j);
                printf("\n");
            }
        }
    }
}

void traceLinkStateRoute(int start, int end)
{
    if (linkStateMatrix[start][end] >= INF_COST)
    {
        printf("No Path Available");
        return;
    }

    int stepSequence[MAX_NODES];
    int hopIndex = 0;
    int currentPos = end;

    while (currentPos != start && currentPos != -1)
    {
        stepSequence[hopIndex++] = currentPos;
        currentPos = linkStateTrackers[start][currentPos];
    }
    stepSequence[hopIndex++] = start;

    for (int i = hopIndex - 1; i >= 0; i--)
    {
        printf("%s", labelArray[stepSequence[i]]);
        if (i > 0)
        {
            printf(" -> ");
        }
    }
}

void triggerLinkStateCalculation()
{
    for (int centerNode = 0; centerNode < nodeCount; centerNode++)
    {
        int processedFlags[MAX_NODES] = {0};

        for (int i = 0; i < nodeCount; i++)
        {
            linkStateMatrix[centerNode][i] = matrixTopology[centerNode][i];

            if (i != centerNode && matrixTopology[centerNode][i] < INF_COST)
                linkStateTrackers[centerNode][i] = centerNode;
            else
                linkStateTrackers[centerNode][i] = -1;
        }

        linkStateMatrix[centerNode][centerNode] = 0;
        processedFlags[centerNode] = 1;
        linkStateTrackers[centerNode][centerNode] = centerNode;

        for (int step = 1; step < nodeCount; step++)
        {
            int selectedIndex = -1;
            int marginDistance = INF_COST;

            for (int i = 0; i < nodeCount; i++)
            {
                if (!processedFlags[i] && linkStateMatrix[centerNode][i] < marginDistance)
                {
                    marginDistance = linkStateMatrix[centerNode][i];
                    selectedIndex = i;
                }
            }

            if (selectedIndex == -1)
                break;

            processedFlags[selectedIndex] = 1;

            for (int scanIndex = 0; scanIndex < nodeCount; scanIndex++)
            {
                if (!processedFlags[scanIndex] && matrixTopology[selectedIndex][scanIndex] < INF_COST &&
                    linkStateMatrix[centerNode][selectedIndex] + matrixTopology[selectedIndex][scanIndex] < linkStateMatrix[centerNode][scanIndex])
                {
                    linkStateMatrix[centerNode][scanIndex] = linkStateMatrix[centerNode][selectedIndex] + matrixTopology[selectedIndex][scanIndex];
                    linkStateTrackers[centerNode][scanIndex] = selectedIndex;
                }
            }
        }
    }

    printf("\n========================================================================\n");
    printf("                          LINK STATE ROUTING\n");
    printf("========================================================================\n");

    for (int i = 0; i < nodeCount; i++)
    {
        printf("\nRouting Table of Router %s\n", labelArray[i]);
        printf("%-15s %-10s %-15s %-30s\n", "Destination", "Cost", "NextHop", "Full Path Route");
        printf("------------------------------------------------------------------------\n");

        for (int j = 0; j < nodeCount; j++)
        {
            if (i == j)
                continue;

            printf("%-15s", labelArray[j]);

            if (linkStateMatrix[i][j] >= INF_COST)
            {
                printf("%-10s %-15s %-30s\n", "INF", "-", "No Path Available");
            }
            else
            {
                int forwardTarget = j;
                while (forwardTarget != -1 && linkStateTrackers[i][forwardTarget] != i)
                {
                    forwardTarget = linkStateTrackers[i][forwardTarget];
                }

                printf("%-10d %-15s ", linkStateMatrix[i][j], labelArray[forwardTarget]);
                traceLinkStateRoute(i, j);
                printf("\n");
            }
        }
    }
}

int main()
{
    int SelectionId;

    loadNetworkSchema();

    do
    {
        printf("\n\n=============================================\n");
        printf("          ROUTING ALGORITHMS\n");
        printf("=============================================\n");
        printf("1. Distance Vector Routing\n");
        printf("2. Link State Routing\n");
        printf("3. Update Link Cost\n");
        printf("4. Exit\n");
        printf("=============================================\n");
        printf("Enter your choice: ");
        if (scanf("%d", &SelectionId) != 1)
        {
            printf("Invalid selection! Exiting...\n");
            break;
        }

        switch (SelectionId)
        {
            case 1:
                triggerDistanceVectorCalculation();
                break;

            case 2:
                triggerLinkStateCalculation();
                break;

            case 3:
                modifyLinkCost();
                break;

            case 4:
                printf("\nProgram terminated.\n");
                break;

            default:
                printf("\nInvalid choice! Please try again.\n");
        }

    } while (SelectionId != 4);

    return 0;
}

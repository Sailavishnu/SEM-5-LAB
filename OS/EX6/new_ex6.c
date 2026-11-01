#include <stdio.h>
#define MAX 10

void calculateNeed(int p, int r, int maxm[][MAX], int alloc[][MAX], int need[][MAX])
{
    for (int i = 0; i < p; i++)
        for (int j = 0; j < r; j++)
            need[i][j] = maxm[i][j] - alloc[i][j];
}

int checkSafety(int p, int r, int alloc[][MAX], int need[][MAX], int avail[], int seq[])
{
    int work[MAX], finish[MAX] = {0}, count = 0, found = 1;

    for (int j = 0; j < r; j++)
        work[j] = avail[j];

    while (found)
    {
        found = 0;
        for (int i = 0; i < p; i++)
        {
            int ok = !finish[i];
            for (int j = 0; j < r; j++)
                if (need[i][j] > work[j])
                    ok = 0;

            if (ok)
            {
                for (int j = 0; j < r; j++)
                    work[j] += alloc[i][j];
                seq[count++] = i;
                finish[i] = found = 1;
            }
        }
    }

    if (count < p)
    {
        printf("UNSAFE (deadlock possible)\n");
        return 0;
    }
    printf("SAFE: ");
    for (int i = 0; i < count; i++)
        printf("P%d ", seq[i]);
    printf("\n");
    return 1;
}

void requestResources(int p, int r, int pid, int req[], int alloc[][MAX], int need[][MAX], int avail[])
{
    int seq[MAX];

    for (int j = 0; j < r; j++)
        if (req[j] > need[pid][j])
        {
            printf("Denied: exceeds P%d's Need.\n", pid);
            return;
        }

    for (int j = 0; j < r; j++)
        if (req[j] > avail[j])
        {
            printf("Denied: not enough Available. P%d must wait.\n", pid);
            return;
        }

    for (int j = 0; j < r; j++)
    {
        avail[j] -= req[j];
        alloc[pid][j] += req[j];
        need[pid][j] -= req[j];
    }

    if (checkSafety(p, r, alloc, need, avail, seq))
        printf("Granted.\n");
    else
    {
        printf("Rolled back. P%d must wait.\n", pid);
        for (int j = 0; j < r; j++)
        {
            avail[j] += req[j];
            alloc[pid][j] -= req[j];
            need[pid][j] += req[j];
        }
    }
}

int main(void)
{
    int p, r, pid;
    int alloc[MAX][MAX], maxm[MAX][MAX], need[MAX][MAX];
    int avail[MAX], req[MAX], seq[MAX];
    char choice;

    printf("Enter number of processes and resource types: ");
    scanf("%d %d", &p, &r);

    printf("Enter Allocation Matrix:\n");
    for (int i = 0; i < p; i++)
        for (int j = 0; j < r; j++)
            scanf("%d", &alloc[i][j]);

    printf("Enter Max Matrix:\n");
    for (int i = 0; i < p; i++)
        for (int j = 0; j < r; j++)
            scanf("%d", &maxm[i][j]);

    printf("Enter Available Resources:\n");
    for (int j = 0; j < r; j++)
        scanf("%d", &avail[j]);

    calculateNeed(p, r, maxm, alloc, need);
    checkSafety(p, r, alloc, need, avail, seq);

    do
    {
        printf("\nProcess no. and request for each resource: ");
        scanf("%d", &pid);
        for (int j = 0; j < r; j++)
            scanf("%d", &req[j]);

        requestResources(p, r, pid, req, alloc, need, avail);

        printf("Another request? (y/n): ");
        scanf(" %c", &choice);
    } while (choice == 'y' || choice == 'Y');

    return 0;
}
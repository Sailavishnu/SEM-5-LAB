#include <stdio.h>
#define MAX 10

int checkSafety(int p, int r, int alloc[][MAX], int need[][MAX], int avail[], int seq[], int *count)
{
    int work[MAX], finish[MAX] = {0}, found = 1;

    *count = 0;
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
                seq[(*count)++] = i;
                finish[i] = found = 1;
            }
        }
    }

    return *count == p;
}

void printSequence(int seq[] , int count)
{
    for (int i = 0; i < count; i++)
        printf("P%d ", seq[i]);
    printf("\n");
}

void requestResources(int p, int r,int alloc[][MAX], int need[][MAX], int avail[],int pid, int req[])
{
    int seq[MAX], count;

    if (pid < 0 || pid >= p)
    {
        printf("Invalid process number. Use 0 to %d.\n", p - 1);
        return;
    }

    for (int j = 0; j < r; j++)
        if (req[j] > need[pid][j])
        {
            printf("Denied: request exceeds P%d's Need.\n", pid);
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

    if (checkSafety(p, r, alloc, need, avail, seq, &count))
    {
        printf("Request GRANTED. New safe sequence: ");
        printSequence(seq, count);
    }
    else
    {
        for (int j = 0; j < r; j++)
        {
            avail[j] += req[j];
            alloc[pid][j] -= req[j];
            need[pid][j] += req[j];
        }
        printf("Request NOT granted: resulting state is unsafe. P%d must wait.\n", pid);
    }
}

int main(void)
{
    int p, r, pid, count;
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

    for (int i = 0; i < p; i++)
        for (int j = 0; j < r; j++)
            need[i][j] = maxm[i][j] - alloc[i][j];

    if (!checkSafety(p, r, alloc, need, avail, seq, &count))
    {
        printf("\nInitial state is UNSAFE (deadlock possible). No requests can be processed.\n");
        return 0;
    }

    printf("\nInitial state is SAFE.\nSafe sequence: ");
    printSequence(seq,count);

    do
    {
        printf("\nEnter process number (0 to %d): ", p - 1);
        scanf("%d", &pid);

        printf("Enter request for each of %d resources: ", r);
        for (int j = 0; j < r; j++)
            scanf("%d", &req[j]);

        requestResources(p, r,alloc, need, avail,pid ,req);

        printf("\nAnother request? (y/n): ");
        scanf(" %c", &choice);
    } while (choice == 'y' || choice == 'Y');

    return 0;
}
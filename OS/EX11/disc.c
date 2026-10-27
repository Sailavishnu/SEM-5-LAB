#include <stdio.h>
#include <stdlib.h>

#define MAX 100

void display(int seq[], int n, int head)
{
    int i, total = 0;
    printf("Head movement: %d", head);
    for (i = 0; i < n; i++)
    {
        total += abs(head - seq[i]);
        head = seq[i];
        printf(" -> %d", head);
    }
    printf("\nTotal head movement = %d\n", total);
}

void sort(int a[], int n)
{
    int i, j, temp;
    for (i = 0; i < n - 1; i++)
    {
        for (j = 0; j < n - i - 1; j++)
        {
            if (a[j] > a[j + 1])
            {
                temp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = temp;
            }
        }
    }
}

void FCFS(int req[], int n, int head)
{
    printf("\nFCFS\n");
    display(req, n, head);
}

void SSTF(int req[], int n, int head)
{
    int done[MAX] = {0};
    int seq[MAX], i, j, pos, min, count = 0;
    int start = head;

    for (i = 0; i < n; i++)
    {
        min = 100000;
        pos = -1;
        for (j = 0; j < n; j++)
        {
            if (!done[j] && abs(head - req[j]) < min)
            {
                min = abs(head - req[j]);
                pos = j;
            }
        }
        done[pos] = 1;
        seq[count++] = req[pos];
        head = req[pos];
    }
    printf("\nSSTF\n");
    display(seq, n, start);
}

void SCAN(int req[], int n, int head, int size, int dir)
{
    int a[MAX], seq[MAX + 1];
    int i, count = 0;
    for (i = 0; i < n; i++)
        a[i] = req[i];
    sort(a, n);

    if (dir == 1)
    {
        for (i = 0; i < n; i++)
            if (a[i] >= head)
                seq[count++] = a[i];
        seq[count++] = size - 1;
        for (i = n - 1; i >= 0; i--)
            if (a[i] < head)
                seq[count++] = a[i];
    }
    else
    {
        for (i = n - 1; i >= 0; i--)
            if (a[i] <= head)
                seq[count++] = a[i];
        seq[count++] = 0;
        for (i = 0; i < n; i++)
            if (a[i] > head)
                seq[count++] = a[i];
    }
    printf("\nSCAN\n");
    display(seq, count, head);
}

void CSCAN(int req[], int n, int head, int size, int dir)
{
    int a[MAX], seq[MAX + 2];
    int i, count = 0;
    for (i = 0; i < n; i++)
        a[i] = req[i];
    sort(a, n);

    if (dir == 1)
    {
        for (i = 0; i < n; i++)
            if (a[i] >= head)
                seq[count++] = a[i];
        seq[count++] = size - 1;
        seq[count++] = 0;
        for (i = 0; i < n; i++)
            if (a[i] < head)
                seq[count++] = a[i];
    }
    else
    {
        for (i = n - 1; i >= 0; i--)
            if (a[i] <= head)
                seq[count++] = a[i];
        seq[count++] = 0;
        seq[count++] = size - 1;
        for (i = n - 1; i >= 0; i--)
            if (a[i] > head)
                seq[count++] = a[i];
    }
    printf("\nCSCAN\n");
    display(seq, count, head);
}

void LOOK(int req[], int n, int head, int dir)
{
    int a[MAX], seq[MAX];
    int i, count = 0;
    for (i = 0; i < n; i++)
        a[i] = req[i];
    sort(a, n);

    if (dir == 1)
    {
        for (i = 0; i < n; i++)
            if (a[i] >= head)
                seq[count++] = a[i];
        for (i = n - 1; i >= 0; i--)
            if (a[i] < head)
                seq[count++] = a[i];
    }
    else
    {
        for (i = n - 1; i >= 0; i--)
            if (a[i] <= head)
                seq[count++] = a[i];
        for (i = 0; i < n; i++)
            if (a[i] > head)
                seq[count++] = a[i];
    }
    printf("\nLOOK\n");
    display(seq, count, head);
}

void CLOOK(int req[], int n, int head, int dir)
{
    int a[MAX], seq[MAX];
    int i, count = 0;
    for (i = 0; i < n; i++)
        a[i] = req[i];
    sort(a, n);

    if (dir == 1)
    {
        for (i = 0; i < n; i++)
            if (a[i] >= head)
                seq[count++] = a[i];
        for (i = 0; i < n; i++)
            if (a[i] < head)
                seq[count++] = a[i];
    }
    else
    {
        for (i = n - 1; i >= 0; i--)
            if (a[i] <= head)
                seq[count++] = a[i];
        for (i = n - 1; i >= 0; i--)
            if (a[i] > head)
                seq[count++] = a[i];
    }
    printf("\nCLOOK\n");
    display(seq, count, head);
}

int main()
{
    int req[MAX], n, head, prev_head, size, dir;
    int choice, i;

    printf("Enter number of requests: ");
    scanf("%d", &n);
    if (n < 1 || n > MAX)
    {
        printf("Invalid number of requests\n");
        return 1;
    }

    printf("Enter request queue: ");
    for (i = 0; i < n; i++)
        scanf("%d", &req[i]);

    printf("Enter initial head position: ");
    scanf("%d", &head);

    printf("Enter previous head position: ");
    scanf("%d", &prev_head);

    printf("Enter total number of cylinders: ");
    scanf("%d", &size);

    if (size < 1 || head < 0 || head >= size || prev_head < 0 || prev_head >= size)
    {
        printf("Invalid disk size or head positions\n");
        return 1;
    }

    for (i = 0; i < n; i++)
    {
        if (req[i] < 0 || req[i] >= size)
        {
            printf("Invalid cylinder request\n");
            return 1;
        }
    }

    // Direction calculation logic
    if (head >= prev_head)
    {
        dir = 1; // Moving High/Right
        printf("Calculated Direction: Right (Head moved from %d to %d)\n", prev_head, head);
    }
    else
    {
        dir = 0; // Moving Low/Left
        printf("Calculated Direction: Left (Head moved from %d to %d)\n", prev_head, head);
    }

    do
    {
        printf("\n--- DISK SCHEDULING MENU ---\n");
        printf("1. FCFS\n");
        printf("2. SSTF\n");
        printf("3. SCAN\n");
        printf("4. CSCAN\n");
        printf("5. LOOK\n");
        printf("6. CLOOK\n");
        printf("7. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            FCFS(req, n, head);
            break;
        case 2:
            SSTF(req, n, head);
            break;
        case 3:
            SCAN(req, n, head, size, dir);
            break;
        case 4:
            CSCAN(req, n, head, size, dir);
            break;
        case 5:
            LOOK(req, n, head, dir);
            break;
        case 6:
            CLOOK(req, n, head, dir);
            break;
        case 7:
            printf("Exiting program...\n");
            break;
        default:
            printf("Invalid choice!\n");
        }
    } while (choice != 7);

    return 0;
}

#include <stdio.h>
#define MAX 20

typedef struct
{
    int id, size;
    int allocatedTo;    /* process id in this hole, -1 = free */
    int fragmentation;  /* hole size - process size */
} MemoryHole;

typedef struct
{
    int id, size;
    int allocatedHole;  /* hole id for this process, -1 = none */
} Process;

void reset(int n, int m, MemoryHole holes[], Process procs[])
{
    for (int j = 0; j < n; j++)
    {
        holes[j].allocatedTo = -1;
        holes[j].fragmentation = 0;
    }
    for (int i = 0; i < m; i++)
        procs[i].allocatedHole = -1;
}

/* flag = 1 -> Best Fit order, flag = 0 -> Worst Fit order */
void sortHoles(int n, MemoryHole holes[], int idx[], int flag)
{
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - i - 1; j++)
        {
            int a = holes[idx[j]].size, b = holes[idx[j + 1]].size;
            if ((flag && a > b) || (!flag && a < b))
            {
                int t = idx[j];
                idx[j] = idx[j + 1];
                idx[j + 1] = t;
            }
        }
}

void firstFit(int n, int m, MemoryHole holes[], Process procs[])
{
    reset(n, m, holes, procs);
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            if (holes[j].allocatedTo == -1 && holes[j].size >= procs[i].size)
            {
                holes[j].allocatedTo = procs[i].id;
                holes[j].fragmentation = holes[j].size - procs[i].size;
                procs[i].allocatedHole = holes[j].id;
                break;
            }
}

void fitBySize(int n, int m, MemoryHole holes[], Process procs[], int flag)
{
    int idx[MAX];

    reset(n, m, holes, procs);
    for (int j = 0; j < n; j++)
        idx[j] = j;
    sortHoles(n, holes, idx, flag);

    for (int i = 0; i < m; i++)
        for (int k = 0; k < n; k++)
        {
            int j = idx[k];
            if (holes[j].allocatedTo == -1 && holes[j].size >= procs[i].size)
            {
                holes[j].allocatedTo = procs[i].id;
                holes[j].fragmentation = holes[j].size - procs[i].size;
                procs[i].allocatedHole = holes[j].id;
                break;
            }
        }
}

void printResult(int n, int m, MemoryHole holes[], Process procs[])
{
    int internal = 0, external = 0;

    for (int i = 0; i < m; i++)
        if (procs[i].allocatedHole == -1)
            printf("P%d -> Not allocated\n", procs[i].id);
        else
            printf("P%d -> H%d\n", procs[i].id, procs[i].allocatedHole);

    printf("Unused holes: ");
    for (int j = 0; j < n; j++)
        if (holes[j].allocatedTo == -1)
        {
            printf("H%d ", holes[j].id);
            external += holes[j].size;
        }
        else
            internal += holes[j].fragmentation;
    printf("\nInternal = %d | External = %d\n", internal, external);
}

int main(void)
{
    int n, m, choice;
    MemoryHole holes[MAX];
    Process procs[MAX];

    printf("Enter number of holes: ");
    scanf("%d", &n);
    for (int j = 0; j < n; j++)
    {
        holes[j].id = j + 1;
        printf("Size of H%d: ", j + 1);
        scanf("%d", &holes[j].size);
    }

    printf("Enter number of processes: ");
    scanf("%d", &m);
    for (int i = 0; i < m; i++)
    {
        procs[i].id = i + 1;
        printf("Size of P%d: ", i + 1);
        scanf("%d", &procs[i].size);
    }

    do
    {
        printf("\n1.First Fit  2.Best Fit  3.Worst Fit  4.Exit\nChoice: ");
        scanf("%d", &choice);

        if (choice == 1)
            firstFit(n, m, holes, procs);
        else if (choice == 2)
            fitBySize(n, m, holes, procs, 1);
        else if (choice == 3)
            fitBySize(n, m, holes, procs, 0);
        else
            continue;

        printResult(n, m, holes, procs);
    } while (choice != 4);

    return 0;
}
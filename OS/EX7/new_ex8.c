
#include <stdio.h>
#define MAX 20

typedef struct
{
    int id, size, allocatedTo, fragmentation;
} MemoryHole;

typedef struct
{
    int id, size, allocatedHole;
} Process;

MemoryHole h[MAX];
Process p[MAX];
int n, m;

void reset()
{
    for (int i = 0; i < n; i++)
    {
        h[i].allocatedTo = -1;
        h[i].fragmentation = 0;
    }
    for (int i = 0; i < m; i++)
        p[i].allocatedHole = -1;
}

void firstFit()
{
    reset();

    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            if (h[j].allocatedTo == -1 && h[j].size >= p[i].size)
            {
                h[j].allocatedTo = p[i].id;
                h[j].fragmentation = h[j].size - p[i].size;
                p[i].allocatedHole = h[j].id;
                break;
            }
}

void fit(int flag)
{
    reset();
    int idx[MAX], i, j, t;

    for (i = 0; i < n; i++)
        idx[i] = i;

    for (i = 0; i < n - 1; i++)
        for (j = 0; j < n - i - 1; j++)
            if ((flag && h[idx[j]].size > h[idx[j + 1]].size) ||
                (!flag && h[idx[j]].size < h[idx[j + 1]].size))
            {
                t = idx[j];
                idx[j] = idx[j + 1];
                idx[j + 1] = t;
            }

    for (i = 0; i < m; i++)
        for (j = 0; j < n; j++)
        {
            int k = idx[j];

            if (h[k].allocatedTo == -1 &&
                h[k].size >= p[i].size)
            {
                h[k].allocatedTo = p[i].id;
                h[k].fragmentation = h[k].size - p[i].size;
                p[i].allocatedHole = h[k].id;
                break;
            }
        }
}

void showResult()
{
    int in = 0, ex = 0;

    for (int i = 0; i < n; i++)
        if (h[i].allocatedTo == -1)
            ex += h[i].size;
        else
            in += h[i].fragmentation;

    printf("Internal Fragmentation = %d\n", in);
    printf("External Fragmentation = %d\n", ex);
}

int main()
{
    int ch;

    printf("Enter number of memory holes: ");
    scanf("%d", &n);

    for (int i = 0; i < n; i++)
    {
        h[i].id = i + 1;
        printf("Size of H%d: ", i + 1);
        scanf("%d", &h[i].size);
    }

    printf("Enter number of processes: ");
    scanf("%d", &m);

    for (int i = 0; i < m; i++)
    {
        p[i].id = i + 1;
        printf("Size of Process %d: ", i + 1);
        scanf("%d", &p[i].size);
    }

    do
    {
        printf("\n1.First Fit\n2.Best Fit\n3.Worst Fit\n4.Exit\nChoice: ");
        scanf("%d", &ch);

        if (ch == 1)
        {
            firstFit();
            showResult();
        }
        else if (ch == 2)
        {
            fit(1);
            showResult();
        }
        else if (ch == 3)
        {
            fit(0);
            showResult();
        }
    } while (ch != 4);

    return 0;
}

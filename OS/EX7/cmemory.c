#include <stdio.h>
#define MAX 20

typedef struct
{
    int id;
    int size;
    int allocatedTo;   // process id allocated to this hole, -1 if free
    int fragmentation; // internal fragmentation left in this hole after allocation
} MemoryHole;

typedef struct
{
    int id;
    int size;
    int allocatedHole; // hole id this process is allocated to, -1 if not allocated
} Process;

MemoryHole holes[MAX];
Process procs[MAX];
int n, m; // n = number of holes, m = number of processes

void reset()
{
    for (int i = 0; i < n; i++)
    {
        holes[i].allocatedTo = -1;
        holes[i].fragmentation = 0;
    }
    for (int i = 0; i < m; i++)
    {
        procs[i].allocatedHole = -1;
    }
}

/* flag = 1 -> ascending order (Best Fit), flag = 0 -> descending order (Worst Fit) */
void sortHoles(int idx[], int flag)
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

void firstFit()
{
    reset();
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

void fitBySize(int flag)
{ /* flag=1 Best Fit, flag=0 Worst Fit */
    reset();
    int idx[MAX];
    for (int i = 0; i < n; i++)
        idx[i] = i;
    sortHoles(idx, flag);
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

void showResult(const char *name)
{
    int intFrag = 0, extFrag = 0;
    printf("\n-- %s --\n", name);
    for (int i = 0; i < m; i++)
    {
        if (procs[i].allocatedHole == -1)
            printf("Process %d -> Not Allocated\n", procs[i].id);
        else
        {
            /* find the hole struct that matches allocatedHole id */
            for (int j = 0; j < n; j++)
            {
                if (holes[j].id == procs[i].allocatedHole)
                {
                    printf("Process %d -> H%d (frag=%d)\n",
                           procs[i].id, holes[j].id, holes[j].fragmentation);
                    intFrag += holes[j].fragmentation;
                    break;
                }
            }
        }
    }

    printf("Unused: ");
    int any = 0;
    for (int j = 0; j < n; j++)
        if (holes[j].allocatedTo == -1)
        {
            printf("H%d ", holes[j].id);
            extFrag += holes[j].size;
            any = 1;
        }
    if (!any)
        printf("None");

    printf("\nInternal Frag = %d | External Frag = %d\n", intFrag, extFrag);
}

int main()
{
    printf("Enter number of memory holes: ");
    scanf("%d", &n);
    for (int i = 0; i < n; i++)
    {
        holes[i].id = i + 1;
        printf("Size of H%d: ", i + 1);
        scanf("%d", &holes[i].size);
    }

    printf("Enter number of processes: ");
    scanf("%d", &m);
    for (int i = 0; i < m; i++)
    {
        procs[i].id = i + 1;
        printf("Size of Process %d: ", i + 1);
        scanf("%d", &procs[i].size);
    }

    int choice;
    do
    {
        printf("\n1.First Fit \n2.Best Fit \n3.Worst Fit \n4.Exit\nChoice: ");
        scanf("%d", &choice);
        if (choice == 1)
        {
            firstFit();
            showResult("First Fit");
        }
        else if (choice == 2)
        {
            fitBySize(1);
            showResult("Best Fit");
        }
        else if (choice == 3)
        {
            fitBySize(0);
            showResult("Worst Fit");
        }
    } while (choice != 4);

    return 0;
}
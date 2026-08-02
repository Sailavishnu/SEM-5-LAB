#include <stdio.h>

struct Process
{
    int pid;
    int bt;
    int wt;
    int tat;
    int priority;
};

void fcfs(struct Process p[], int n)
{
    int i;
    float avgwt = 0, avgtat = 0;

    p[0].wt = 0;

    for (i = 1; i < n; i++)
        p[i].wt = p[i - 1].wt + p[i - 1].bt;

    printf("\nGantt Chart:\n|");
    for (i = 0; i < n; i++)
        printf(" P%d |", p[i].pid);

    printf("\n\nPID\tBT\tWT\tTAT\n");

    for (i = 0; i < n; i++)
    {
        p[i].tat = p[i].wt + p[i].bt;
        avgwt += p[i].wt;
        avgtat += p[i].tat;

        printf("P%d\t%d\t%d\t%d\n", p[i].pid, p[i].bt, p[i].wt, p[i].tat);
    }

    printf("\nAverage Waiting Time = %.2f", avgwt / n);
    printf("\nAverage Turnaround Time = %.2f\n", avgtat / n);
}

void sjf(struct Process p[], int n)
{
    int i, j;
    struct Process temp;
    float avgwt = 0, avgtat = 0;

    for (i = 0; i < n - 1; i++)
    {
        for (j = i + 1; j < n; j++)
        {
            if (p[i].bt > p[j].bt)
            {
                temp = p[i];
                p[i] = p[j];
                p[j] = temp;
            }
        }
    }

    p[0].wt = 0;

    for (i = 1; i < n; i++)
        p[i].wt = p[i - 1].wt + p[i - 1].bt;

    printf("\nGantt Chart:\n|");
    for (i = 0; i < n; i++)
        printf(" P%d |", p[i].pid);

    printf("\n\nPID\tBT\tWT\tTAT\n");

    for (i = 0; i < n; i++)
    {
        p[i].tat = p[i].wt + p[i].bt;
        avgwt += p[i].wt;
        avgtat += p[i].tat;

        printf("P%d\t%d\t%d\t%d\n", p[i].pid, p[i].bt, p[i].wt, p[i].tat);
    }

    printf("\nAverage Waiting Time = %.2f", avgwt / n);
    printf("\nAverage Turnaround Time = %.2f\n", avgtat / n);
}

void priorityScheduling(struct Process p[], int n)
{
    int i, j;
    struct Process temp;
    float avgwt = 0, avgtat = 0;

    for (i = 0; i < n - 1; i++)
    {
        for (j = i + 1; j < n; j++)
        {
            if (p[i].priority > p[j].priority)
            {
                temp = p[i];
                p[i] = p[j];
                p[j] = temp;
            }
        }
    }

    p[0].wt = 0;

    for (i = 1; i < n; i++)
        p[i].wt = p[i - 1].wt + p[i - 1].bt;

    printf("\nGantt Chart:\n|");
    for (i = 0; i < n; i++)
        printf(" P%d |", p[i].pid);

    printf("\n\nPID\tBT\tPriority\tWT\tTAT\n");

    for (i = 0; i < n; i++)
    {
        p[i].tat = p[i].wt + p[i].bt;
        avgwt += p[i].wt;
        avgtat += p[i].tat;

        printf("P%d\t%d\t%d\t\t%d\t%d\n",
               p[i].pid, p[i].bt, p[i].priority, p[i].wt, p[i].tat);
    }

    printf("\nAverage Waiting Time = %.2f", avgwt / n);
    printf("\nAverage Turnaround Time = %.2f\n", avgtat / n);
}

void roundRobin(struct Process p[], int n, int tq)
{
    int rem[20];
    int wt[20] = {0};
    int tat[20];
    int time = 0, i;
    float avgwt = 0, avgtat = 0;
    int done;

    for (i = 0; i < n; i++)
        rem[i] = p[i].bt;

    printf("\nGantt Chart:\n|");

    do
    {
        done = 1;

        for (i = 0; i < n; i++)
        {
            if (rem[i] > 0)
            {
                done = 0;

                if (rem[i] > tq)
                {
                    printf(" P%d |", p[i].pid);
                    time += tq;
                    rem[i] -= tq;
                }
                else
                {
                    printf(" P%d |", p[i].pid);
                    time += rem[i];
                    wt[i] = time - p[i].bt;
                    rem[i] = 0;
                }
            }
        }

    } while (!done);

    printf("\n\nPID\tBT\tWT\tTAT\n");

    for (i = 0; i < n; i++)
    {
        tat[i] = wt[i] + p[i].bt;
        avgwt += wt[i];
        avgtat += tat[i];

        printf("P%d\t%d\t%d\t%d\n", p[i].pid, p[i].bt, wt[i], tat[i]);
    }

    printf("\nAverage Waiting Time = %.2f", avgwt / n);
    printf("\nAverage Turnaround Time = %.2f\n", avgtat / n);
}

int main()
{
    struct Process p[20], temp[20];
    int n, i, ch, tq;

    printf("Enter number of processes: ");
    scanf("%d", &n);

    for (i = 0; i < n; i++)
    {
        p[i].pid = i + 1;

        printf("\nProcess %d\n", i + 1);

        printf("Burst Time: ");
        scanf("%d", &p[i].bt);

        printf("Priority: ");
        scanf("%d", &p[i].priority);
    }

    while (1)
    {
        printf("\nCPU Scheduling Algorithms");
        printf("\n1.FCFS");
        printf("\n2.SJF");
        printf("\n3.Priority Scheduling");
        printf("\n4.Round Robin");
        printf("\n5.Exit");

        printf("\nEnter your choice: ");
        scanf("%d", &ch);

        for (i = 0; i < n; i++)
            temp[i] = p[i];

        switch (ch)
        {
        case 1:
            fcfs(temp, n);
            break;

        case 2:
            sjf(temp, n);
            break;

        case 3:
            priorityScheduling(temp, n);
            break;

        case 4:
            printf("Enter Time Quantum: ");
            scanf("%d", &tq);
            roundRobin(temp, n, tq);
            break;

        case 5:
            return 0;

        default:
            printf("Invalid Choice");
        }
    }
}
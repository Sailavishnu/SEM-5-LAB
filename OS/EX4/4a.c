#include <stdio.h>

struct Process
{
    int pid;
    int bt;
    int wt;
    int tat;
    int priority;
};

/* ---------------------------------------------------------
   Function to draw a properly spaced Gantt Chart
   --------------------------------------------------------- */
void drawGanttChart(int ganttPid[], int ganttStart[],
                    int ganttEnd[], int count)
{
    int i;

    printf("\nGantt Chart:\n\n");

    /* Top border */
    for (i = 0; i < count; i++)
        printf("--------");
    printf("\n");

    /* Process names */
    for (i = 0; i < count; i++)
        printf("|  P%-3d ", ganttPid[i]);
    printf("|\n");

    /* Bottom border */
    for (i = 0; i < count; i++)
        printf("--------");
    printf("\n");

    /* Time values */
    printf("%-8d", ganttStart[0]);

    for (i = 0; i < count; i++)
        printf("%-8d", ganttEnd[i]);

    printf("\n");
}

/* ---------------------------------------------------------
   FCFS Scheduling
   --------------------------------------------------------- */
void fcfs(struct Process p[], int n)
{
    int i;
    float avgwt = 0, avgtat = 0;

    int ganttPid[20];
    int ganttStart[20];
    int ganttEnd[20];

    int time = 0;

    p[0].wt = 0;

    for (i = 0; i < n; i++)
    {
        ganttPid[i] = p[i].pid;
        ganttStart[i] = time;

        p[i].wt = time;

        time += p[i].bt;

        ganttEnd[i] = time;
    }

    drawGanttChart(ganttPid, ganttStart, ganttEnd, n);

    printf("\nPID\tBT\tWT\tTAT\n");

    for (i = 0; i < n; i++)
    {
        p[i].tat = p[i].wt + p[i].bt;

        avgwt += p[i].wt;
        avgtat += p[i].tat;

        printf("P%d\t%d\t%d\t%d\n",
               p[i].pid,
               p[i].bt,
               p[i].wt,
               p[i].tat);
    }

    printf("\nAverage Waiting Time = %.2f",
           avgwt / n);

    printf("\nAverage Turnaround Time = %.2f\n",
           avgtat / n);
}


/* ---------------------------------------------------------
   SJF Scheduling
   --------------------------------------------------------- */
void sjf(struct Process p[], int n)
{
    int i, j;
    struct Process temp;

    float avgwt = 0, avgtat = 0;

    int ganttPid[20];
    int ganttStart[20];
    int ganttEnd[20];

    int time = 0;

    /* Sort according to Burst Time */
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

    for (i = 0; i < n; i++)
    {
        ganttPid[i] = p[i].pid;
        ganttStart[i] = time;

        p[i].wt = time;

        time += p[i].bt;

        ganttEnd[i] = time;
    }

    drawGanttChart(ganttPid, ganttStart, ganttEnd, n);

    printf("\nPID\tBT\tWT\tTAT\n");

    for (i = 0; i < n; i++)
    {
        p[i].tat = p[i].wt + p[i].bt;

        avgwt += p[i].wt;
        avgtat += p[i].tat;

        printf("P%d\t%d\t%d\t%d\n",
               p[i].pid,
               p[i].bt,
               p[i].wt,
               p[i].tat);
    }

    printf("\nAverage Waiting Time = %.2f",
           avgwt / n);

    printf("\nAverage Turnaround Time = %.2f\n",
           avgtat / n);
}


/* ---------------------------------------------------------
   Priority Scheduling
   --------------------------------------------------------- */
void priorityScheduling(struct Process p[], int n)
{
    int i, j;
    struct Process temp;

    float avgwt = 0, avgtat = 0;

    int ganttPid[20];
    int ganttStart[20];
    int ganttEnd[20];

    int time = 0;

    /* Sort according to Priority
       Lower number = Higher priority */
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

    for (i = 0; i < n; i++)
    {
        ganttPid[i] = p[i].pid;
        ganttStart[i] = time;

        p[i].wt = time;

        time += p[i].bt;

        ganttEnd[i] = time;
    }

    drawGanttChart(ganttPid, ganttStart, ganttEnd, n);

    printf("\nPID\tBT\tPriority\tWT\tTAT\n");

    for (i = 0; i < n; i++)
    {
        p[i].tat = p[i].wt + p[i].bt;

        avgwt += p[i].wt;
        avgtat += p[i].tat;

        printf("P%d\t%d\t%d\t\t%d\t%d\n",
               p[i].pid,
               p[i].bt,
               p[i].priority,
               p[i].wt,
               p[i].tat);
    }

    printf("\nAverage Waiting Time = %.2f",
           avgwt / n);

    printf("\nAverage Turnaround Time = %.2f\n",
           avgtat / n);
}


/* ---------------------------------------------------------
   Round Robin Scheduling
   --------------------------------------------------------- */
void roundRobin(struct Process p[], int n, int tq)
{
    int rem[20];
    int wt[20] = {0};
    int tat[20];

    int time = 0;
    int i;
    int done;

    float avgwt = 0, avgtat = 0;

    /* 
       Maximum number of Gantt segments.
       Each process can execute multiple times.
    */
    int ganttPid[200];
    int ganttStart[200];
    int ganttEnd[200];

    int gc = 0;

    for (i = 0; i < n; i++)
        rem[i] = p[i].bt;

    /*
       Round Robin execution
    */
    do
    {
        done = 1;

        for (i = 0; i < n; i++)
        {
            if (rem[i] > 0)
            {
                done = 0;

                /*
                   Store start time
                */
                ganttPid[gc] = p[i].pid;
                ganttStart[gc] = time;

                if (rem[i] > tq)
                {
                    time += tq;
                    rem[i] -= tq;
                }
                else
                {
                    time += rem[i];

                    wt[i] = time - p[i].bt;

                    rem[i] = 0;
                }

                /*
                   Store end time
                */
                ganttEnd[gc] = time;

                gc++;
            }
        }

    } while (!done);

    drawGanttChart(ganttPid, ganttStart, ganttEnd, gc);

    printf("\nPID\tBT\tWT\tTAT\n");

    for (i = 0; i < n; i++)
    {
        tat[i] = wt[i] + p[i].bt;

        avgwt += wt[i];
        avgtat += tat[i];

        printf("P%d\t%d\t%d\t%d\n",
               p[i].pid,
               p[i].bt,
               wt[i],
               tat[i]);
    }

    printf("\nAverage Waiting Time = %.2f",
           avgwt / n);

    printf("\nAverage Turnaround Time = %.2f\n",
           avgtat / n);
}


/* ---------------------------------------------------------
   Main Function
   --------------------------------------------------------- */
int main()
{
    struct Process p[20], temp[20];

    int n, i, ch, tq;

    printf("Enter number of processes: ");
    scanf("%d", &n);

    /* Input process details */
    for (i = 0; i < n; i++)
    {
        p[i].pid = i + 1;

        printf("\nProcess %d\n", i + 1);

        printf("Burst Time: ");
        scanf("%d", &p[i].bt);

        printf("Priority: ");
        scanf("%d", &p[i].priority);
    }

    /* Menu */
    while (1)
    {
        printf("\n=================================");
        printf("\n     CPU Scheduling Algorithms");
        printf("\n=================================");

        printf("\n1. FCFS");
        printf("\n2. SJF");
        printf("\n3. Priority Scheduling");
        printf("\n4. Round Robin");
        printf("\n5. Exit");

        printf("\n\nEnter your choice: ");
        scanf("%d", &ch);

        /*
           Copy original process data into temp.
           This prevents sorting in SJF/Priority
           from affecting the original data.
        */
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
                printf("\nExiting...\n");
                return 0;

            default:
                printf("\nInvalid Choice\n");
        }
    }

    return 0;
}
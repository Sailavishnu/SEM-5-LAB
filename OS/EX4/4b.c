#include <stdio.h>
#include <limits.h>

struct Process
{
    int pid;
    int at; // Arrival Time
    int bt; // Burst Time
    int priority;
    int wt;  // Waiting Time
    int tat; // Turnaround Time
    int rt;  // Remaining Time
};

struct Process p[20];
int n;

/*
   priorityFlag:
   1 -> Lower number = Higher priority
   2 -> Higher number = Higher priority
*/
int priorityFlag;

/* =========================================================
   GANTT CHART
   ========================================================= */

void printGanttChart(int ganttPid[], int ganttStart[],
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
    {
        if (ganttPid[i] == 0)
            printf("| IDLE   ");
        else
            printf("|  P%-3d ", ganttPid[i]);
    }

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

/* =========================================================
   FCFS
   ========================================================= */

void fcfs()
{
    struct Process s[20], temp;

    int i, j;
    int time = 0;

    float avgwt = 0;
    float avgtat = 0;

    int ganttPid[100];
    int ganttStart[100];
    int ganttEnd[100];
    int gc = 0;

    /* Copy processes */
    for (i = 0; i < n; i++)
        s[i] = p[i];

    /* Sort according to Arrival Time */
    for (i = 0; i < n - 1; i++)
    {
        for (j = 0; j < n - i - 1; j++)
        {
            if (s[j].at > s[j + 1].at)
            {
                temp = s[j];
                s[j] = s[j + 1];
                s[j + 1] = temp;
            }
        }
    }

    for (i = 0; i < n; i++)
    {
        /* CPU idle */
        if (time < s[i].at)
        {
            ganttPid[gc] = 0;
            ganttStart[gc] = time;
            ganttEnd[gc] = s[i].at;
            gc++;

            time = s[i].at;
        }

        ganttPid[gc] = s[i].pid;
        ganttStart[gc] = time;

        time += s[i].bt;

        ganttEnd[gc] = time;
        gc++;

        /*
           Completion Time = time

           TAT = Completion Time - Arrival Time
           WT  = TAT - Burst Time
        */
        s[i].tat = time - s[i].at;
        s[i].wt = s[i].tat - s[i].bt;
    }

    printGanttChart(ganttPid, ganttStart, ganttEnd, gc);

    printf("\nProcess\tAT\tBT\tWT\tTAT\n");

    for (i = 0; i < n; i++)
    {
        avgwt += s[i].wt;
        avgtat += s[i].tat;

        printf("P%d\t%d\t%d\t%d\t%d\n",
               s[i].pid,
               s[i].at,
               s[i].bt,
               s[i].wt,
               s[i].tat);
    }

    printf("\nAverage Waiting Time = %.2f", avgwt / n);
    printf("\nAverage Turnaround Time = %.2f\n", avgtat / n);
}

/* =========================================================
   SJF - NON PREEMPTIVE
   ========================================================= */

void sjf()
{
    struct Process s[20];

    int done[20] = {0};

    int i;
    int count = 0;
    int time = 0;
    int idx;

    float avgwt = 0;
    float avgtat = 0;

    int ganttPid[100];
    int ganttStart[100];
    int ganttEnd[100];
    int gc = 0;

    for (i = 0; i < n; i++)
        s[i] = p[i];

    while (count < n)
    {
        idx = -1;

        int minbt = INT_MAX;

        /*
           Select shortest available process
        */
        for (i = 0; i < n; i++)
        {
            if (!done[i] &&
                s[i].at <= time &&
                s[i].bt < minbt)
            {
                minbt = s[i].bt;
                idx = i;
            }
        }

        /*
           No process is available.
           CPU becomes idle.
        */
        if (idx == -1)
        {
            int nextAt = INT_MAX;

            for (i = 0; i < n; i++)
            {
                if (!done[i] && s[i].at < nextAt)
                    nextAt = s[i].at;
            }

            ganttPid[gc] = 0;
            ganttStart[gc] = time;
            ganttEnd[gc] = nextAt;
            gc++;

            time = nextAt;
            continue;
        }

        ganttPid[gc] = s[idx].pid;
        ganttStart[gc] = time;

        time += s[idx].bt;

        ganttEnd[gc] = time;
        gc++;

        s[idx].tat = time - s[idx].at;
        s[idx].wt = s[idx].tat - s[idx].bt;

        done[idx] = 1;
        count++;
    }

    printGanttChart(ganttPid, ganttStart, ganttEnd, gc);

    printf("\nProcess\tAT\tBT\tWT\tTAT\n");

    for (i = 0; i < n; i++)
    {
        avgwt += s[i].wt;
        avgtat += s[i].tat;

        printf("P%d\t%d\t%d\t%d\t%d\n",
               s[i].pid,
               s[i].at,
               s[i].bt,
               s[i].wt,
               s[i].tat);
    }

    printf("\nAverage Waiting Time = %.2f", avgwt / n);
    printf("\nAverage Turnaround Time = %.2f\n", avgtat / n);
}

/* =========================================================
   SRTF - PREEMPTIVE SJF
   ========================================================= */

void srtf()
{
    struct Process s[20];

    int i;

    int time = 0;
    int completed = 0;

    float avgwt = 0;
    float avgtat = 0;

    int ganttPid[200];
    int ganttStart[200];
    int ganttEnd[200];
    int gc = 0;

    int minAt = INT_MAX;

    for (i = 0; i < n; i++)
    {
        s[i] = p[i];
        s[i].rt = s[i].bt;

        if (s[i].at < minAt)
            minAt = s[i].at;
    }

    time = minAt;

    while (completed < n)
    {
        int idx = -1;
        int minrt = INT_MAX;

        /*
           Select process with shortest
           remaining time.
        */
        for (i = 0; i < n; i++)
        {
            if (s[i].at <= time &&
                s[i].rt > 0 &&
                s[i].rt < minrt)
            {
                minrt = s[i].rt;
                idx = i;
            }
        }

        /*
           CPU idle
        */
        if (idx == -1)
        {
            int nextAt = INT_MAX;

            for (i = 0; i < n; i++)
            {
                if (s[i].rt > 0 &&
                    s[i].at < nextAt)
                {
                    nextAt = s[i].at;
                }
            }

            ganttPid[gc] = 0;
            ganttStart[gc] = time;
            ganttEnd[gc] = nextAt;
            gc++;

            time = nextAt;
            continue;
        }

        /*
           Continue previous Gantt block
           if the same process is running.
        */
        if (gc > 0 &&
            ganttPid[gc - 1] == s[idx].pid)
        {
            s[idx].rt--;
            time++;

            ganttEnd[gc - 1] = time;
        }
        else
        {
            ganttPid[gc] = s[idx].pid;
            ganttStart[gc] = time;

            s[idx].rt--;
            time++;

            ganttEnd[gc] = time;
            gc++;
        }

        /*
           Process completed
        */
        if (s[idx].rt == 0)
        {
            completed++;

            s[idx].tat = time - s[idx].at;
            s[idx].wt = s[idx].tat - s[idx].bt;
        }
    }

    printGanttChart(ganttPid, ganttStart, ganttEnd, gc);

    printf("\nProcess\tAT\tBT\tWT\tTAT\n");

    for (i = 0; i < n; i++)
    {
        avgwt += s[i].wt;
        avgtat += s[i].tat;

        printf("P%d\t%d\t%d\t%d\t%d\n",
               s[i].pid,
               s[i].at,
               s[i].bt,
               s[i].wt,
               s[i].tat);
    }

    printf("\nAverage Waiting Time = %.2f", avgwt / n);
    printf("\nAverage Turnaround Time = %.2f\n", avgtat / n);
}

/* =========================================================
   PRIORITY - NON PREEMPTIVE
   ========================================================= */

void priorityScheduling()
{
    struct Process s[20];

    int done[20] = {0};

    int i;
    int count = 0;
    int time = 0;
    int idx;

    float avgwt = 0;
    float avgtat = 0;

    int ganttPid[100];
    int ganttStart[100];
    int ganttEnd[100];
    int gc = 0;

    for (i = 0; i < n; i++)
        s[i] = p[i];

    while (count < n)
    {
        idx = -1;

        /*
           Find best priority among available processes.
        */

        for (i = 0; i < n; i++)
        {
            if (!done[i] && s[i].at <= time)
            {
                if (idx == -1)
                {
                    idx = i;
                }
                else if (priorityFlag == 1)
                {
                    /*
                       Lower number = Higher priority
                    */
                    if (s[i].priority < s[idx].priority)
                        idx = i;
                }
                else
                {
                    /*
                       Higher number = Higher priority
                    */
                    if (s[i].priority > s[idx].priority)
                        idx = i;
                }
            }
        }

        /*
           No process available.
        */
        if (idx == -1)
        {
            int nextAt = INT_MAX;

            for (i = 0; i < n; i++)
            {
                if (!done[i] &&
                    s[i].at < nextAt)
                {
                    nextAt = s[i].at;
                }
            }

            ganttPid[gc] = 0;
            ganttStart[gc] = time;
            ganttEnd[gc] = nextAt;
            gc++;

            time = nextAt;
            continue;
        }

        ganttPid[gc] = s[idx].pid;
        ganttStart[gc] = time;

        time += s[idx].bt;

        ganttEnd[gc] = time;
        gc++;

        s[idx].tat = time - s[idx].at;
        s[idx].wt = s[idx].tat - s[idx].bt;

        done[idx] = 1;
        count++;
    }

    printGanttChart(ganttPid, ganttStart, ganttEnd, gc);

    printf("\nProcess\tAT\tBT\tPriority\tWT\tTAT\n");

    for (i = 0; i < n; i++)
    {
        avgwt += s[i].wt;
        avgtat += s[i].tat;

        printf("P%d\t%d\t%d\t%d\t\t%d\t%d\n",
               s[i].pid,
               s[i].at,
               s[i].bt,
               s[i].priority,
               s[i].wt,
               s[i].tat);
    }

    printf("\nAverage Waiting Time = %.2f", avgwt / n);
    printf("\nAverage Turnaround Time = %.2f\n", avgtat / n);
}

/* =========================================================
   PRIORITY - PREEMPTIVE
   ========================================================= */

void preemptivePriority()
{
    struct Process s[20];

    int i;

    int time = 0;
    int completed = 0;

    float avgwt = 0;
    float avgtat = 0;

    int ganttPid[200];
    int ganttStart[200];
    int ganttEnd[200];
    int gc = 0;

    int minAt = INT_MAX;

    for (i = 0; i < n; i++)
    {
        s[i] = p[i];
        s[i].rt = s[i].bt;

        if (s[i].at < minAt)
            minAt = s[i].at;
    }

    time = minAt;

    while (completed < n)
    {
        int idx = -1;

        /*
           Find best priority among available processes.
        */
        for (i = 0; i < n; i++)
        {
            if (s[i].at <= time &&
                s[i].rt > 0)
            {
                if (idx == -1)
                {
                    idx = i;
                }
                else if (priorityFlag == 1)
                {
                    /*
                       Lower number = Higher priority
                    */
                    if (s[i].priority < s[idx].priority)
                        idx = i;
                }
                else
                {
                    /*
                       Higher number = Higher priority
                    */
                    if (s[i].priority > s[idx].priority)
                        idx = i;
                }
            }
        }

        /*
           CPU idle
        */
        if (idx == -1)
        {
            int nextAt = INT_MAX;

            for (i = 0; i < n; i++)
            {
                if (s[i].rt > 0 &&
                    s[i].at < nextAt)
                {
                    nextAt = s[i].at;
                }
            }

            ganttPid[gc] = 0;
            ganttStart[gc] = time;
            ganttEnd[gc] = nextAt;
            gc++;

            time = nextAt;
            continue;
        }

        /*
           Continue previous block if same process.
        */
        if (gc > 0 &&
            ganttPid[gc - 1] == s[idx].pid)
        {
            s[idx].rt--;
            time++;

            ganttEnd[gc - 1] = time;
        }
        else
        {
            ganttPid[gc] = s[idx].pid;
            ganttStart[gc] = time;

            s[idx].rt--;
            time++;

            ganttEnd[gc] = time;
            gc++;
        }

        /*
           Process completed
        */
        if (s[idx].rt == 0)
        {
            completed++;

            s[idx].tat = time - s[idx].at;
            s[idx].wt = s[idx].tat - s[idx].bt;
        }
    }

    printGanttChart(ganttPid, ganttStart, ganttEnd, gc);

    printf("\nProcess\tAT\tBT\tPriority\tWT\tTAT\n");

    for (i = 0; i < n; i++)
    {
        avgwt += s[i].wt;
        avgtat += s[i].tat;

        printf("P%d\t%d\t%d\t%d\t\t%d\t%d\n",
               s[i].pid,
               s[i].at,
               s[i].bt,
               s[i].priority,
               s[i].wt,
               s[i].tat);
    }

    printf("\nAverage Waiting Time = %.2f", avgwt / n);
    printf("\nAverage Turnaround Time = %.2f\n", avgtat / n);
}

/* =========================================================
   ROUND ROBIN
   ========================================================= */

void roundRobin()
{
    struct Process r[20];

    int i;
    int tq;

    int time = 0;
    int completed = 0;

    int completedFlag[20] = {0};
    int inQueue[20] = {0};

    int queue[200];
    int front = 0;
    int rear = 0;

    float avgwt = 0;
    float avgtat = 0;

    int ganttPid[200];
    int ganttStart[200];
    int ganttEnd[200];
    int gc = 0;

    int minAt = INT_MAX;

    for (i = 0; i < n; i++)
    {
        r[i] = p[i];
        r[i].rt = r[i].bt;

        if (r[i].at < minAt)
            minAt = r[i].at;
    }

    printf("Enter Time Quantum: ");
    scanf("%d", &tq);

    time = minAt;

    /*
       Add initially available processes.
    */
    for (i = 0; i < n; i++)
    {
        if (r[i].at <= time &&
            !inQueue[i] &&
            !completedFlag[i])
        {
            queue[rear++] = i;
            inQueue[i] = 1;
        }
    }

    while (completed < n)
    {
        /*
           Ready queue empty -> CPU idle.
        */
        if (front == rear)
        {
            int nextAt = INT_MAX;

            for (i = 0; i < n; i++)
            {
                if (!completedFlag[i] &&
                    r[i].at < nextAt)
                {
                    nextAt = r[i].at;
                }
            }

            if (time < nextAt)
            {
                ganttPid[gc] = 0;
                ganttStart[gc] = time;
                ganttEnd[gc] = nextAt;
                gc++;
            }

            time = nextAt;

            /*
               Add newly arrived processes.
            */
            for (i = 0; i < n; i++)
            {
                if (r[i].at <= time &&
                    !inQueue[i] &&
                    !completedFlag[i])
                {
                    queue[rear++] = i;
                    inQueue[i] = 1;
                }
            }

            continue;
        }

        /*
           Take first process from queue.
        */
        int idx = queue[front++];

        inQueue[idx] = 0;

        int start = time;
        int exec;

        if (r[idx].rt > tq)
            exec = tq;
        else
            exec = r[idx].rt;

        time += exec;
        r[idx].rt -= exec;

        /*
           Store Gantt block.
        */
        ganttPid[gc] = r[idx].pid;
        ganttStart[gc] = start;
        ganttEnd[gc] = time;
        gc++;

        /*
           Add processes that arrived during execution.
        */
        for (i = 0; i < n; i++)
        {
            if (r[i].at <= time &&
                !inQueue[i] &&
                !completedFlag[i] &&
                i != idx)
            {
                queue[rear++] = i;
                inQueue[i] = 1;
            }
        }

        /*
           Process still has remaining time.
           Put it at the back.
        */
        if (r[idx].rt > 0)
        {
            queue[rear++] = idx;
            inQueue[idx] = 1;
        }
        else
        {
            /*
               Process completed.
            */
            completedFlag[idx] = 1;
            completed++;

            r[idx].tat = time - r[idx].at;
            r[idx].wt = r[idx].tat - r[idx].bt;
        }
    }

    printGanttChart(ganttPid, ganttStart, ganttEnd, gc);

    printf("\nProcess\tAT\tBT\tWT\tTAT\n");

    for (i = 0; i < n; i++)
    {
        avgwt += r[i].wt;
        avgtat += r[i].tat;

        printf("P%d\t%d\t%d\t%d\t%d\n",
               r[i].pid,
               r[i].at,
               r[i].bt,
               r[i].wt,
               r[i].tat);
    }

    printf("\nAverage Waiting Time = %.2f", avgwt / n);
    printf("\nAverage Turnaround Time = %.2f\n", avgtat / n);
}

/* =========================================================
   MAIN
   ========================================================= */

int main()
{
    int i;
    int choice;

    printf("Enter number of processes: ");
    scanf("%d", &n);

    /*
       Get process information.
    */
    for (i = 0; i < n; i++)
    {
        p[i].pid = i + 1;

        printf("\nProcess %d\n", i + 1);

        printf("Arrival Time: ");
        scanf("%d", &p[i].at);

        printf("Burst Time: ");
        scanf("%d", &p[i].bt);

        printf("Priority: ");
        scanf("%d", &p[i].priority);
    }

    /*
       Get priority direction.
    */
    printf("\n========================================");
    printf("\n          Priority Type");
    printf("\n========================================");

    printf("\n1. Lower number = Higher priority");
    printf("\n2. Higher number = Higher priority");

    printf("\n\nEnter choice: ");
    scanf("%d", &priorityFlag);

    /*
       Scheduling menu
    */
    do
    {
        printf("\n\n========================================");
        printf("\n          CPU Scheduling Menu");
        printf("\n========================================");

        printf("\n1. FCFS");
        printf("\n2. SJF (Non-Preemptive)");
        printf("\n3. SRTF (Preemptive SJF)");
        printf("\n4. Priority Scheduling (Non-Preemptive)");
        printf("\n5. Priority Scheduling (Preemptive)");
        printf("\n6. Round Robin");
        printf("\n7. Exit");

        printf("\n\nEnter your choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            fcfs();
            break;

        case 2:
            sjf();
            break;

        case 3:
            srtf();
            break;

        case 4:
            priorityScheduling();
            break;

        case 5:
            preemptivePriority();
            break;

        case 6:
            roundRobin();
            break;

        case 7:
            printf("\nExiting...\n");
            break;

        default:
            printf("\nInvalid Choice\n");
        }

    } while (choice != 7);

    return 0;
}
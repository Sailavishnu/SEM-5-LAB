#include <stdio.h>
#define MAX 100

typedef struct {
    int id;
    int arrival;
    int burst;
    int priority;
    int remaining;
    int completion;
    int turnaround;
    int waiting;
} Process;

/* copy the input, reset the remaining time */
void copyProcesses(Process src[], Process p[], int n)
{
    for (int i = 0; i < n; i++) {
        p[i] = src[i];
        p[i].remaining = p[i].burst;
    }
}

/* after completion is known: TAT = CT - AT, WT = TAT - BT */
void printAverages(char name[], Process p[], int n)
{
    float totalWT = 0, totalTAT = 0;
    for (int i = 0; i < n; i++) {
        p[i].turnaround = p[i].completion - p[i].arrival;
        p[i].waiting = p[i].turnaround - p[i].burst;
        totalWT += p[i].waiting;
        totalTAT += p[i].turnaround;
    }
    printf("%s: Avg WT = %.2f, Avg TAT = %.2f\n", name, totalWT / n, totalTAT / n);
}

/* ---------- 1. FCFS ---------- */
void fcfs(Process src[], int n)
{
    Process p[MAX];
    int time = 0;

    copyProcesses(src, p, n);

    /* sort by arrival time (bubble sort) */
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - i - 1; j++)
            if (p[j].arrival > p[j + 1].arrival) {
                Process t = p[j];
                p[j] = p[j + 1];
                p[j + 1] = t;
            }

    for (int i = 0; i < n; i++) {
        if (time < p[i].arrival)
            time = p[i].arrival;          /* CPU waits if nobody is there */
        time = time + p[i].burst;
        p[i].completion = time;
    }

    printAverages("FCFS", p, n);
}

/* ---------- 2. SJF (non-preemptive) ---------- */
void sjf(Process src[], int n)
{
    Process p[MAX];
    int done[MAX] = {0};
    int time = 0, finished = 0;

    copyProcesses(src, p, n);

    while (finished < n) {
        /* find the shortest burst among processes that have arrived */
        int pick = -1;
        for (int i = 0; i < n; i++) {
            if (done[i] || p[i].arrival > time)
                continue;
            if (pick == -1 || p[i].burst < p[pick].burst)
                pick = i;
        }

        if (pick == -1) {
            /* nobody has arrived yet: jump to the next arrival */
            int next = 1000000;
            for (int i = 0; i < n; i++)
                if (!done[i] && p[i].arrival < next)
                    next = p[i].arrival;
            time = next;
        } else {
            /* run it to completion */
            time = time + p[pick].burst;
            p[pick].completion = time;
            done[pick] = 1;
            finished++;
        }
    }

    printAverages("SJF", p, n);
}

/* ---------- 3. SRTF (preemptive SJF) ---------- */
void srtf(Process src[], int n)
{
    Process p[MAX];
    int time = 0, finished = 0;

    copyProcesses(src, p, n);

    while (finished < n) {
        /* every time unit: shortest remaining time among arrived processes */
        int pick = -1;
        for (int i = 0; i < n; i++) {
            if (p[i].arrival > time || p[i].remaining == 0)
                continue;
            if (pick == -1 || p[i].remaining < p[pick].remaining)
                pick = i;
        }

        if (pick == -1) {
            time++;                       /* CPU idle for one unit */
        } else {
            p[pick].remaining--;          /* run one unit */
            time++;
            if (p[pick].remaining == 0) {
                p[pick].completion = time;
                finished++;
            }
        }
    }

    printAverages("SRTF", p, n);
}

/* ---------- 4. Priority (non-preemptive) ---------- */
void priorityNonPreemptive(Process src[], int n)
{
    Process p[MAX];
    int done[MAX] = {0};
    int time = 0, finished = 0;

    copyProcesses(src, p, n);

    while (finished < n) {
        /* lowest priority number among arrived processes */
        int pick = -1;
        for (int i = 0; i < n; i++) {
            if (done[i] || p[i].arrival > time)
                continue;
            if (pick == -1 || p[i].priority < p[pick].priority)
                pick = i;
        }

        if (pick == -1) {
            int next = 1000000;
            for (int i = 0; i < n; i++)
                if (!done[i] && p[i].arrival < next)
                    next = p[i].arrival;
            time = next;
        } else {
            time = time + p[pick].burst;
            p[pick].completion = time;
            done[pick] = 1;
            finished++;
        }
    }

    printAverages("Priority (Non-Preemptive)", p, n);
}

/* ---------- 5. Priority (preemptive) ---------- */
void priorityPreemptive(Process src[], int n)
{
    Process p[MAX];
    int time = 0, finished = 0;

    copyProcesses(src, p, n);

    while (finished < n) {
        /* every time unit: lowest priority number among arrived processes */
        int pick = -1;
        for (int i = 0; i < n; i++) {
            if (p[i].arrival > time || p[i].remaining == 0)
                continue;
            if (pick == -1 || p[i].priority < p[pick].priority)
                pick = i;
        }

        if (pick == -1) {
            time++;
        } else {
            p[pick].remaining--;
            time++;
            if (p[pick].remaining == 0) {
                p[pick].completion = time;
                finished++;
            }
        }
    }

    printAverages("Priority (Preemptive)", p, n);
}

/* ---------- 6. Round Robin ---------- */
void roundRobin(Process src[], int n, int quantum)
{
    Process p[MAX];
    int queue[MAX * 100];
    int inQueue[MAX] = {0};
    int time = 0, front = 0, rear = 0, finished = 0;

    copyProcesses(src, p, n);

    /* sort by arrival time so the queue order is correct */
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - i - 1; j++)
            if (p[j].arrival > p[j + 1].arrival) {
                Process t = p[j];
                p[j] = p[j + 1];
                p[j + 1] = t;
            }

    /* add every process that has arrived by 'time' to the queue */
    for (int i = 0; i < n; i++)
        if (p[i].arrival <= time) {
            queue[rear++] = i;
            inQueue[i] = 1;
        }

    while (finished < n) {
        /* queue empty: jump to the next arrival */
        if (front == rear) {
            int next = 1000000;
            for (int i = 0; i < n; i++)
                if (p[i].remaining > 0 && p[i].arrival < next)
                    next = p[i].arrival;
            time = next;
            for (int i = 0; i < n; i++)
                if (!inQueue[i] && p[i].arrival <= time) {
                    queue[rear++] = i;
                    inQueue[i] = 1;
                }
            continue;
        }

        /* run the first process for one quantum (or less) */
        int i = queue[front++];
        int run = (p[i].remaining < quantum) ? p[i].remaining : quantum;
        time = time + run;
        p[i].remaining = p[i].remaining - run;

        /* new arrivals join the queue first */
        for (int k = 0; k < n; k++)
            if (!inQueue[k] && p[k].arrival <= time) {
                queue[rear++] = k;
                inQueue[k] = 1;
            }

        /* then the preempted process goes to the back */
        if (p[i].remaining > 0) {
            queue[rear++] = i;
        } else {
            p[i].completion = time;
            finished++;
        }
    }

    printAverages("Round Robin", p, n);
}

/* ---------- main ---------- */
int main(void)
{
    Process all[MAX];
    int n, choice, quantum;

    printf("Enter number of processes: ");
    scanf("%d", &n);

    for (int i = 0; i < n; i++) {
        all[i].id = i + 1;
        printf("P%d arrival burst priority: ", i + 1);
        scanf("%d %d %d", &all[i].arrival, &all[i].burst, &all[i].priority);
    }

    do {
        printf("\n1.FCFS 2.SJF 3.SRTF 4.Priority NP 5.Priority P 6.Round Robin 7.Exit\nChoice: ");
        scanf("%d", &choice);

        if (choice == 1)      fcfs(all, n);
        else if (choice == 2) sjf(all, n);
        else if (choice == 3) srtf(all, n);
        else if (choice == 4) priorityNonPreemptive(all, n);
        else if (choice == 5) priorityPreemptive(all, n);
        else if (choice == 6) {
            printf("Time quantum: ");
            scanf("%d", &quantum);
            roundRobin(all, n, quantum);
        }
    } while (choice != 7);

    return 0;
}
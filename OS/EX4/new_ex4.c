
#include <stdio.h>
#include <stdbool.h>

#define MAX 100

typedef struct {
    int id, at, bt, rt, priority, lt, tat, wt;
} Process;

typedef struct {
    int pid, start, end;
} Block;

void input(Process p[], int *n) {
    printf("Number of processes: ");
    scanf("%d", n);

    for (int i = 0; i < *n; i++) {
        p[i].id = i + 1;

        printf("\nP%d Burst Time: ", i + 1);
        scanf("%d", &p[i].bt);

        printf("P%d Priority (lower = higher): ", i + 1);
        scanf("%d", &p[i].priority);

        printf("P%d Arrival Time: ", i + 1);
        scanf("%d", &p[i].at);

        p[i].rt = p[i].bt;
    }
}

void reset(Process p[], Process original[], int n) {
    for (int i = 0; i < n; i++) {
        p[i] = original[i];
        p[i].rt = p[i].bt;
        p[i].lt = 0;
    }
}

void gantt(Block g[], int size) {
    printf("\nGantt Chart:\n");

    for (int i = 0; i < size; i++) {
        if (g[i].pid == -1)
            printf("| IDLE ");
        else
            printf("| P%d ", g[i].pid);
    }
    printf("|\n%d", g[0].start);

    for (int i = 0; i < size; i++)
        printf("\t%d", g[i].end);

    printf("\n");
}

void results(Process p[], int n, Block g[], int size) {
    float twt = 0, ttat = 0;

    gantt(g, size);

    printf("\nPID\tAT\tBT\tPriority\tlt\tTAT\tWT\n");

    for (int i = 0; i < n; i++) {
        p[i].tat = p[i].lt - p[i].at;
        p[i].wt = p[i].tat - p[i].bt;

        twt += p[i].wt;
        ttat += p[i].tat;

        printf("P%d\t%d\t%d\t%d\t\t%d\t%d\t%d\n",
               p[i].id, p[i].at, p[i].bt, p[i].priority,
               p[i].lt, p[i].tat, p[i].wt);
    }

    printf("Average WT = %.2f\n", twt / n);
    printf("Average TAT = %.2f\n", ttat / n);
}

/* 1. FCFS */
void fcfs(Process p[], int n) {
    Block g[MAX * 2];
    int size = 0, time = 0;

    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - i - 1; j++)
            if (p[j].at > p[j + 1].at) {
                Process t = p[j];
                p[j] = p[j + 1];
                p[j + 1] = t;
            }

    for (int i = 0; i < n; i++) {
        if (time < p[i].at) {
            g[size++] = (Block){-1, time, p[i].at};
            time = p[i].at;
        }

        g[size++] = (Block){p[i].id, time, time + p[i].bt};
        time += p[i].bt;
        p[i].lt = time;
    }

    results(p, n, g, size);
}

/* 2. SJF Non-Preemptive */
void sjf(Process p[], int n) {
    Block g[MAX * 2];
    bool done[MAX] = {false};
    int size = 0, time = 0, count = 0;

    while (count < n) {
        int idx = -1;

        for (int i = 0; i < n; i++)
            if (!done[i] && p[i].at <= time &&
                (idx == -1 || p[i].bt < p[idx].bt))
                idx = i;

        if (idx == -1) {
            int next = 1000000000;

            for (int i = 0; i < n; i++)
                if (!done[i] && p[i].at < next)
                    next = p[i].at;

            g[size++] = (Block){-1, time, next};
            time = next;
        } else {
            g[size++] = (Block){p[idx].id, time, time + p[idx].bt};
            time += p[idx].bt;
            p[idx].lt = time;
            done[idx] = true;
            count++;
        }
    }

    results(p, n, g, size);
}

/* 3. SRTF Preemptive */
void srtf(Process p[], int n) {
    Block g[MAX * 100];
    int size = 0, time = 0, count = 0, prev = -2;

    while (count < n) {
        int idx = -1;

        for (int i = 0; i < n; i++)
            if (p[i].at <= time && p[i].rt > 0 &&
                (idx == -1 || p[i].rt < p[idx].rt))
                idx = i;

        if (idx == -1) {
            if (prev != -1) {
                g[size++] = (Block){-1, time, time + 1};
                prev = -1;
            } else {
                g[size - 1].end++;
            }
        } else {
            if (prev != p[idx].id) {
                g[size++] = (Block){p[idx].id, time, time + 1};
                prev = p[idx].id;
            } else {
                g[size - 1].end++;
            }

            p[idx].rt--;

            if (p[idx].rt == 0) {
                p[idx].lt = time + 1;
                count++;
            }
        }

        time++;
    }

    results(p, n, g, size);
}

/* 4. Priority Non-Preemptive */
void priority_np(Process p[], int n) {
    Block g[MAX * 2];
    bool done[MAX] = {false};
    int size = 0, time = 0, count = 0;

    while (count < n) {
        int idx = -1;

        for (int i = 0; i < n; i++)
            if (!done[i] && p[i].at <= time &&
                (idx == -1 || p[i].priority < p[idx].priority))
                idx = i;

        if (idx == -1) {
            int next = 1000000000;

            for (int i = 0; i < n; i++)
                if (!done[i] && p[i].at < next)
                    next = p[i].at;

            g[size++] = (Block){-1, time, next};
            time = next;
        } else {
            g[size++] = (Block){p[idx].id, time, time + p[idx].bt};
            time += p[idx].bt;
            p[idx].lt = time;
            done[idx] = true;
            count++;
        }
    }

    results(p, n, g, size);
}

/* 5. Priority Preemptive */
void priority_p(Process p[], int n) {
    Block g[MAX * 100];
    int size = 0, time = 0, count = 0, prev = -2;

    while (count < n) {
        int idx = -1;

        for (int i = 0; i < n; i++)
            if (p[i].at <= time && p[i].rt > 0 &&
                (idx == -1 || p[i].priority < p[idx].priority))
                idx = i;

        if (idx == -1) {
            if (prev != -1) {
                g[size++] = (Block){-1, time, time + 1};
                prev = -1;
            } else {
                g[size - 1].end++;
            }
        } else {
            if (prev != p[idx].id) {
                g[size++] = (Block){p[idx].id, time, time + 1};
                prev = p[idx].id;
            } else {
                g[size - 1].end++;
            }

            p[idx].rt--;

            if (p[idx].rt == 0) {
                p[idx].lt = time + 1;
                count++;
            }
        }

        time++;
    }

    results(p, n, g, size);
}

/* 6. Round Robin */
void round_robin(Process p[], int n, int q) {
    Block g[MAX * 100];
    int queue[MAX * 100], front = 0, rear = 0;
    bool added[MAX] = {false};
    int size = 0, time = 0, count = 0;

    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - i - 1; j++)
            if (p[j].at > p[j + 1].at) {
                Process t = p[j];
                p[j] = p[j + 1];
                p[j + 1] = t;
            }

    while (count < n) {
        if (front == rear) {
            int next = 1000000000;

            for (int i = 0; i < n; i++)
                if (p[i].rt > 0 && p[i].at < next)
                    next = p[i].at;

            if (time < next) {
                g[size++] = (Block){-1, time, next};
                time = next;
            }

            for (int i = 0; i < n; i++)
                if (!added[i] && p[i].at <= time && p[i].rt > 0) {
                    queue[rear++] = i;
                    added[i] = true;
                }
        }

        int idx = queue[front++];
        int run = p[idx].rt < q ? p[idx].rt : q;

        g[size++] = (Block){p[idx].id, time, time + run};
        time += run;
        p[idx].rt -= run;

        for (int i = 0; i < n; i++)
            if (!added[i] && p[i].at <= time && p[i].rt > 0) {
                queue[rear++] = i;
                added[i] = true;
            }

        if (p[idx].rt > 0)
            queue[rear++] = idx;
        else {
            p[idx].lt = time;
            count++;
        }
    }

    results(p, n, g, size);
}

int main() {
    Process original[MAX], p[MAX];
    int n, choice, q;

    input(original, &n);

    while (1) {
        reset(p, original, n);

        printf("\n1.FCFS  2.SJF  3.SRTF");
        printf("\n4.Priority NP  5.Priority P");
        printf("\n6.Round Robin  7.Change Input  8.Exit");
        printf("\nChoice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: fcfs(p, n); break;
            case 2: sjf(p, n); break;
            case 3: srtf(p, n); break;
            case 4: priority_np(p, n); break;
            case 5: priority_p(p, n); break;

            case 6:
                printf("Time quantum: ");
                scanf("%d", &q);
                if (q > 0)
                    round_robin(p, n, q);
                else
                    printf("Quantum must be positive.\n");
                break;

            case 7:
                input(original, &n);
                break;

            case 8: return 0;
            default: printf("Invalid choice\n");
        }
    }
}

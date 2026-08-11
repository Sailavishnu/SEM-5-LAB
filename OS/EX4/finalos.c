#include <stdio.h>
#include <limits.h>
#include <string.h>

struct Process {
    int pid, at, bt, priority, wt, tat, rt;
};

struct Process p[20];
int n, priorityFlag;

void printGanttChart(int ganttPid[], int ganttStart[], int ganttEnd[], int count) {
    printf("\nGantt Chart:\n\n");
    for (int i = 0; i < count; i++) printf("--------");
    printf("\n");
    for (int i = 0; i < count; i++)
        printf("%s", ganttPid[i] == 0 ? "| IDLE   " : (char[20]){0});
    for (int i = 0; i < count; i++)
        printf(ganttPid[i] == 0 ? "" : "|  P%-3d ", ganttPid[i]);
    printf("|\n");
    for (int i = 0; i < count; i++) printf("--------");
    printf("\n%-8d", ganttStart[0]);
    for (int i = 0; i < count; i++) printf("%-8d", ganttEnd[i]);
    printf("\n");
}

int findNextAt(struct Process s[], int done[], int checkBit) {
    int nextAt = INT_MAX;
    for (int i = 0; i < n; i++) {
        if (done[i] == checkBit && s[i].at < nextAt)
            nextAt = s[i].at;
    }
    return nextAt;
}

void addIdleBlock(int ganttPid[], int ganttStart[], int ganttEnd[], int *gc, int time, int nextAt) {
    ganttPid[*gc] = 0;
    ganttStart[*gc] = time;
    ganttEnd[(*gc)++] = nextAt;
}

void printResults(struct Process s[], const char *header, int showPriority) {
    float avgwt = 0, avgtat = 0;
    printf("\nProcess\tAT\tBT\t%s\tWT\tTAT\n", showPriority ? "Priority" : "");
    for (int i = 0; i < n; i++) {
        avgwt += s[i].wt;
        avgtat += s[i].tat;
        if (showPriority)
            printf("P%d\t%d\t%d\t%d\t\t%d\t%d\n", s[i].pid, s[i].at, s[i].bt, s[i].priority, s[i].wt, s[i].tat);
        else
            printf("P%d\t%d\t%d\t%d\t%d\n", s[i].pid, s[i].at, s[i].bt, s[i].wt, s[i].tat);
    }
    printf("\nAverage Waiting Time = %.2f", avgwt / n);
    printf("\nAverage Turnaround Time = %.2f\n", avgtat / n);
}

void fcfs() {
    struct Process s[20];
    memcpy(s, p, sizeof(p));
    
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - i - 1; j++)
            if (s[j].at > s[j + 1].at) {
                struct Process temp = s[j];
                s[j] = s[j + 1];
                s[j + 1] = temp;
            }
    
    int ganttPid[100], ganttStart[100], ganttEnd[100], gc = 0, time = 0;
    for (int i = 0; i < n; i++) {
        if (time < s[i].at) {
            addIdleBlock(ganttPid, ganttStart, ganttEnd, &gc, time, s[i].at);
            time = s[i].at;
        }
        ganttPid[gc] = s[i].pid;
        ganttStart[gc] = time;
        time += s[i].bt;
        ganttEnd[gc++] = time;
        s[i].tat = time - s[i].at;
        s[i].wt = s[i].tat - s[i].bt;
    }
    printGanttChart(ganttPid, ganttStart, ganttEnd, gc);
    printResults(s, "", 0);
}

void sjf() {
    struct Process s[20];
    int done[20] = {0};
    memcpy(s, p, sizeof(p));
    
    int count = 0, time = 0, gc = 0;
    int ganttPid[100], ganttStart[100], ganttEnd[100];
    
    while (count < n) {
        int idx = -1, minbt = INT_MAX;
        for (int i = 0; i < n; i++)
            if (!done[i] && s[i].at <= time && s[i].bt < minbt) {
                minbt = s[i].bt;
                idx = i;
            }
        
        if (idx == -1) {
            int nextAt = findNextAt(s, done, 0);
            addIdleBlock(ganttPid, ganttStart, ganttEnd, &gc, time, nextAt);
            time = nextAt;
            continue;
        }
        
        ganttPid[gc] = s[idx].pid;
        ganttStart[gc] = time;
        time += s[idx].bt;
        ganttEnd[gc++] = time;
        s[idx].tat = time - s[idx].at;
        s[idx].wt = s[idx].tat - s[idx].bt;
        done[idx] = 1;
        count++;
    }
    printGanttChart(ganttPid, ganttStart, ganttEnd, gc);
    printResults(s, "", 0);
}

void srtf() {
    struct Process s[20];
    int minAt = INT_MAX;
    memcpy(s, p, sizeof(p));
    
    for (int i = 0; i < n; i++) {
        s[i].rt = s[i].bt;
        if (s[i].at < minAt) minAt = s[i].at;
    }
    
    int time = minAt, completed = 0, gc = 0;
    int ganttPid[200], ganttStart[200], ganttEnd[200];
    
    while (completed < n) {
        int idx = -1, minrt = INT_MAX;
        for (int i = 0; i < n; i++)
            if (s[i].at <= time && s[i].rt > 0 && s[i].rt < minrt) {
                minrt = s[i].rt;
                idx = i;
            }
        
        if (idx == -1) {
            int nextAt = INT_MAX;
            for (int i = 0; i < n; i++)
                if (s[i].rt > 0 && s[i].at < nextAt) nextAt = s[i].at;
            addIdleBlock(ganttPid, ganttStart, ganttEnd, &gc, time, nextAt);
            time = nextAt;
            continue;
        }
        
        if (gc > 0 && ganttPid[gc - 1] == s[idx].pid) {
            ganttEnd[gc - 1] = ++time;
        } else {
            ganttPid[gc] = s[idx].pid;
            ganttStart[gc] = time;
            ganttEnd[gc++] = ++time;
        }
        
        s[idx].rt--;
        if (s[idx].rt == 0) {
            completed++;
            s[idx].tat = time - s[idx].at;
            s[idx].wt = s[idx].tat - s[idx].bt;
        }
    }
    printGanttChart(ganttPid, ganttStart, ganttEnd, gc);
    printResults(s, "", 0);
}

void priorityScheduling() {
    struct Process s[20];
    int done[20] = {0};
    memcpy(s, p, sizeof(p));
    
    int count = 0, time = 0, gc = 0;
    int ganttPid[100], ganttStart[100], ganttEnd[100];
    
    while (count < n) {
        int idx = -1, bestPriority;
        (priorityFlag == 1) ? (bestPriority = INT_MAX) : (bestPriority = INT_MIN);
        
        for (int i = 0; i < n; i++) {
            if (!done[i] && s[i].at <= time) {
                if ((priorityFlag == 1 && s[i].priority < bestPriority) ||
                    (priorityFlag == 2 && s[i].priority > bestPriority)) {
                    bestPriority = s[i].priority;
                    idx = i;
                }
            }
        }
        
        if (idx == -1) {
            int nextAt = findNextAt(s, done, 0);
            addIdleBlock(ganttPid, ganttStart, ganttEnd, &gc, time, nextAt);
            time = nextAt;
            continue;
        }
        
        ganttPid[gc] = s[idx].pid;
        ganttStart[gc] = time;
        time += s[idx].bt;
        ganttEnd[gc++] = time;
        s[idx].tat = time - s[idx].at;
        s[idx].wt = s[idx].tat - s[idx].bt;
        done[idx] = 1;
        count++;
    }
    printGanttChart(ganttPid, ganttStart, ganttEnd, gc);
    printResults(s, "", 1);
}

void preemptivePriority() {
    struct Process s[20];
    int minAt = INT_MAX;
    memcpy(s, p, sizeof(p));
    
    for (int i = 0; i < n; i++) {
        s[i].rt = s[i].bt;
        if (s[i].at < minAt) minAt = s[i].at;
    }
    
    int time = minAt, completed = 0, gc = 0;
    int ganttPid[200], ganttStart[200], ganttEnd[200];
    
    while (completed < n) {
        int idx = -1, bestPriority;
        (priorityFlag == 1) ? (bestPriority = INT_MAX) : (bestPriority = INT_MIN);
        
        for (int i = 0; i < n; i++) {
            if (s[i].at <= time && s[i].rt > 0) {
                if ((priorityFlag == 1 && s[i].priority < bestPriority) ||
                    (priorityFlag == 2 && s[i].priority > bestPriority)) {
                    bestPriority = s[i].priority;
                    idx = i;
                }
            }
        }
        
        if (idx == -1) {
            int nextAt = INT_MAX;
            for (int i = 0; i < n; i++)
                if (s[i].rt > 0 && s[i].at < nextAt) nextAt = s[i].at;
            addIdleBlock(ganttPid, ganttStart, ganttEnd, &gc, time, nextAt);
            time = nextAt;
            continue;
        }
        
        if (gc > 0 && ganttPid[gc - 1] == s[idx].pid) {
            ganttEnd[gc - 1] = ++time;
        } else {
            ganttPid[gc] = s[idx].pid;
            ganttStart[gc] = time;
            ganttEnd[gc++] = ++time;
        }
        
        s[idx].rt--;
        if (s[idx].rt == 0) {
            completed++;
            s[idx].tat = time - s[idx].at;
            s[idx].wt = s[idx].tat - s[idx].bt;
        }
    }
    printGanttChart(ganttPid, ganttStart, ganttEnd, gc);
    printResults(s, "", 1);
}

void roundRobin() {
    struct Process r[20];
    int completedFlag[20] = {0}, inQueue[20] = {0};
    int minAt = INT_MAX;
    memcpy(r, p, sizeof(p));
    
    for (int i = 0; i < n; i++) {
        r[i].rt = r[i].bt;
        if (r[i].at < minAt) minAt = r[i].at;
    }
    
    int tq, time = minAt, completed = 0, gc = 0;
    int ganttPid[200], ganttStart[200], ganttEnd[200];
    int queue[200], front = 0, rear = 0;
    
    printf("Enter Time Quantum: ");
    scanf("%d", &tq);
    
    for (int i = 0; i < n; i++)
        if (r[i].at <= time) queue[rear++] = i, inQueue[i] = 1;
    
    while (completed < n) {
        if (front == rear) {
            int nextAt = INT_MAX;
            for (int i = 0; i < n; i++)
                if (!completedFlag[i] && r[i].at < nextAt) nextAt = r[i].at;
            if (time < nextAt) addIdleBlock(ganttPid, ganttStart, ganttEnd, &gc, time, nextAt);
            time = nextAt;
            for (int i = 0; i < n; i++)
                if (r[i].at <= time && !inQueue[i] && !completedFlag[i])
                    queue[rear++] = i, inQueue[i] = 1;
            continue;
        }
        
        int idx = queue[front++];
        inQueue[idx] = 0;
        int exec = (r[idx].rt > tq) ? tq : r[idx].rt;
        
        ganttPid[gc] = r[idx].pid;
        ganttStart[gc] = time;
        time += exec;
        ganttEnd[gc++] = time;
        r[idx].rt -= exec;
        
        for (int i = 0; i < n; i++)
            if (r[i].at <= time && !inQueue[i] && !completedFlag[i] && i != idx)
                queue[rear++] = i, inQueue[i] = 1;
        
        if (r[idx].rt > 0) queue[rear++] = idx, inQueue[idx] = 1;
        else {
            completedFlag[idx] = 1;
            completed++;
            r[idx].tat = time - r[idx].at;
            r[idx].wt = r[idx].tat - r[idx].bt;
        }
    }
    printGanttChart(ganttPid, ganttStart, ganttEnd, gc);
    printResults(r, "", 0);
}

int main() {
    printf("Enter number of processes: ");
    scanf("%d", &n);
    
    for (int i = 0; i < n; i++) {
        p[i].pid = i + 1;
        printf("\nProcess %d\nArrival Time: ", i + 1);
        scanf("%d", &p[i].at);
        printf("Burst Time: ");
        scanf("%d", &p[i].bt);
        printf("Priority: ");
        scanf("%d", &p[i].priority);
    }
    
    printf("\n========================================");
    printf("\n          Priority Type");
    printf("\n========================================");
    printf("\n1. Lower number = Higher priority");
    printf("\n2. Higher number = Higher priority\n\nEnter choice: ");
    scanf("%d", &priorityFlag);
    
    int choice;
    do {
        printf("\n\n========================================");
        printf("\n          CPU Scheduling Menu");
        printf("\n========================================");
        printf("\n1. FCFS\n2. SJF (Non-Preemptive)\n3. SRTF (Preemptive SJF)");
        printf("\n4. Priority Scheduling (Non-Preemptive)\n5. Priority Scheduling (Preemptive)");
        printf("\n6. Round Robin\n7. Exit\n\nEnter your choice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: fcfs(); break;
            case 2: sjf(); break;
            case 3: srtf(); break;
            case 4: priorityScheduling(); break;
            case 5: preemptivePriority(); break;
            case 6: roundRobin(); break;
            case 7: printf("\nExiting...\n"); break;
            default: printf("\nInvalid Choice\n");
        }
    } while (choice != 7);
    
    return 0;
}
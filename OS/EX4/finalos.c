#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_PROCESSES 100

typedef struct {
    int id;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int priority;
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int start_time;
} Process;

typedef struct {
    int pid;
    int start_time;
    int end_time;
} GanttBlock;

// Function prototypes
void input_processes(Process proc[], int *n);
void reset_processes(Process proc[], Process temp[], int n);
void print_results(Process proc[], int n, GanttBlock gantt[], int gantt_size);
void print_gantt_chart(GanttBlock gantt[], int gantt_size);

// Scheduling Algorithms
void fcfs(Process proc[], int n);
void sjf_non_preemptive(Process proc[], int n);
void srtf_preemptive(Process proc[], int n);
void priority_non_preemptive(Process proc[], int n);
void priority_preemptive(Process proc[], int n);
void round_robin(Process proc[], int n, int quantum);

int main() {
    int n, choice, quantum;
    Process proc[MAX_PROCESSES], temp[MAX_PROCESSES];

    input_processes(proc, &n);

    while (1) {
        reset_processes(proc, temp, n);
        printf("\n=========================================");
        printf("\n        CPU SCHEDULING ALGORITHMS        ");
        printf("\n=========================================");
        printf("\n1. First Come First Serve (FCFS)");
        printf("\n2. Shortest Job First (SJF - Non Preemptive)");
        printf("\n3. Shortest Remaining Time First (SRTF - Preemptive)");
        printf("\n4. Priority (Non-Preemptive)");
        printf("\n5. Priority (Preemptive)");
        printf("\n6. Round Robin (RR)");
        printf("\n7. Change Input Data");
        printf("\n8. Exit");
        printf("\nEnter your choice: ");
        if (scanf("%d", &choice) != 1) break;

        switch (choice) {
            case 1:
                fcfs(temp, n);
                break;
            case 2:
                sjf_non_preemptive(temp, n);
                break;
            case 3:
                srtf_preemptive(temp, n);
                break;
            case 4:
                priority_non_preemptive(temp, n);
                break;
            case 5:
                priority_preemptive(temp, n);
                break;
            case 6:
                printf("Enter Time Quantum: ");
                scanf("%d", &quantum);
                round_robin(temp, n, quantum);
                break;
            case 7:
                input_processes(proc, &n);
                break;
            case 8:
                exit(0);
            default:
                printf("\nInvalid Choice!");
        }
    }
    return 0;
}

void input_processes(Process proc[], int *n) {
    char input[100];
    printf("\nEnter total number of processes: ");
    scanf("%d", n);
    getchar(); // Consume newline

    for (int i = 0; i < *n; i++) {
        proc[i].id = i + 1;
        printf("\n--- Process P%d ---", proc[i].id);

        printf("\nEnter Burst Time: ");
        scanf("%d", &proc[i].burst_time);

        printf("Enter Priority (lower number = higher priority): ");
        scanf("%d", &proc[i].priority);
        getchar(); // Consume newline

        printf("Enter Arrival Time (Press Enter to default to 0): ");
        fgets(input, sizeof(input), stdin);

        if (input[0] == '\n' || input[0] == '\r') {
            proc[i].arrival_time = 0;
        } else {
            proc[i].arrival_time = atoi(input);
        }

        proc[i].remaining_time = proc[i].burst_time;
        proc[i].start_time = -1;
    }
}

void reset_processes(Process proc[], Process temp[], int n) {
    for (int i = 0; i < n; i++) {
        temp[i] = proc[i];
        temp[i].remaining_time = temp[i].burst_time;
        temp[i].start_time = -1;
    }
}

void print_gantt_chart(GanttBlock gantt[], int gantt_size) {
    printf("\n\n--- GANTT CHART ---\n");

    // Top border
    for (int i = 0; i < gantt_size; i++) {
        int duration = gantt[i].end_time - gantt[i].start_time;
        int width = (duration < 3) ? 4 : duration * 2;
        printf("+");
        for (int j = 0; j < width; j++) printf("-");
    }
    printf("+\n|");

    // Process IDs
    for (int i = 0; i < gantt_size; i++) {
        int duration = gantt[i].end_time - gantt[i].start_time;
        int width = (duration < 3) ? 4 : duration * 2;
        if (gantt[i].pid == -1) {
            int pad = (width - 4) / 2;
            printf("%*sIDLE%*s|", pad + (width - 4) % 2, "", pad, "");
        } else {
            char pstr[10];
            sprintf(pstr, "P%d", gantt[i].pid);
            int len = strlen(pstr);
            int pad = (width - len) / 2;
            printf("%*s%s%*s|", pad + (width - len) % 2, "", pstr, pad, "");
        }
    }
    printf("\n");

    // Bottom border
    for (int i = 0; i < gantt_size; i++) {
        int duration = gantt[i].end_time - gantt[i].start_time;
        int width = (duration < 3) ? 4 : duration * 2;
        printf("+");
        for (int j = 0; j < width; j++) printf("-");
    }
    printf("+\n");

    // Time ticks
    printf("%d", gantt[0].start_time);
    for (int i = 0; i < gantt_size; i++) {
        int duration = gantt[i].end_time - gantt[i].start_time;
        int width = (duration < 3) ? 4 : duration * 2;
        printf("%*d", width + 1, gantt[i].end_time);
    }
    printf("\n");
}

void print_results(Process proc[], int n, GanttBlock gantt[], int gantt_size) {
    float total_wt = 0, total_tat = 0;

    print_gantt_chart(gantt, gantt_size);

    printf("\n==============================================================");
    printf("\nPID\tArrival\tBurst\tPriority\tCompletion\tTAT\tWT");
    printf("\n==============================================================");
    for (int i = 0; i < n; i++) {
        proc[i].turnaround_time = proc[i].completion_time - proc[i].arrival_time;
        proc[i].waiting_time = proc[i].turnaround_time - proc[i].burst_time;

        total_wt += proc[i].waiting_time;
        total_tat += proc[i].turnaround_time;

        printf("\nP%d\t%d\t%d\t%d\t\t%d\t\t%d\t%d",
               proc[i].id, proc[i].arrival_time, proc[i].burst_time,
               proc[i].priority, proc[i].completion_time,
               proc[i].turnaround_time, proc[i].waiting_time);
    }
    printf("\n==============================================================");
    printf("\nAverage Waiting Time     : %.2f", total_wt / n);
    printf("\nAverage Turnaround Time  : %.2f\n", total_tat / n);
}

// 1. First Come First Serve
void fcfs(Process proc[], int n) {
    // Sort by Arrival Time
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (proc[j].arrival_time > proc[j + 1].arrival_time) {
                Process temp = proc[j];
                proc[j] = proc[j + 1];
                proc[j + 1] = temp;
            }
        }
    }

    GanttBlock gantt[MAX_PROCESSES * 2];
    int g_size = 0, current_time = 0;

    for (int i = 0; i < n; i++) {
        if (current_time < proc[i].arrival_time) {
            gantt[g_size++] = (GanttBlock){-1, current_time, proc[i].arrival_time};
            current_time = proc[i].arrival_time;
        }
        gantt[g_size++] = (GanttBlock){proc[i].id, current_time, current_time + proc[i].burst_time};
        current_time += proc[i].burst_time;
        proc[i].completion_time = current_time;
    }

    print_results(proc, n, gantt, g_size);
}

// 2. Shortest Job First (Non-Preemptive)
void sjf_non_preemptive(Process proc[], int n) {
    GanttBlock gantt[MAX_PROCESSES * 2];
    int g_size = 0, current_time = 0, completed = 0;
    bool is_completed[MAX_PROCESSES] = {false};

    while (completed < n) {
        int idx = -1;
        int min_burst = 1e9;

        for (int i = 0; i < n; i++) {
            if (proc[i].arrival_time <= current_time && !is_completed[i]) {
                if (proc[i].burst_time < min_burst) {
                    min_burst = proc[i].burst_time;
                    idx = i;
                }
                if (proc[i].burst_time == min_burst) {
                    if (proc[i].arrival_time < proc[idx].arrival_time) {
                        idx = i;
                    }
                }
            }
        }

        if (idx != -1) {
            gantt[g_size++] = (GanttBlock){proc[idx].id, current_time, current_time + proc[idx].burst_time};
            current_time += proc[idx].burst_time;
            proc[idx].completion_time = current_time;
            is_completed[idx] = true;
            completed++;
        } else {
            int next_arrival = 1e9;
            for (int i = 0; i < n; i++) {
                if (!is_completed[i] && proc[i].arrival_time < next_arrival) {
                    next_arrival = proc[i].arrival_time;
                }
            }
            gantt[g_size++] = (GanttBlock){-1, current_time, next_arrival};
            current_time = next_arrival;
        }
    }

    print_results(proc, n, gantt, g_size);
}

// 3. Shortest Remaining Time First (Preemptive)
void srtf_preemptive(Process proc[], int n) {
    GanttBlock gantt[MAX_PROCESSES * 100];
    int g_size = 0, current_time = 0, completed = 0;
    int prev_pid = -2;

    while (completed < n) {
        int idx = -1;
        int min_remaining = 1e9;

        for (int i = 0; i < n; i++) {
            if (proc[i].arrival_time <= current_time && proc[i].remaining_time > 0) {
                if (proc[i].remaining_time < min_remaining) {
                    min_remaining = proc[i].remaining_time;
                    idx = i;
                }
            }
        }

        if (idx != -1) {
            if (prev_pid != proc[idx].id) {
                gantt[g_size++] = (GanttBlock){proc[idx].id, current_time, current_time + 1};
                prev_pid = proc[idx].id;
            } else {
                gantt[g_size - 1].end_time++;
            }

            proc[idx].remaining_time--;
            current_time++;

            if (proc[idx].remaining_time == 0) {
                proc[idx].completion_time = current_time;
                completed++;
            }
        } else {
            if (prev_pid != -1) {
                gantt[g_size++] = (GanttBlock){-1, current_time, current_time + 1};
                prev_pid = -1;
            } else {
                gantt[g_size - 1].end_time++;
            }
            current_time++;
        }
    }

    print_results(proc, n, gantt, g_size);
}

// 4. Priority Scheduling (Non-Preemptive)
void priority_non_preemptive(Process proc[], int n) {
    GanttBlock gantt[MAX_PROCESSES * 2];
    int g_size = 0, current_time = 0, completed = 0;
    bool is_completed[MAX_PROCESSES] = {false};

    while (completed < n) {
        int idx = -1;
        int highest_priority = 1e9;

        for (int i = 0; i < n; i++) {
            if (proc[i].arrival_time <= current_time && !is_completed[i]) {
                if (proc[i].priority < highest_priority) {
                    highest_priority = proc[i].priority;
                    idx = i;
                }
                if (proc[i].priority == highest_priority) {
                    if (proc[i].arrival_time < proc[idx].arrival_time) {
                        idx = i;
                    }
                }
            }
        }

        if (idx != -1) {
            gantt[g_size++] = (GanttBlock){proc[idx].id, current_time, current_time + proc[idx].burst_time};
            current_time += proc[idx].burst_time;
            proc[idx].completion_time = current_time;
            is_completed[idx] = true;
            completed++;
        } else {
            int next_arrival = 1e9;
            for (int i = 0; i < n; i++) {
                if (!is_completed[i] && proc[i].arrival_time < next_arrival) {
                    next_arrival = proc[i].arrival_time;
                }
            }
            gantt[g_size++] = (GanttBlock){-1, current_time, next_arrival};
            current_time = next_arrival;
        }
    }

    print_results(proc, n, gantt, g_size);
}

// 5. Priority Scheduling (Preemptive)
void priority_preemptive(Process proc[], int n) {
    GanttBlock gantt[MAX_PROCESSES * 100];
    int g_size = 0, current_time = 0, completed = 0;
    int prev_pid = -2;

    while (completed < n) {
        int idx = -1;
        int highest_priority = 1e9;

        for (int i = 0; i < n; i++) {
            if (proc[i].arrival_time <= current_time && proc[i].remaining_time > 0) {
                if (proc[i].priority < highest_priority) {
                    highest_priority = proc[i].priority;
                    idx = i;
                }
            }
        }

        if (idx != -1) {
            if (prev_pid != proc[idx].id) {
                gantt[g_size++] = (GanttBlock){proc[idx].id, current_time, current_time + 1};
                prev_pid = proc[idx].id;
            } else {
                gantt[g_size - 1].end_time++;
            }

            proc[idx].remaining_time--;
            current_time++;

            if (proc[idx].remaining_time == 0) {
                proc[idx].completion_time = current_time;
                completed++;
            }
        } else {
            if (prev_pid != -1) {
                gantt[g_size++] = (GanttBlock){-1, current_time, current_time + 1};
                prev_pid = -1;
            } else {
                gantt[g_size - 1].end_time++;
            }
            current_time++;
        }
    }

    print_results(proc, n, gantt, g_size);
}

// 6. Round Robin
void round_robin(Process proc[], int n, int quantum) {
    GanttBlock gantt[MAX_PROCESSES * 100];
    int g_size = 0, current_time = 0, completed = 0;

    // Queue implementation
    int queue[MAX_PROCESSES * 10];
    int front = 0, rear = 0;
    bool in_queue[MAX_PROCESSES] = {false};

    // Sort by Arrival Time initially
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (proc[j].arrival_time > proc[j + 1].arrival_time) {
                Process temp = proc[j];
                proc[j] = proc[j + 1];
                proc[j + 1] = temp;
            }
        }
    }

    // Push initial processes arriving at t=0 or earliest time
    for (int i = 0; i < n; i++) {
        if (proc[i].arrival_time <= current_time) {
            queue[rear++] = i;
            in_queue[i] = true;
        }
    }

    while (completed < n) {
        if (front == rear) { // Queue is empty
            int min_arrival = 1e9;
            for (int i = 0; i < n; i++) {
                if (proc[i].remaining_time > 0 && proc[i].arrival_time < min_arrival) {
                    min_arrival = proc[i].arrival_time;
                }
            }
            gantt[g_size++] = (GanttBlock){-1, current_time, min_arrival};
            current_time = min_arrival;

            for (int i = 0; i < n; i++) {
                if (proc[i].arrival_time <= current_time && proc[i].remaining_time > 0 && !in_queue[i]) {
                    queue[rear++] = i;
                    in_queue[i] = true;
                }
            }
            continue;
        }

        int idx = queue[front++];
        int execute_time = (proc[idx].remaining_time > quantum) ? quantum : proc[idx].remaining_time;

        gantt[g_size++] = (GanttBlock){proc[idx].id, current_time, current_time + execute_time};
        current_time += execute_time;
        proc[idx].remaining_time -= execute_time;

        // Add new processes arriving during execution
        for (int i = 0; i < n; i++) {
            if (proc[i].arrival_time <= current_time && proc[i].remaining_time > 0 && !in_queue[i]) {
                queue[rear++] = i;
                in_queue[i] = true;
            }
        }

        // Re-add current process if not finished
        if (proc[idx].remaining_time > 0) {
            queue[rear++] = idx;
        } else {
            proc[idx].completion_time = current_time;
            completed++;
        }
    }

    print_results(proc, n, gantt, g_size);
}

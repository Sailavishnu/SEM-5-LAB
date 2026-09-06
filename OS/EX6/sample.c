#include <stdio.h>
#define MAX_P 10
#define MAX_R 10

struct Bankers { int p, r; int alloc[MAX_P][MAX_R], maxm[MAX_P][MAX_R], need[MAX_P][MAX_R], avail[MAX_R]; };

void inputData(struct Bankers *b) {
    printf("Enter number of processes: "); scanf("%d", &b->p);
    printf("Enter number of resource types: "); scanf("%d", &b->r);
    printf("\nEnter Allocation Matrix:\n");
    for (int i = 0; i < b->p; i++) { printf("P%d: ", i); for (int j = 0; j < b->r; j++) scanf("%d", &b->alloc[i][j]); }
    printf("\nEnter Max Matrix:\n");
    for (int i = 0; i < b->p; i++) { printf("P%d: ", i); for (int j = 0; j < b->r; j++) scanf("%d", &b->maxm[i][j]); }
    printf("\nEnter Available Resources:\n");
    for (int j = 0; j < b->r; j++) scanf("%d", &b->avail[j]);
}

void calculateNeed(struct Bankers *b) {
    printf("\nNeed Matrix:\n");
    for (int i = 0; i < b->p; i++) {
        printf("P%d: ", i);
        for (int j = 0; j < b->r; j++) printf("%d ", b->need[i][j] = b->maxm[i][j] - b->alloc[i][j]);
        printf("\n");
    }
}

int checkSafety(struct Bankers *b, int seq[], int *count) {
    int work[MAX_R], finish[MAX_P] = {0};
    *count = 0;
    for (int j = 0; j < b->r; j++) work[j] = b->avail[j];
    printf("\nInitial Work = "); for (int j = 0; j < b->r; j++) printf("%d ", work[j]); printf("\n");

    while (*count < b->p) {
        int found = 0;
        for (int i = 0; i < b->p; i++) {
            if (finish[i]) continue;
            int ok = 1;
            for (int j = 0; j < b->r; j++) if (b->need[i][j] > work[j]) { ok = 0; break; }
            if (ok) {
                for (int j = 0; j < b->r; j++) work[j] += b->alloc[i][j];
                seq[(*count)++] = i; finish[i] = 1; found = 1;
                printf("P%d runs -> Work = ", i);
                for (int j = 0; j < b->r; j++) printf("%d ", work[j]);
                printf("\n");
            }
        }
        if (!found) break;
    }
    return (*count == b->p);
}

void printSafeSequence(int seq[], int count) {
    printf("Safe sequence: ");
    for (int i = 0; i < count; i++) printf("P%d%s", seq[i], i == count - 1 ? "\n" : " -> ");
}

void showInitialSafety(struct Bankers *b) {
    int seq[MAX_P], count;
    if (checkSafety(b, seq, &count)) { printf("\nSystem is in SAFE state.\n"); printSafeSequence(seq, count); }
    else printf("\nSystem is in UNSAFE state (deadlock possible).\n");
}

void requestResources(struct Bankers *b, int pid, int req[]) {
    printf("\n--- Request from P%d: ", pid);
    for (int j = 0; j < b->r; j++) printf("%d ", req[j]);
    printf("---\n");

    for (int j = 0; j < b->r; j++)
        if (req[j] > b->need[pid][j]) { printf("Request denied: exceeds P%d's declared Need. Invalid request.\n", pid); return; }

    for (int j = 0; j < b->r; j++)
        if (req[j] > b->avail[j]) { printf("Request denied: not enough resources available right now. P%d must wait.\n", pid); return; }

    for (int j = 0; j < b->r; j++) { b->avail[j] -= req[j]; b->alloc[pid][j] += req[j]; b->need[pid][j] -= req[j]; }

    int seq[MAX_P], count;
    if (checkSafety(b, seq, &count)) {
        printf("Request CAN be granted immediately. New state is safe.\n");
        printSafeSequence(seq, count);
    } else {
        printf("Request CANNOT be granted: resulting state is unsafe. P%d must wait.\n", pid);
        for (int j = 0; j < b->r; j++) { b->avail[j] += req[j]; b->alloc[pid][j] -= req[j]; b->need[pid][j] += req[j]; }
    }
}

void inputRequest(struct Bankers *b, int *pid, int req[]) {
    printf("\nEnter process number requesting resources (e.g. 2 for P2): "); scanf("%d", pid);
    printf("Enter requested amount for each of %d resource types: ", b->r);
    for (int j = 0; j < b->r; j++) scanf("%d", &req[j]);
}

int main() {
    struct Bankers b;
    int pid, req[MAX_R];

    inputData(&b);
    calculateNeed(&b);
    showInitialSafety(&b);
    inputRequest(&b, &pid, req);
    requestResources(&b, pid, req);

    return 0;
}
#include <stdio.h>
#define MAX_P 10
#define MAX_R 10

struct Bankers {
    int p, r;
    int alloc[MAX_P][MAX_R], maxm[MAX_P][MAX_R], need[MAX_P][MAX_R], avail[MAX_R];
};

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
        for (int j = 0; j < b->r; j++) {
            b->need[i][j] = b->maxm[i][j] - b->alloc[i][j];
            printf("%d ", b->need[i][j]);
        }
        printf("\n");
    }
}

/* returns 1 if safe (fills safeSeq/count), 0 if not safe.
   prints Work after every process that gets picked */
int checkSafety(struct Bankers *b, int safeSeq[], int *count) {
    int work[MAX_R], finish[MAX_P] = {0};
    *count = 0;
    for (int j = 0; j < b->r; j++) work[j] = b->avail[j];

    printf("\nInitial Work = ");
    for (int j = 0; j < b->r; j++) printf("%d ", work[j]);
    printf("\n");

    while (*count < b->p) {
        int found = 0;
        for (int i = 0; i < b->p; i++) {
            if (finish[i]) continue;
            int canRun = 1;
            for (int j = 0; j < b->r; j++) if (b->need[i][j] > work[j]) { canRun = 0; break; }
            if (canRun) {
                for (int j = 0; j < b->r; j++) work[j] += b->alloc[i][j];
                safeSeq[(*count)++] = i;
                finish[i] = 1;
                found = 1;

                printf("P%d runs -> Work = ", i);
                for (int j = 0; j < b->r; j++) printf("%d ", work[j]);
                printf("\n");
            }
        }
        if (!found) break;
    }

    return (*count == b->p);
}

void printSafeSequence(int safeSeq[], int count) {
    printf("Safe sequence: ");
    for (int i = 0; i < count; i++)
        printf("P%d%s", safeSeq[i], i == count - 1 ? "\n" : " -> ");
}

int main() {
    struct Bankers b;
    int safeSeq[MAX_P], count;

    inputData(&b);
    calculateNeed(&b);

    if (checkSafety(&b, safeSeq, &count)) {
        printf("\nSystem is in SAFE state.\n");
        printSafeSequence(safeSeq, count);
    } else {
        printf("\nSystem is in UNSAFE state (deadlock possible).\n");
    }

    return 0;
}
#include <stdio.h>
#define MAX 20

int hole[MAX], holeAlloc[MAX], proc[MAX], procBlock[MAX];
int n, m;

void reset() {
    for (int i = 0; i < n; i++) { holeAlloc[i] = -1; }
    for (int i = 0; i < m; i++) { procBlock[i] = -1; }
}

/* single sort function for both Best Fit and Worst Fit
   flag = 1 -> ascending (Best Fit), flag = 0 -> descending (Worst Fit) */
void sortHoles(int idx[], int flag) {
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - i - 1; j++) {
            int a = hole[idx[j]], b = hole[idx[j + 1]];
            if ((flag && a > b) || (!flag && a < b)) {
                int t = idx[j]; idx[j] = idx[j + 1]; idx[j + 1] = t;
            }
        }
}

void firstFit() {
    reset();
    for (int i = 0; i < m; i++)
        for (int j = 0; j < n; j++)
            if (holeAlloc[j] == -1 && hole[j] >= proc[i]) {
                holeAlloc[j] = i; procBlock[i] = j; break;
            }
}

void fitBySize(int flag) {   /* flag=1 Best Fit, flag=0 Worst Fit */
    reset();
    int idx[MAX];
    for (int i = 0; i < n; i++) idx[i] = i;
    sortHoles(idx, flag);
    for (int i = 0; i < m; i++)
        for (int k = 0; k < n; k++) {
            int j = idx[k];
            if (holeAlloc[j] == -1 && hole[j] >= proc[i]) {
                holeAlloc[j] = i; procBlock[i] = j; break;
            }
        }
}

void showResult(const char *name) {
    int intFrag = 0, extFrag = 0;
    printf("\n-- %s --\n", name);
    for (int i = 0; i < m; i++) {
        if (procBlock[i] == -1)
            printf("Process %d -> Not Allocated\n", i + 1);
        else {
            int h = procBlock[i];
            printf("Process %d -> H%d (frag=%d)\n", i + 1, h + 1, hole[h] - proc[i]);
            intFrag += hole[h] - proc[i];
        }
    }
    printf("Unused: ");
    int any = 0;
    for (int j = 0; j < n; j++)
        if (holeAlloc[j] == -1) { printf("H%d ", j + 1); extFrag += hole[j]; any = 1; }
    if (!any) printf("None");
    printf("\nInternal Frag = %d | External Frag = %d\n", intFrag, extFrag);
}

int main() {
    printf("Enter number of memory holes: ");
    scanf("%d", &n);
    for (int i = 0; i < n; i++) { printf("Size of H%d: ", i + 1); scanf("%d", &hole[i]); }

    printf("Enter number of processes: ");
    scanf("%d", &m);
    for (int i = 0; i < m; i++) { printf("Size of Process %d: ", i + 1); scanf("%d", &proc[i]); }

    int choice;
    do {
        printf("\n1.First Fit 2.Best Fit 3.Worst Fit 4.Exit\nChoice: ");
        scanf("%d", &choice);
        if (choice == 1) { firstFit(); showResult("First Fit"); }
        else if (choice == 2) { fitBySize(1); showResult("Best Fit"); }
        else if (choice == 3) { fitBySize(0); showResult("Worst Fit"); }
    } while (choice != 4);

    return 0;
}

















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

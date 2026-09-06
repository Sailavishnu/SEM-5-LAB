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
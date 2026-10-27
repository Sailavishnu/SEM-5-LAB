
#include <stdio.h>
#define MAX 100

int search(int f[], int nf, int p) {
    for (int i = 0; i < nf; i++)
        if (f[i] == p) return i;
    return -1;
}

void print(int f[], int nf) {
    for (int i = 0; i < nf; i++)
        if (f[i] == -1) printf(" - ");
        else printf("%2d ", f[i]);
}

void result(char *name, int faults, int np) {
    printf("\n%s: Page Faults = %d, Page Hits = %d\n",
           name, faults, np - faults);
    printf("Hit Ratio = %.2f, Fault Ratio = %.2f\n",
           (float)(np - faults) / np, (float)faults / np);
}

void fifo(int p[], int np, int nf) {
    int f[MAX], front = 0, faults = 0, i;
    for (i = 0; i < nf; i++) f[i] = -1;

    printf("\n--- FIFO Page Replacement ---\n");
    printf("Page | Frames | Status\n");

    for (i = 0; i < np; i++) {
        printf("%3d  | ", p[i]);
        int hit = search(f, nf, p[i]) != -1;

        if (!hit) {
            f[front] = p[i];
            front = (front + 1) % nf;
            faults++;
        }
        print(f, nf);
        printf("| %s\n", hit ? "Hit" : "Fault");
    }
    result("FIFO", faults, np);
}

void lru(int p[], int np, int nf) {
    int f[MAX], last[MAX], faults = 0, i, j;
    for (i = 0; i < nf; i++) f[i] = last[i] = -1;

    printf("\n--- LRU Page Replacement ---\n");
    printf("Page | Frames | Status\n");

    for (i = 0; i < np; i++) {
        printf("%3d  | ", p[i]);
        int pos = search(f, nf, p[i]);

        if (pos != -1) last[pos] = i;
        else {
            int v = 0;
            for (j = 0; j < nf; j++) {
                if (f[j] == -1) { v = j; break; }
                if (last[j] < last[v]) v = j;
            }
            f[v] = p[i];
            last[v] = i;
            faults++;
        }
        print(f, nf);
        printf("| %s\n", pos != -1 ? "Hit" : "Fault");
    }
    result("LRU", faults, np);
}

void optimal(int p[], int np, int nf) {
    int f[MAX], faults = 0, i, j, k;
    for (i = 0; i < nf; i++) f[i] = -1;

    printf("\n--- Optimal Page Replacement ---\n");
    printf("Page | Frames | Status\n");

    for (i = 0; i < np; i++) {
        printf("%3d  | ", p[i]);

        if (search(f, nf, p[i]) != -1) {
            print(f, nf);
            printf("| Hit\n");
            continue;
        }

        int v = -1;
        for (j = 0; j < nf; j++)
            if (f[j] == -1) { v = j; break; }

        if (v == -1) {
            int far = -1;
            for (j = 0; j < nf; j++) {
                int next = np;
                for (k = i + 1; k < np; k++)
                    if (p[k] == f[j]) { next = k; break; }
                if (next > far) { far = next; v = j; }
            }
        }

        f[v] = p[i];
        faults++;
        print(f, nf);
        printf("| Fault\n");
    }
    result("Optimal", faults, np);
}

int main() {
    int p[MAX], np, nf, ch, i;

    printf("Enter number of pages: ");
    scanf("%d", &np);
    printf("Enter the page reference string: ");
    for (i = 0; i < np; i++) scanf("%d", &p[i]);
    printf("Enter number of frames: ");
    scanf("%d", &nf);

    do {
        printf("\n===== MENU =====\n");
        printf("1. FIFO\n2. LRU\n3. Optimal\n4. Run all\n5. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &ch);

        switch (ch) {
            case 1: fifo(p, np, nf); break;
            case 2: lru(p, np, nf); break;
            case 3: optimal(p, np, nf); break;
            case 4:
                fifo(p, np, nf);
                lru(p, np, nf);
                optimal(p, np, nf);
                break;
            case 5: printf("Exiting...\n"); break;
            default: printf("Invalid choice!\n");
        }
    } while (ch != 5);

    return 0;
}

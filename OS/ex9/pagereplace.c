#include <stdio.h>

#define MAX 100

/* Returns index of page in frames, or -1 if not present */
int search(int frames[], int nf, int page) {
    for (int i = 0; i < nf; i++)
        if (frames[i] == page)
            return i;
    return -1;
}

void print_frames(int frames[], int nf) {
    for (int i = 0; i < nf; i++) {
        if (frames[i] == -1) printf(" - ");
        else printf("%2d ", frames[i]);
    }
}

void print_result(const char *name, int faults, int np) {
    printf("\n%s: Page Faults = %d, Page Hits = %d\n", name, faults, np - faults);
    printf("Hit Ratio = %.2f, Fault Ratio = %.2f\n",
           (float)(np - faults) / np, (float)faults / np);
}

/* ---------------- FIFO ---------------- */
void fifo(int pages[], int np, int nf) {
    int frames[MAX], front = 0, faults = 0;
    for (int i = 0; i < nf; i++) frames[i] = -1;

    printf("\n--- FIFO Page Replacement ---\n");
    printf("Page | Frames | Status\n");
    for (int i = 0; i < np; i++) {
        printf("%3d  | ", pages[i]);
        if (search(frames, nf, pages[i]) == -1) {
            frames[front] = pages[i];          /* replace oldest page */
            front = (front + 1) % nf;
            faults++;
            print_frames(frames, nf);
            printf("| Fault\n");
        } else {
            print_frames(frames, nf);
            printf("| Hit\n");
        }
    }
    print_result("FIFO", faults, np);
}

/* ---------------- LRU ---------------- */
void lru(int pages[], int np, int nf) {
    int frames[MAX], last_used[MAX], faults = 0;
    for (int i = 0; i < nf; i++) { frames[i] = -1; last_used[i] = -1; }

    printf("\n--- LRU Page Replacement ---\n");
    printf("Page | Frames | Status\n");
    for (int i = 0; i < np; i++) {
        printf("%3d  | ", pages[i]);
        int pos = search(frames, nf, pages[i]);
        if (pos != -1) {
            last_used[pos] = i;                /* update recency on hit */
            print_frames(frames, nf);
            printf("| Hit\n");
        } else {
            int victim = 0;
            for (int j = 0; j < nf; j++) {
                if (frames[j] == -1) { victim = j; break; }   /* free frame */
                if (last_used[j] < last_used[victim]) victim = j;
            }
            frames[victim] = pages[i];
            last_used[victim] = i;
            faults++;
            print_frames(frames, nf);
            printf("| Fault\n");
        }
    }
    print_result("LRU", faults, np);
}

/* ---------------- Optimal ---------------- */
void optimal(int pages[], int np, int nf) {
    int frames[MAX], faults = 0;
    for (int i = 0; i < nf; i++) frames[i] = -1;

    printf("\n--- Optimal Page Replacement ---\n");
    printf("Page | Frames | Status\n");
    for (int i = 0; i < np; i++) {
        printf("%3d  | ", pages[i]);
        if (search(frames, nf, pages[i]) != -1) {
            print_frames(frames, nf);
            printf("| Hit\n");
            continue;
        }

        int victim = -1;
        for (int j = 0; j < nf; j++)
            if (frames[j] == -1) { victim = j; break; }        /* free frame */

        if (victim == -1) {
            int farthest = -1;
            for (int j = 0; j < nf; j++) {
                int next = np;                                  /* np = never used again */
                for (int k = i + 1; k < np; k++)
                    if (pages[k] == frames[j]) { next = k; break; }
                if (next > farthest) { farthest = next; victim = j; }
            }
        }
        frames[victim] = pages[i];
        faults++;
        print_frames(frames, nf);
        printf("| Fault\n");
    }
    print_result("Optimal", faults, np);
}

int main(void) {
    int pages[MAX], np, nf, choice;

    printf("Enter number of pages: ");
    scanf("%d", &np);
    printf("Enter the page reference string: ");
    for (int i = 0; i < np; i++) scanf("%d", &pages[i]);
    printf("Enter number of frames: ");
    scanf("%d", &nf);

    do {
        printf("\n===== MENU =====\n");
        printf("1. FIFO\n2. LRU\n3. Optimal\n4. Run all\n5. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: fifo(pages, np, nf); break;
            case 2: lru(pages, np, nf); break;
            case 3: optimal(pages, np, nf); break;
            case 4: fifo(pages, np, nf); lru(pages, np, nf); optimal(pages, np, nf); break;
            case 5: printf("Exiting...\n"); break;
            default: printf("Invalid choice!\n");
        }
    } while (choice != 5);

    return 0;
}
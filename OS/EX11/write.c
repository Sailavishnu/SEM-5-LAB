/* Simplified Disk Scheduling: FCFS, SSTF, SCAN, CSCAN, LOOK, CLOOK */
#include <stdio.h>
#include <stdlib.h>

#define MAX 100

void display(int a[], int n, int head) {
    int i, total = 0;
    printf("Head movement: %d", head);
    for (i = 0; i < n; i++) {
        total += abs(head - a[i]);
        head = a[i];
        printf(" -> %d", head);
    }
    printf("\nTotal head movement = %d\n", total);
}

void sort(int a[], int n) {
    int i, j, t;
    for (i = 0; i < n; i++)
        for (j = i + 1; j < n; j++)
            if (a[i] > a[j]) {
                t = a[i]; a[i] = a[j]; a[j] = t;
            }
}

int main() {
    int req[MAX], n, head, i, choice;

    printf("Enter number of requests: ");
    scanf("%d", &n);
    printf("Enter request queue: ");
    for (i = 0; i < n; i++) scanf("%d", &req[i]);
    printf("Enter initial head position: ");
    scanf("%d", &head);

    do {
        printf("\n1.FCFS  2.SSTF  3.SCAN  4.CSCAN  5.LOOK  6.CLOOK  7.Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        if (choice == 1) {
            printf("FCFS\n");
            display(req, n, head);
        } else if (choice == 2) {
            int done[MAX] = {0}, seq[MAX], count = 0, j, min, pos;
            int h = head;
            for (i = 0; i < n; i++) {
                min = 99999; pos = -1;
                for (j = 0; j < n; j++)
                    if (!done[j] && abs(h - req[j]) < min) {
                        min = abs(h - req[j]); pos = j;
                    }
                done[pos] = 1;
                seq[count++] = req[pos];
                h = req[pos];
            }
            printf("SSTF\n");
            display(seq, count, head);
        } else if (choice >= 3 && choice <= 6) {
            int a[MAX], seq[MAX], count = 0;
            for (i = 0; i < n; i++) a[i] = req[i];
            sort(a, n);

            /* Simple right-moving versions of SCAN, CSCAN, LOOK and CLOOK */
            for (i = 0; i < n; i++)
                if (a[i] >= head) seq[count++] = a[i];

            if (choice == 3 || choice == 4) seq[count++] = 199;
            if (choice == 4) seq[count++] = 0;

            for (i = 0; i < n; i++)
                if (a[i] < head) seq[count++] = a[i];

            if (choice == 3) printf("SCAN\n");
            if (choice == 4) printf("CSCAN\n");
            if (choice == 5) printf("LOOK\n");
            if (choice == 6) printf("CLOOK\n");
            display(seq, count, head);
        } else if (choice == 7) {
            printf("Exiting...\n");
        } else {
            printf("Invalid choice\n");
        }
    } while (choice != 7);

    return 0;
}

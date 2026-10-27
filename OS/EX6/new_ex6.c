
#include <stdio.h>
#define P 10
#define R 10

struct Bankers {
    int p, r, alloc[P][R], max[P][R], need[P][R], avail[R];
};

int safety(struct Bankers *b, int seq[]) {
    int work[R], finish[P] = {0}, count = 0;

    for (int j = 0; j < b->r; j++)
        work[j] = b->avail[j];

    while (count < b->p) {
        int found = 0;

        for (int i = 0; i < b->p; i++) {
            if (finish[i]) continue;

            int j;
            for (j = 0; j < b->r; j++)
                if (b->need[i][j] > work[j]) break;

            if (j == b->r) {
                for (j = 0; j < b->r; j++)
                    work[j] += b->alloc[i][j];

                seq[count++] = i;
                finish[i] = found = 1;
            }
        }

        if (!found) return 0;
    }
    return 1;
}

void request(struct Bankers *b) {
    int pid, req[R], seq[P];

    printf("Enter process number: ");
    scanf("%d", &pid);

    if (pid < 0 || pid >= b->p) {
        printf("Invalid process number\n");
        return;
    }

    printf("Enter request: ");
    for (int j = 0; j < b->r; j++)
        scanf("%d", &req[j]);

    for (int j = 0; j < b->r; j++)
        if (req[j] > b->need[pid][j]) {
            printf("Request exceeds Need\n");
            return;
        }

    for (int j = 0; j < b->r; j++)
        if (req[j] > b->avail[j]) {
            printf("Resources unavailable; process must wait\n");
            return;
        }

    for (int j = 0; j < b->r; j++) {
        b->avail[j] -= req[j];
        b->alloc[pid][j] += req[j];
        b->need[pid][j] -= req[j];
    }

    if (safety(b, seq)) {
        printf("Request granted. Safe sequence: ");
        for (int i = 0; i < b->p; i++)
            printf("P%d%s", seq[i], i == b->p - 1 ? "\n" : " -> ");
    } else {
        for (int j = 0; j < b->r; j++) {
            b->avail[j] += req[j];
            b->alloc[pid][j] -= req[j];
            b->need[pid][j] += req[j];
        }
        printf("Request denied. Unsafe state; rollback done.\n");
    }
}

int main() {
    struct Bankers b;
    int seq[P], i, j;
    char ch;

    printf("Enter processes and resources: ");
    scanf("%d%d", &b.p, &b.r);

    printf("Enter Allocation Matrix:\n");
    for (i = 0; i < b.p; i++)
        for (j = 0; j < b.r; j++)
            scanf("%d", &b.alloc[i][j]);

    printf("Enter Max Matrix:\n");
    for (i = 0; i < b.p; i++)
        for (j = 0; j < b.r; j++) {
            scanf("%d", &b.max[i][j]);
            b.need[i][j] = b.max[i][j] - b.alloc[i][j];
        }

    printf("Enter Available Resources:\n");
    for (j = 0; j < b.r; j++)
        scanf("%d", &b.avail[j]);

    printf("Need Matrix:\n");
    for (i = 0; i < b.p; i++) {
        for (j = 0; j < b.r; j++)
            printf("%d ", b.need[i][j]);
        printf("\n");
    }

    if (safety(&b, seq)) {
        printf("System is SAFE\nSequence: ");
        for (i = 0; i < b.p; i++)
            printf("P%d%s", seq[i], i == b.p - 1 ? "\n" : " -> ");
    } else
        printf("System is UNSAFE\n");

    do {
        request(&b);
        printf("Another request? (y/n): ");
        scanf(" %c", &ch);
    } while (ch == 'y' || ch == 'Y');

    return 0;
}

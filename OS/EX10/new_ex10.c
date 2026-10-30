
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX 50

int main()
{
    int n, size, blocks[MAX], freeBlock[MAX];
    char name[MAX][20];

    srand(time(NULL));

    printf("Enter block size: ");
    scanf("%d", &size);

    printf("Enter number of files: ");
    scanf("%d", &n);

    for (int i = 0; i < n; i++) {
        printf("Enter file name and size: ");
        scanf("%19s %d", name[i], &blocks[i]);

        blocks[i] = (blocks[i] + size - 1) / size;
    }

    // 1. CONTIGUOUS ALLOCATION
    printf("\n--- CONTIGUOUS ALLOCATION ---\n");

    for (int i = 0; i < MAX; i++)
        freeBlock[i] = 1;

    for (int i = 0; i < n; i++) {
        int start = -1, count = 0;

        for (int j = 0; j < MAX; j++) {
            if (freeBlock[j]) {
                if (count == 0)
                    start = j;
                count++;
            } else {
                count = 0;
            }

            if (count == blocks[i])
                break;
        }

        if (count == blocks[i]) {
            printf("%s: ", name[i]);

            for (int j = start; j < start + blocks[i]; j++) {
                printf("%d ", j);
                freeBlock[j] = 0;
            }
            printf("\n");
        } else {
            printf("%s: Not allocated\n", name[i]);
        }
    }

    // 2. LINKED ALLOCATION
    printf("\n--- LINKED ALLOCATION ---\n");

    for (int i = 0; i < MAX; i++)
        freeBlock[i] = 1;

    for (int i = 0; i < n; i++) {
        printf("%s: ", name[i]);

        int allocated = 0;

        for (int j = 0; j < blocks[i]; j++) {
            int b;

            do {
                b = rand() % MAX;
            } while (freeBlock[b] == 0);

            freeBlock[b] = 0;
            printf("%d -> ", b);
            allocated++;
        }

        printf("END\n");
    }

    // 3. INDEXED ALLOCATION
    printf("\n--- INDEXED ALLOCATION ---\n");

    for (int i = 0; i < MAX; i++)
        freeBlock[i] = 1;

    for (int i = 0; i < n; i++) {
        int index = -1, count = 0;

        for (int j = 0; j < MAX; j++)
            count += freeBlock[j];

        if (count < blocks[i] + 1) {
            printf("%s: Not allocated\n", name[i]);
            continue;
        }

        do {
            index = rand() % MAX;
        } while (!freeBlock[index]);

        freeBlock[index] = 0;

        printf("%s: Index block = %d, Data blocks = ",
               name[i], index);

        for (int j = 0; j < blocks[i]; j++) {
            int b;

            do {
                b = rand() % MAX;
            } while (!freeBlock[b]);

            freeBlock[b] = 0;
            printf("%d ", b);
        }

        printf("\n");
    }

    return 0;
}

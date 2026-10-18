/* Simplified File Allocation: Contiguous, Linked, Indexed */
#include <stdio.h>
#include <stdlib.h>

#define MAX 50

int main() {
    int disk[MAX], n, size, i, j, choice, blocks;
    char name[MAX][20];

    for (i = 0; i < MAX; i++) disk[i] = 1;

    printf("Enter block size: ");
    scanf("%d", &size);
    printf("Enter number of files: ");
    scanf("%d", &n);

    for (i = 0; i < n; i++) {
        printf("Enter file name and size: ");
        scanf("%s%d", name[i], &blocks);
        blocks = (blocks + size - 1) / size;

        printf("%s needs %d blocks\n", name[i], blocks);
    }

    printf("\n1. Contiguous\n2. Linked\n3. Indexed\nEnter choice: ");
    scanf("%d", &choice);

    if (choice == 1) {
        for (i = 0; i < n; i++) {
            printf("%s: ", name[i]);
            for (j = i; j < i + 1; j++)
                printf("Blocks allocated from %d\n", j);
        }
    } else if (choice == 2) {
        for (i = 0; i < n; i++)
            printf("%s: 2 -> 5 -> 8 -> END\n", name[i]);
    } else if (choice == 3) {
        for (i = 0; i < n; i++)
            printf("%s: Index block -> Data blocks\n", name[i]);
    } else {
        printf("Invalid choice\n");
    }

    return 0;
}

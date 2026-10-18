#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX 50

// File information
typedef struct {
    char file_name[20];
    int file_size;    // Size in bytes (user input)
    int length;       // Number of blocks needed (calculated)
} FileMeta;

// --- LINKED ALLOCATION STRUCTURE ---
typedef struct Node {
    int block_no;
    struct Node* next;
} Node;

int BLOCK_SIZE = 1024; // Will be set by user

/* Count free blocks */
int countFreeBlocks(int freeBlock[]) {
    int count = 0;
    for (int i = 0; i < MAX; i++) {
        if (freeBlock[i] == 1) count++;
    }
    return count;
}

/* Pick a random free block and mark it used */
int getRandomFreeBlock(int freeBlock[]) {
    int count = 0;
    int available[MAX];
    for (int i = 0; i < MAX; i++) {
        if (freeBlock[i] == 1) {
            available[count++] = i;
        }
    }
    if (count == 0) return -1;
    int block = available[rand() % count];
    freeBlock[block] = 0;
    return block;
}

/* Find a contiguous range of free blocks (First-Fit) */
int findContiguousFreeBlocks(int freeBlock[], int length) {
    int count = 0;
    int start_candidate = -1;
    for (int i = 0; i < MAX; i++) {
        if (freeBlock[i] == 1) {
            if (count == 0) start_candidate = i;
            count++;
            if (count == length) return start_candidate;
        } else {
            count = 0;
            start_candidate = -1;
        }
    }
    return -1;
}

void resetFreeBlocks(int freeBlock[]) {
    for (int i = 0; i < MAX; i++) freeBlock[i] = 1;
}

void displayFreeBlocks(int freeBlock[]) {
    printf("\nFree blocks: ");
    for (int i = 0; i < MAX; i++) {
        if (freeBlock[i] == 1) printf("%d ", i);
    }
    printf("\n");
}

/* 1. Contiguous Allocation */
void contiguous(FileMeta f[], int n, int freeBlock[]) {
    printf("\nContiguous Allocation (block size = %d bytes)\n", BLOCK_SIZE);

    for (int i = 0; i < n; i++) {
        printf("\nFile %s (%d bytes, %d blocks)\n",
               f[i].file_name, f[i].file_size, f[i].length);

        if (f[i].length == 0) {
            printf("  No blocks needed\n");
            continue;
        }

        int start = findContiguousFreeBlocks(freeBlock, f[i].length);
        if (start == -1) {
            printf("  Not allocated (no contiguous space)\n");
            continue;
        }

        printf("  Blocks: ");
        for (int j = 0; j < f[i].length; j++) {
            freeBlock[start + j] = 0;
            printf("%d ", start + j);
        }
        printf("\n");
    }
}

/* 2. Linked Allocation */
void linked(FileMeta f[], int n, int freeBlock[]) {
    printf("\nLinked Allocation (block size = %d bytes)\n", BLOCK_SIZE);

    for (int i = 0; i < n; i++) {
        printf("\nFile %s (%d bytes, %d blocks)\n",
               f[i].file_name, f[i].file_size, f[i].length);

        if (f[i].length == 0) {
            printf("  No blocks needed\n");
            continue;
        }

        Node* head = NULL;
        Node* tail = NULL;
        int allocated = 0;

        for (int j = 0; j < f[i].length; j++) {
            int block = getRandomFreeBlock(freeBlock);
            if (block == -1) break;

            Node* newNode = (Node*)malloc(sizeof(Node));
            newNode->block_no = block;
            newNode->next = NULL;
            if (head == NULL) head = newNode;
            else tail->next = newNode;
            tail = newNode;
            allocated++;
        }

        printf("  Chain: ");
        Node* temp = head;
        while (temp != NULL) {
            printf("%d -> ", temp->block_no);
            Node* nextNode = temp->next;
            free(temp);
            temp = nextNode;
        }
        printf("END\n");

        if (allocated < f[i].length) {
            printf("  Disk full: only %d of %d blocks allocated\n",
                   allocated, f[i].length);
        }
    }
}

/* 3. Indexed Allocation (single-level: one index block per file) */
void indexed(FileMeta f[], int n, int freeBlock[]) {
    printf("\nIndexed Allocation (block size = %d bytes)\n", BLOCK_SIZE);

    for (int i = 0; i < n; i++) {
        printf("\nFile %s (%d bytes, %d blocks)\n",
               f[i].file_name, f[i].file_size, f[i].length);

        if (f[i].length == 0) {
            printf("  No blocks needed\n");
            continue;
        }

        // Need 1 index block + all data blocks
        if (countFreeBlocks(freeBlock) < f[i].length + 1) {
            printf("  Not allocated (not enough free blocks)\n");
            continue;
        }

        int indexBlock = getRandomFreeBlock(freeBlock);
        printf("  Index block: %d\n", indexBlock);
        printf("  Data blocks: ");
        for (int j = 0; j < f[i].length; j++) {
            printf("%d ", getRandomFreeBlock(freeBlock));
        }
        printf("\n");
    }
}

int main() {
    FileMeta f[MAX];
    int freeBlock[MAX];
    int n, choice;
    srand(time(NULL));
    resetFreeBlocks(freeBlock);

    printf("===== FILE ALLOCATION TECHNIQUES =====\n");
    printf("\nEnter disk block size (bytes): ");
    scanf("%d", &BLOCK_SIZE);
    if (BLOCK_SIZE <= 0) BLOCK_SIZE = 1024;

    printf("Enter number of files: ");
    scanf("%d", &n);
    if (n < 0 || n > MAX) {
        printf("Invalid number of files (max %d).\n", MAX);
        return 1;
    }

    printf("Enter file name and size in bytes for each file:\n");
    for (int i = 0; i < n; i++) {
        printf("File %d: ", i + 1);
        scanf("%19s %d", f[i].file_name, &f[i].file_size);

        if (f[i].file_size <= 0) {
            f[i].length = 0;
        } else {
            f[i].length = (f[i].file_size + BLOCK_SIZE - 1) / BLOCK_SIZE; // Ceiling division
        }
    }

    do {
        printf("\n===== MENU =====\n");
        printf("1. Contiguous Allocation\n");
        printf("2. Linked Allocation\n");
        printf("3. Indexed Allocation\n");
        printf("4. Display Free Blocks\n");
        printf("5. Reset Free Blocks\n");
        printf("6. Exit\n");
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("\nInvalid input. Exiting.\n");
            break;
        }

        switch (choice) {
            case 1:
                resetFreeBlocks(freeBlock);
                contiguous(f, n, freeBlock);
                break;
            case 2:
                resetFreeBlocks(freeBlock);
                linked(f, n, freeBlock);
                break;
            case 3:
                resetFreeBlocks(freeBlock);
                indexed(f, n, freeBlock);
                break;
            case 4:
                displayFreeBlocks(freeBlock);
                break;
            case 5:
                resetFreeBlocks(freeBlock);
                printf("\nFree blocks reset.\n");
                break;
            case 6:
                printf("\nProgram ended.\n");
                break;
            default:
                printf("\nInvalid choice!\n");
        }
    } while (choice != 6);

    return 0;
}

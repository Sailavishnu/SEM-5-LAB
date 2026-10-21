
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define MAX 50

int freeB[MAX], size = 1024;

typedef struct Node {
    int block;
    struct Node *next;
} Node;

void reset() {
    for (int i = 0; i < MAX; i++) freeB[i] = 1;
}

int getBlock() {
    int a[MAX], n = 0;
    for (int i = 0; i < MAX; i++)
        if (freeB[i]) a[n++] = i;
    if (!n) return -1;
    int b = a[rand() % n];
    freeB[b] = 0;
    return b;
}

void contiguous(char name[], int len) {
    int i, j, count;
    for (i = 0; i <= MAX - len; i++) {
        for (j = 0; j < len && freeB[i+j]; j++);
        if (j == len) {
            printf("%s: ", name);
            for (j = 0; j < len; j++) {
                freeB[i+j] = 0;
                printf("%d ", i+j);
            }
            printf("\n");
            return;
        }
    }
    printf("%s: Not allocated\n", name);
}

void linked(char name[], int len) {
    Node *head = NULL, *tail = NULL, *p;
    int b, i;
    for (i = 0; i < len; i++) {
        b = getBlock();
        if (b == -1) break;
        p = malloc(sizeof(Node));
        p->block = b;
        p->next = NULL;
        if (!head) head = p;
        else tail->next = p;
        tail = p;
    }
    printf("%s: ", name);
    while (head) {
        printf("%d -> ", head->block);
        p = head;
        head = head->next;
        free(p);
    }
    printf("END\n");
}

void indexed(char name[], int len) {
    int i, b;
    if (len + 1 > MAX) {
        printf("%s: Not allocated\n", name);
        return;
    }
    for (i = 0; i < len + 1; i++)
        if (getBlock() == -1) {
            printf("%s: Not allocated\n", name);
            return;
        }
    printf("%s: Index block and %d data blocks allocated\n",
           name, len);
}

int main() {
    char name[20];
    int bytes, len, choice;
    srand(time(NULL));

    printf("Block size: ");
    scanf("%d", &size);
    if (size <= 0) size = 1024;

    printf("File name and size: ");
    scanf("%19s%d", name, &bytes);
    len = (bytes + size - 1) / size;

    reset();
    printf("1. Contiguous  2. Linked  3. Indexed\nChoice: ");
    scanf("%d", &choice);

    if (choice == 1) contiguous(name, len);
    else if (choice == 2) linked(name, len);
    else if (choice == 3) indexed(name, len);
    else printf("Invalid choice\n");

    return 0;
}

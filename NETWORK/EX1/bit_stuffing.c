#include <stdio.h>
#define MAX 100

int data[MAX], stuffed[MAX], destuffed[MAX];
int flag[] = {0,1,1,1,1,1,1,0};
int flagLen = 8;

void printBits(char *label, int arr[], int n) {
    printf("%s", label);
    for (int i = 0; i < n; i++)
        printf("%d", arr[i]);
    printf("\n");
}

int main() {
    int n, ones = 0, j = 0;

    printf("Enter the number of bits: ");
    scanf("%d", &n);

    printf("Enter Bits (0s and 1s): ");
    for (int i = 0; i < n; i++)
        scanf("%d", &data[i]);

    printBits("\nOriginal : ", data, n);

    // ---- Bit stuffing ----
    for (int i = 0; i < n; i++) {
        stuffed[j++] = data[i];
        ones = (data[i] == 1) ? ones + 1 : 0;

        if (ones == 5) {
            stuffed[j++] = 0;
            ones = 0;
        }
    }
    int stuffedLen = j;
    printBits("Stuffed  : ", stuffed, stuffedLen);

    // ---- Framing ----
    int framed[MAX], k = 0;
    for (int i = 0; i < flagLen; i++)
        framed[k++] = flag[i];
    for (int i = 0; i < stuffedLen; i++)
        framed[k++] = stuffed[i];
    for (int i = 0; i < flagLen; i++)
        framed[k++] = flag[i];

    int framedLen = k;
    printBits("Framed   : ", framed, framedLen);

    // ---- Destuffing (skip flags, remove stuffed 0s after five 1s) ----
    ones = 0;
    j = 0;
    for (int i = flagLen; i < framedLen - flagLen; i++) {
        if (ones == 5) {
            if (framed[i] != 0) {
                printf("\nError: Invalid stuffing\n");
                return 0;
            }
            ones = 0;
            continue; // skip the stuffed 0
        }
        destuffed[j++] = framed[i];
        ones = (framed[i] == 1) ? ones + 1 : 0;
    }

    printBits("Destuffed: ", destuffed, j);

    // ---- Verify ----
    int match = 1;
    if (j != n) match = 0;
    else {
        for (int i = 0; i < n; i++)
            if (destuffed[i] != data[i]) { match = 0; break; }
    }
    printf("%s\n", match ? "Destuffing successful" : "Mismatch error in destuffing");

    return 0;
}

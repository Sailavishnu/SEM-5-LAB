#include <stdio.h>
#include "hamming.h"

int  code[MAX];
int  filled[MAX];
char label[MAX][4];

int isPowerOf2(int pos) {
    return pos > 0 && (pos & (pos - 1)) == 0;
}

/* names every position: powers of 2 -> P1, P2, P3 ... ; the rest -> D(n) ... D1 */
void assignLabels(int totalLen, int p) {
    int dNum = totalLen - p;
    for (int j = totalLen; j >= 1; j--) {
        if (isPowerOf2(j)) {
            int k = 1;
            while ((1 << k) <= j)          /* k = log2(j) + 1 */
                k++;
            sprintf(label[j], "P%d", k);
        } else
            sprintf(label[j], "D%d", dNum--);
    }
}

void displayFrame(int totalLen) {
    printf("\nPosition :\n");
    for (int j = totalLen; j >= 1; j--)
        printf("%3d", j);
    printf("\n\nType :\n");
    for (int j = totalLen; j >= 1; j--)
        printf("%3s", label[j]);
    printf("\n\nValue :\n");
    for (int j = totalLen; j >= 1; j--) {
        if (filled[j])
            printf("%3d", code[j]);
        else
            printf("  _");
    }
    printf("\n");
}

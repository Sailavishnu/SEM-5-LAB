#include <stdio.h>
#include <string.h>
#include "hamming.h"

/* smallest p such that n + p + 1 <= 2^p */
static int findParityBits(int n) {
    int p = 1;
    printf("\nStep 2:\nFinding parity bits (p)\n");
    for (; n + p + 1 > (1 << p); p++)
        printf("\nTry p = %d\n%d + %d + 1 <= 2^%d  -> No\n", p, n, p, p);
    printf("\nTry p = %d\n%d + %d + 1 <= 2^%d  -> Yes\n", p, n, p, p);
    printf("\nRequired parity bits = %d\n", p);
    return p;
}

/* first typed bit goes to the highest data slot, parity slots stay empty */
static void placeDataBits(const char *data, int totalLen) {
    int idx = 0;
    for (int j = totalLen; j >= 1; j--) {
        filled[j] = !isPowerOf2(j);
        code[j] = filled[j] ? data[idx++] - '0' : 0;
    }
}

/* Pk covers every position that has bit k set in its binary form */
static void calculateParity(int totalLen, int p, int parityType) {
    printf("\nStep 4:\nCalculating parity bits (%s parity)\n", parityType == 0 ? "Even" : "Odd");
    for (int k = 1; k <= p; k++) {
        int pos = 1 << (k - 1), count = 0;
        if (pos > totalLen)
            continue;

        printf("\nCalculate P%d\n\nChecking positions\n", k);
        for (int j = 1; j <= totalLen; j++)
            if (j & pos)
                printf("%d ", j);

        printf("\n\nValues\n");
        for (int j = 1; j <= totalLen; j++) {
            if (!(j & pos))
                continue;
            if (j == pos)
                printf("_ ");
            else {
                printf("%d ", code[j]);
                count += code[j];
            }
        }
        code[pos] = (count % 2) ^ (parityType != 0);   /* even: count%2, odd: opposite */
        filled[pos] = 1;
        printf("\n\nP%d = %d\n", k, code[pos]);
    }
}

void generateHammingCode(const char *data, int parityType) {
    int n = strlen(data), p, totalLen;

    printf("\nStep 1:\nNumber of data bits (n) = %d\n", n);
    p = findParityBits(n);
    totalLen = n + p;
    printf("Total bits = %d\n", totalLen);

    assignLabels(totalLen, p);
    placeDataBits(data, totalLen);
    printf("\nStep 3:\nInsert empty parity locations\n");
    displayFrame(totalLen);

    calculateParity(totalLen, p, parityType);
    printf("\nFinal Hamming Code\n");
    displayFrame(totalLen);

    printf("\n\nData to be transmitted: ");
    for (int j = totalLen; j >= 1; j--)
        printf("%d", code[j]);
    printf("\n");
}

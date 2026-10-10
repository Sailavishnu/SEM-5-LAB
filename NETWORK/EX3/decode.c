#include <stdio.h>
#include <string.h>
#include "hamming.h"

/* recompute every parity check; the failed checks form the error position (syndrome) */
static int detectError(int totalLen, int p, int parityType) {
    int syndrome = 0;
    printf("\nChecking received code:\n");
    for (int k = p; k >= 1; k--) {
        int pos = 1 << (k - 1), count = 0, bit;
        if (pos > totalLen)
            continue;
        for (int j = 1; j <= totalLen; j++)
            if (j & pos)
                count += code[j];              /* parity bit itself is included */
        bit = (count % 2) ^ (parityType != 0);
        printf("Checking P%d ... = %d\n", k, bit);
        syndrome += bit * pos;
    }
    printf("\nSyndrome (binary value of check bits) = %d\n", syndrome);
    return syndrome;
}

static void correctError(int errorPos, int totalLen) {
    if (errorPos == 0)
        printf("\nNo error detected.\n");
    else if (errorPos > totalLen)
        printf("\nError position out of range - more than 1 bit may be corrupted.\n");
    else {
        printf("\nError found at Position %d -> flipping bit to correct.\n", errorPos);
        code[errorPos] = !code[errorPos];
    }
    printf("\nCorrected Code\n");
    displayFrame(totalLen);
    printf("\n");
}

static void extractData(int totalLen) {
    printf("\nOriginal Data: ");
    for (int j = totalLen; j >= 1; j--)
        if (!isPowerOf2(j))
            printf("%d", code[j]);
    printf("\n");
}

void checkReceivedCode(const char *input, int parityType) {
    int totalLen = strlen(input), p = 0;
    while ((1 << p) < totalLen + 1)
        p++;

    assignLabels(totalLen, p);
    for (int j = totalLen, idx = 0; j >= 1; j--, idx++) {
        code[j] = input[idx] - '0';
        filled[j] = 1;
    }

    printf("\nReceived Code\n");
    displayFrame(totalLen);
    correctError(detectError(totalLen, p, parityType), totalLen);
    extractData(totalLen);
}

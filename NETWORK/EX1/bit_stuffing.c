#include <stdio.h>
#include <string.h>
#include "../convert/convert.h"
#include "ex1.h"

#define MAX_STR  100
#define MAX_BITS 1000
#define FLAG_LEN 8

static const int flag[FLAG_LEN] = {0, 1, 1, 1, 1, 1, 1, 0};
static int data[MAX_BITS], stuffed[MAX_BITS], destuffed[MAX_BITS];

static void printBits(const char *label, const int *arr, int n) {
    printf("%s", label);
    for (int i = 0; i < n; i++)
        printf("%d%s", arr[i], ((i + 1) % 8 == 0 && i != n - 1) ? " " : "");
    printf("\n");
}

/* Receiver: check both flags, remove stuffed 0s, compare with original data.
   Returns 1 if any error is found. */
static int receive(const int *framed, int framedLen, int n, int *destuffedLen) {
    int error = 0, ones = 0, j = 0;

    for (int i = 0; i < FLAG_LEN; i++)
        if (framed[i] != flag[i] || framed[framedLen - FLAG_LEN + i] != flag[i])
            error = 1;

    for (int i = FLAG_LEN; !error && i < framedLen - FLAG_LEN; i++) {
        if (ones == 5) {                 /* the bit after five 1s must be a stuffed 0 */
            error = framed[i] != 0;
            ones = 0;
            continue;
        }
        destuffed[j++] = framed[i];
        ones = framed[i] ? ones + 1 : 0;
    }
    *destuffedLen = j;
    return error || j != n || memcmp(destuffed, data, n * sizeof(int)) != 0;
}

void bitStuffing(void) {
    char inputString[MAX_STR], bin_str[9], outputString[MAX_STR];
    int ascii_arr[MAX_STR], out_ascii_arr[MAX_STR], framed[MAX_BITS], backup[MAX_BITS];
    int str_len, n = 0, ones = 0, stuffedLen = 0, framedLen, destuffedLen;
    int wantToFlip = 0, flipIndex = 0, chars = 0;

    printf("\n--- BIT STUFFING CONFIGURATION ---\n");
    printf("Enter the data input string: ");
    scanf("%99s", inputString);

    /* string -> ASCII -> stream of bits */
    str_to_ascii(inputString, ascii_arr, &str_len);
    for (int i = 0; i < str_len; i++) {
        ascii_to_bin((char)ascii_arr[i], bin_str);
        for (int b = 0; b < 8; b++)
            data[n++] = bin_str[b] == '1';
    }

    printf("\n--- TRANSMITTER SIDE ---\n");
    printf("Input String : %s\n", inputString);
    printBits("Original Bin : ", data, n);

    /* stuff a 0 after every five consecutive 1s */
    for (int i = 0; i < n; i++) {
        stuffed[stuffedLen++] = data[i];
        ones = data[i] ? ones + 1 : 0;
        if (ones == 5) {
            stuffed[stuffedLen++] = 0;
            ones = 0;
        }
    }
    printBits("Stuffed Bin  : ", stuffed, stuffedLen);

    /* frame = FLAG + stuffed data + FLAG */
    memcpy(framed, flag, sizeof flag);
    memcpy(framed + FLAG_LEN, stuffed, stuffedLen * sizeof(int));
    memcpy(framed + FLAG_LEN + stuffedLen, flag, sizeof flag);
    framedLen = stuffedLen + 2 * FLAG_LEN;
    printBits("Framed Bin   : ", framed, framedLen);
    memcpy(backup, framed, framedLen * sizeof(int));

    printf("\nDo you want to flip a bit to simulate a network error? (1 = Yes, 0 = No): ");
    scanf("%d", &wantToFlip);
    if (wantToFlip == 1) {
        printf("Enter bit index to flip (0 to %d): ", framedLen - 1);
        scanf("%d", &flipIndex);
        if (flipIndex >= 0 && flipIndex < framedLen) {
            framed[flipIndex] = !framed[flipIndex];
            printf("Bit at index %d has been flipped successfully!\n", flipIndex);
            printBits("Corrupted Bin: ", framed, framedLen);
        } else
            printf("Invalid index. Proceeding without bit flip.\n");
    }

    printf("\n--- RECEIVER SIDE ---\n");
    if (receive(framed, framedLen, n, &destuffedLen)) {
        printBits("Actual data:    ", backup, framedLen);
        printBits("Changed data:   ", framed, framedLen);
        printf("Both don't match, so message discarded.\n");
        return;
    }

    printBits("Destuffed data: ", destuffed, destuffedLen);
    for (int i = 0; i < destuffedLen; i += 8) {     /* every 8 bits -> one character */
        for (int b = 0; b < 8; b++)
            bin_str[b] = '0' + destuffed[i + b];
        bin_str[8] = '\0';
        out_ascii_arr[chars++] = bin_to_ascii(bin_str);
    }
    ascii_to_str(out_ascii_arr, chars, outputString);
    printf("Output Text  : %s\n", outputString);
    printf("Data matches perfectly!\n");
}

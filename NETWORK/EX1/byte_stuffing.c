#include <stdio.h>
#include <string.h>
#include "../convert/convert.h"
#include "ex1.h"

#define MAX_STR   100
#define MAX_BYTES 500

static int dataBytes[MAX_BYTES], stuffedBytes[MAX_BYTES], framedBytes[MAX_BYTES], destuffedBytes[MAX_BYTES];
static int sofByte, eofByte, escByte;

static void printByte(int byte) {
    for (int i = 7; i >= 0; i--)
        printf("%d", (byte >> i) & 1);
}

static void printBytesAsBinary(const char *label, const int *arr, int n) {
    printf("%s", label);
    for (int i = 0; i < n; i++) {
        printByte(arr[i]);
        printf(" ");
    }
    printf("\n");
}

/* special bytes are shown as S / F / E, normal bytes stay as they are */
static int mapChar(int b) {
    return b == sofByte ? 'S' : b == eofByte ? 'F' : b == escByte ? 'E' : b;
}

static void printText(const char *label, const int *arr, int n) {
    printf("%s", label);
    for (int i = 0; i < n; i++)
        printf("%c", (char)mapChar(arr[i]));
}

static int readFrameByte(const char *name) {
    char s[9];
    printf("Enter %s (Press 1 for default 01111110): ", name);
    scanf("%8s", s);
    return strcmp(s, "1") == 0 ? 0x7E : (int)bin_to_ascii(s);
}

/* Receiver: check SOF/EOF, remove escape bytes, compare with original data.
   Returns 1 if any error is found. */
static int receive(int framedLen, int numBytes, int *destuffedLen) {
    int error = framedBytes[0] != sofByte || framedBytes[framedLen - 1] != eofByte, m = 0;

    for (int i = 1; !error && i < framedLen - 1; i++) {
        if (framedBytes[i] == escByte) {
            if (i + 1 >= framedLen - 1)
                error = 1;
            else
                destuffedBytes[m++] = framedBytes[++i];   /* take the escaped byte as data */
        } else if (framedBytes[i] == sofByte || framedBytes[i] == eofByte)
            error = 1;
        else
            destuffedBytes[m++] = framedBytes[i];
    }
    *destuffedLen = m;
    return error || m != numBytes || memcmp(destuffedBytes, dataBytes, numBytes * sizeof(int)) != 0;
}

void byteStuffing(void) {
    char inputString[MAX_STR], ebits[9], outputString[MAX_STR];
    int ascii_arr[MAX_STR], out_ascii_arr[MAX_STR], backup[MAX_BYTES];
    int numBytes, stuffedLen = 0, framedLen, destuffedLen, wantToCorrupt = 0;

    printf("\n--- BYTE STUFFING CONFIGURATION ---\n");
    sofByte = readFrameByte("Start of Frame");
    eofByte = readFrameByte("End of Frame");
    printf("Enter Escape byte (8 bits): ");
    scanf("%8s", ebits);
    escByte = (int)bin_to_ascii(ebits);

    printf("Enter the data input string: ");
    scanf(" %99[^\n]", inputString);
    str_to_ascii(inputString, ascii_arr, &numBytes);

    /* typing 'F' means a SOF-valued byte, 'E' means an ESC-valued byte */
    for (int i = 0; i < numBytes; i++)
        dataBytes[i] = inputString[i] == 'F' ? sofByte : inputString[i] == 'E' ? escByte : ascii_arr[i];

    printf("\n--- TRANSMITTER SIDE ---\n");
    printf("SOF  = "); printByte(sofByte);
    printf("\nEOF  = "); printByte(eofByte);
    printf("\nEsc  = "); printByte(escByte);
    printf("\n");
    printBytesAsBinary("Original data: ", dataBytes, numBytes);

    /* put an ESC before every SOF / EOF / ESC byte in the data */
    for (int i = 0; i < numBytes; i++) {
        if (dataBytes[i] == sofByte || dataBytes[i] == eofByte || dataBytes[i] == escByte)
            stuffedBytes[stuffedLen++] = escByte;
        stuffedBytes[stuffedLen++] = dataBytes[i];
    }
    printBytesAsBinary("Stuffed data : ", stuffedBytes, stuffedLen);

    /* frame = SOF + stuffed data + EOF */
    framedBytes[0] = sofByte;
    memcpy(framedBytes + 1, stuffedBytes, stuffedLen * sizeof(int));
    framedBytes[stuffedLen + 1] = eofByte;
    framedLen = stuffedLen + 2;
    printBytesAsBinary("Framed data  : ", framedBytes, framedLen);
    memcpy(backup, framedBytes, framedLen * sizeof(int));

    printf("\nDo you want to corrupt a byte to simulate a network error? (1 = Yes, 0 = No): ");
    scanf("%d", &wantToCorrupt);
    if (wantToCorrupt == 1) {
        int corruptIndex = 0, newValue = 0;
        printf("Enter byte index to change (0 to %d): ", framedLen - 1);
        scanf("%d", &corruptIndex);
        if (corruptIndex >= 0 && corruptIndex < framedLen) {
            printf("Enter new decimal value for this byte (0-255): ");
            scanf("%d", &newValue);
            framedBytes[corruptIndex] = newValue;
            printf("Byte at index %d altered successfully!\n", corruptIndex);
            printBytesAsBinary("Corrupted data: ", framedBytes, framedLen);
        } else
            printf("Invalid index. Proceeding without byte manipulation.\n");
    }

    printf("\n--- RECEIVER SIDE ---\n");
    if (receive(framedLen, numBytes, &destuffedLen)) {
        printText("Actual data:    ", backup, framedLen);
        printText("\nChanged data:   ", framedBytes, framedLen);
        printf("\nBoth don't match, so message discarded.\n");
        return;
    }

    printBytesAsBinary("Destuffed data: ", destuffedBytes, destuffedLen);
    for (int i = 0; i < destuffedLen; i++)
        out_ascii_arr[i] = mapChar(destuffedBytes[i]);
    ascii_to_str(out_ascii_arr, destuffedLen, outputString);
    printf("Output Text   : %s\n", outputString);
    printf("Data matches perfectly!\n");
}

#include <stdio.h>
#include <string.h>
#include "../convert/convert.h"

#define MAX_STR 100
#define MAX_BITS 1000
#define MAX_BYTES 500

int data[MAX_BITS], stuffed[MAX_BITS], destuffed[MAX_BITS];
int dataBytes[MAX_BYTES], stuffedBytes[MAX_BYTES], framedBytes[MAX_BYTES], destuffedBytes[MAX_BYTES];
int flag[] = {0,1,1,1,1,1,1,0};
int flagLen = 8;
int flagByte, escByte;

void printBits(char *label, int arr[], int n) {
    int i;
    printf("%s", label);
    for (i = 0; i < n; i++) {
        printf("%d", arr[i]);
        if ((i + 1) % 8 == 0 && i != n - 1) {
            printf(" ");
        }
    }
    printf("\n");
}

void printByteArray(int byte) {
    int i;
    for (i = 7; i >= 0; i--) {
        printf("%d", (byte >> i) & 1);
    }
}

void printBytesAsBinary(char *label, int arr[], int n) {
    int i;
    printf("%s", label);
    for (i = 0; i < n; i++) {
        printByteArray(arr[i]);
        printf(" ");
    }
    printf("\n");
}

void bitStuffing() {
    char inputString[MAX_STR];
    int ascii_arr[MAX_STR];
    int str_len = 0;
    int i, b, j, k, n = 0;
    int ones = 0;
    int stuffedLen = 0;
    int framedLen = 0;
    int destuffedLen = 0;
    int match = 1;
    char bin_str[9];
    int out_ascii_arr[MAX_STR];
    int out_char_count = 0;
    char outputString[MAX_STR];
    int framed[MAX_BITS];
    int originalFramedBackup[MAX_BITS];

    printf("\n--- BIT STUFFING CONFIGURATION ---\n");
    printf("Enter the data input string: ");
    scanf("%99s", inputString);

    str_to_ascii(inputString, ascii_arr, &str_len);

    for (i = 0; i < str_len; i++) {
        ascii_to_bin((char)ascii_arr[i], bin_str);
        for (b = 0; b < 8; b++) {
            data[n++] = (bin_str[b] == '1') ? 1 : 0;
        }
    }

    printf("\n--- TRANSMITTER SIDE ---\n");
    printf("Input String : %s\n", inputString);
    printBits("Original Bin : ", data, n);

    ones = 0;
    j = 0;
    for (i = 0; i < n; i++) {
        stuffed[j++] = data[i];
        ones = (data[i] == 1) ? ones + 1 : 0;
        if (ones == 5) {
            stuffed[j++] = 0;
            ones = 0;
        }
    }
    stuffedLen = j;
    printBits("Stuffed Bin  : ", stuffed, stuffedLen);

    k = 0;
    for (i = 0; i < flagLen; i++) framed[k++] = flag[i];
    for (i = 0; i < stuffedLen; i++) framed[k++] = stuffed[i];
    for (i = 0; i < flagLen; i++) framed[k++] = flag[i];
    framedLen = k;
    printBits("Framed Bin   : ", framed, framedLen);

    for (i = 0; i < framedLen; i++) {
        originalFramedBackup[i] = framed[i];
    }

    int wantToFlip = 0;
    printf("\nDo you want to flip a bit to simulate a network error? (1 = Yes, 0 = No): ");
    scanf("%d", &wantToFlip);
    
    if (wantToFlip == 1) {
        int flipIndex = 0;
        printf("Enter bit index to flip (0 to %d): ", framedLen - 1);
        scanf("%d", &flipIndex);
        
        if (flipIndex >= 0 && flipIndex < framedLen) {
            framed[flipIndex] = !framed[flipIndex];
            printf("Bit at index %d has been flipped successfully!\n", flipIndex);
            printBits("Corrupted Bin: ", framed, framedLen);
        } else {
            printf("Invalid index. Proceeding without bit flip.\n");
        }
    }

    printf("\n--- RECEIVER SIDE ---\n");
    
    int error = 0;
    ones = 0;
    j = 0;

    for (i = 0; i < flagLen; i++) {
        if (framed[i] != flag[i] || framed[framedLen - flagLen + i] != flag[i]) {
            error = 1;
        }
    }

    if (!error) {
        for (i = flagLen; i < framedLen - flagLen; i++) {
            if (ones == 5) {
                if (framed[i] != 0) {
                    error = 1;
                    break;
                }
                ones = 0;
                continue; 
            }
            destuffed[j++] = framed[i];
            ones = (framed[i] == 1) ? ones + 1 : 0;
        }
        destuffedLen = j;
    }

    if (!error && destuffedLen == n) {
        for (i = 0; i < n; i++) {
            if (destuffed[i] != data[i]) {
                error = 1;
                break;
            }
        }
    } else {
        error = 1;
    }

    if (error == 1) {
        printBits("Actual data:    ", originalFramedBackup, framedLen);
        printBits("Changed data:   ", framed, framedLen);
        printf("Both don't match, so message discarded.\n");
    } else {
        printBits("Destuffed data: ", destuffed, destuffedLen);
        
        out_char_count = 0;
        for (i = 0; i < destuffedLen; i += 8) {
            for (b = 0; b < 8; b++) {
                bin_str[b] = (destuffed[i + b] == 1) ? '1' : '0';
            }
            bin_str[8] = '\0';
            out_ascii_arr[out_char_count++] = (int)bin_to_ascii(bin_str);
        }
        ascii_to_str(out_ascii_arr, out_char_count, outputString);
        printf("Output Text  : %s\n", outputString);
        printf("Data matches perfectly!\n");
    }
}

void byteStuffing() {
    char inputString[MAX_STR];
    int ascii_arr[MAX_STR];
    int str_len = 0;
    int i, j, k, m;
    int stuffedLen = 0;
    int framedLen = 0;
    int destuffedLen = 0;
    int numBytes = 0;
    int error = 0;
    char ebits[9];
    char sofChoice[9];
    char eofChoice[9];
    int sofByte, eofByte;
    int out_ascii_arr[MAX_STR];
    char outputString[MAX_STR];
    int originalFramedBackup[MAX_BYTES];

    printf("\n--- BYTE STUFFING CONFIGURATION ---\n");
    
    printf("Enter Start of Frame (Press 1 for default 01111110): ");
    scanf("%8s", sofChoice);
    if (strcmp(sofChoice, "1") == 0) {
        sofByte = 0b01111110;
    } else {
        sofByte = (int)bin_to_ascii(sofChoice);
    }

    printf("Enter End of Frame (Press 1 for default 01111110): ");
    scanf("%8s", eofChoice);
    if (strcmp(eofChoice, "1") == 0) {
        eofByte = 0b01111110;
    } else {
        eofByte = (int)bin_to_ascii(eofChoice);
    }

    printf("Enter Escape byte (8 bits): ");
    scanf("%8s", ebits);
    escByte = (int)bin_to_ascii(ebits);

    printf("Enter the data input string: ");
    scanf(" %99[^\n]", inputString);
    
    str_to_ascii(inputString, ascii_arr, &str_len);
    numBytes = str_len;

    for (i = 0; i < numBytes; i++) {
        if (inputString[i] == 'F') {
            dataBytes[i] = sofByte;
        } else if (inputString[i] == 'E') {
            dataBytes[i] = escByte;
        } else {
            dataBytes[i] = ascii_arr[i];
        }
    }

    printf("\n--- TRANSMITTER SIDE ---\n");
    printf("SOF  = "); printByteArray(sofByte);
    printf("\nEOF  = "); printByteArray(eofByte);
    printf("\nEsc  = "); printByteArray(escByte);
    printf("\n");
    printBytesAsBinary("Original data: ", dataBytes, numBytes);

    j = 0;
    for (i = 0; i < numBytes; i++) {
        if (dataBytes[i] == sofByte || dataBytes[i] == eofByte || dataBytes[i] == escByte) {
            stuffedBytes[j++] = escByte;
            stuffedBytes[j++] = dataBytes[i];
        } else {
            stuffedBytes[j++] = dataBytes[i];
        }
    }
    stuffedLen = j;
    printBytesAsBinary("Stuffed data : ", stuffedBytes, stuffedLen);

    k = 0;
    framedBytes[k++] = sofByte;
    for (i = 0; i < stuffedLen; i++) {
        framedBytes[k++] = stuffedBytes[i];
    }
    framedBytes[k++] = eofByte;
    framedLen = k;
    printBytesAsBinary("Framed data  : ", framedBytes, framedLen);

    for (i = 0; i < framedLen; i++) {
        originalFramedBackup[i] = framedBytes[i];
    }

    int wantToCorrupt = 0;
    printf("\nDo you want to corrupt a byte to simulate a network error? (1 = Yes, 0 = No): ");
    scanf("%d", &wantToCorrupt);

    if (wantToCorrupt == 1) {
        int corruptIndex = 0;
        int newValue = 0;
        printf("Enter byte index to change (0 to %d): ", framedLen - 1);
        scanf("%d", &corruptIndex);

        if (corruptIndex >= 0 && corruptIndex < framedLen) {
            printf("Enter new decimal value for this byte (0-255): ");
            scanf("%d", &newValue);
            framedBytes[corruptIndex] = newValue;
            printf("Byte at index %d altered successfully!\n", corruptIndex);
            printBytesAsBinary("Corrupted data: ", framedBytes, framedLen);
        } else {
            printf("Invalid index. Proceeding without byte manipulation.\n");
        }
    }

    printf("\n--- RECEIVER SIDE ---\n");
    
    if (framedBytes[0] != sofByte || framedBytes[framedLen - 1] != eofByte) {
        error = 1;
    } else {
        m = 0;
        for (i = 1; i < framedLen - 1; i++) {
            if (framedBytes[i] == escByte) {
                if (i + 1 >= framedLen - 1) { error = 1; break; }
                i++;
                destuffedBytes[m++] = framedBytes[i];
            } else if (framedBytes[i] == sofByte || framedBytes[i] == eofByte) {
                error = 1;
                break;
            } else {
                destuffedBytes[m++] = framedBytes[i];
            }
        }
        destuffedLen = m;
    }

    if (!error && destuffedLen == numBytes) {
        for (i = 0; i < numBytes; i++) {
            if (destuffedBytes[i] != dataBytes[i]) { error = 1; break; }
        }
    } else {
        error = 1;
    }

    if (error == 1) {
        printf("Actual data:    ");
        for (i = 0; i < framedLen; i++) {
            if (originalFramedBackup[i] == sofByte) printf("S");
            else if (originalFramedBackup[i] == eofByte) printf("F");
            else if (originalFramedBackup[i] == escByte) printf("E");
            else printf("%c", (char)originalFramedBackup[i]);
        }
        printf("\nChanged data:   ");
        for (i = 0; i < framedLen; i++) {
            if (framedBytes[i] == sofByte) printf("S");
            else if (framedBytes[i] == eofByte) printf("F");
            else if (framedBytes[i] == escByte) printf("E");
            else printf("%c", (char)framedBytes[i]);
        }
        printf("\nBoth don't match, so message discarded.\n");
    } else {
        printBytesAsBinary("Destuffed data: ", destuffedBytes, destuffedLen);
        
        for (i = 0; i < destuffedLen; i++) {
            if (destuffedBytes[i] == sofByte) {
                out_ascii_arr[i] = (int)'S';
            } else if (destuffedBytes[i] == eofByte) {
                out_ascii_arr[i] = (int)'F';
            } else if (destuffedBytes[i] == escByte) {
                out_ascii_arr[i] = (int)'E';
            } else {
                out_ascii_arr[i] = destuffedBytes[i];
            }
        }
        ascii_to_str(out_ascii_arr, destuffedLen, outputString);
        printf("Output Text   : %s\n", outputString);
        printf("Data matches perfectly!\n");
    }
}


int main() {
    int choice;
    while (1) {
        printf("\n1. Bit Stuffing\n");
        printf("2. Byte Stuffing\n");
        printf("3. Exit\n");
        printf("Enter your choice (1, 2, or 3): ");
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                bitStuffing();
                break;
            case 2:
                byteStuffing();
                break;
            case 3:
                printf("\nExiting program\n");
                return 0;
            default:
                printf("Invalid selection. Please enter 1, 2, or 3.\n");
                break;
        }
    }
    return 0;
}

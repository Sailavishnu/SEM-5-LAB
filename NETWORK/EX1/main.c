#include <stdio.h>
#include <string.h>
#include "../convert/convert.h"

#define MAX_STR 100
#define MAX_BITS 1000
#define MAX_BYTES 500

// Shared memory arrays for protocols
int data[MAX_BITS], stuffed[MAX_BITS], destuffed[MAX_BITS];
int dataBytes[MAX_BYTES], stuffedBytes[MAX_BYTES], framedBytes[MAX_BYTES], destuffedBytes[MAX_BYTES];

// Bit stuffing markers
int flag[] = {0,1,1,1,1,1,1,0};
int flagLen = 8;

// Byte stuffing markers
int flagByte, escByte;

// ========== UTILITY FUNCTIONS ==========

void printBits(char *label, int arr[], int n) {
    int i;
    printf("%s", label);
    for (i = 0; i < n; i++) {
        printf("%d", arr[i]);
        // Add space after every 8 bits for readability
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

// ========== BIT STUFFING FUNCTION ==========

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

    printf("\n--- BIT STUFFING CONFIGURATION ---\n");
    printf("Enter the data input string: ");
    scanf("%s", inputString);

    // 1. Convert input text to raw ASCII array
    str_to_ascii(inputString, ascii_arr, &str_len);

    // 2. Unpack ASCII integers into sequential flat bit arrays
    for (i = 0; i < str_len; i++) {
        ascii_to_bin((char)ascii_arr[i], bin_str);
        for (b = 0; b < 8; b++) {
            data[n++] = (bin_str[b] == '1') ? 1 : 0;
        }
    }

    printf("\n--- TRANSMITTER SIDE ---\n");
    printf("Input String : %s\n", inputString);
    printBits("Original Bin : ", data, n);

    // ---- Bit stuffing logic ----
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

    // ---- Framing logic ----
    k = 0;
    for (i = 0; i < flagLen; i++) framed[k++] = flag[i];
    for (i = 0; i < stuffedLen; i++) framed[k++] = stuffed[i];
    for (i = 0; i < flagLen; i++) framed[k++] = flag[i];
    framedLen = k;
    printBits("Framed Bin   : ", framed, framedLen);

    printf("\n--- RECEIVER SIDE ---\n");

    // ---- Destuffing logic ----
    ones = 0;
    j = 0;
    for (i = flagLen; i < framedLen - flagLen; i++) {
        if (ones == 5) {
            if (framed[i] != 0) {
                printf("\nError: Invalid stuffing detected\n");
                return;
            }
            ones = 0;
            continue; // drop stuffed 0
        }
        destuffed[j++] = framed[i];
        ones = (framed[i] == 1) ? ones + 1 : 0;
    }
    destuffedLen = j;
    printBits("Destuffed Bin: ", destuffed, destuffedLen);

    // 3. Assemble destuffed serial stream chunks back to characters
    out_char_count = 0;
    for (i = 0; i < destuffedLen; i += 8) {
        for (b = 0; b < 8; b++) {
            bin_str[b] = (destuffed[i + b] == 1) ? '1' : '0';
        }
        bin_str[8] = '\0';
        out_ascii_arr[out_char_count++] = (int)bin_to_ascii(bin_str);
    }

    // 4. Translate back into string format
    ascii_to_str(out_ascii_arr, out_char_count, outputString);
    printf("Output Text  : %s\n", outputString);

    // ---- Final Bit Match Verification ----
    match = 1;
    if (destuffedLen != n) match = 0;
    else {
        for (i = 0; i < n; i++) {
            if (destuffed[i] != data[i]) { match = 0; break; }
        }
    }
    printf("\nVerification : %s\n", match ? "SUCCESS (Data matches perfectly)" : "FAIL (Mismatch error)");
}

// ========== BYTE STUFFING FUNCTION ==========

void byteStuffing() {
    char inputString[MAX_STR];
    int ascii_arr[MAX_STR];
    int str_len = 0;
    int i, j, k, m;
    int stuffedLen = 0;
    int framedLen = 0;
    int destuffedLen = 0;
    int numBytes = 0;
    int match = 1;
    int error = 0;
    char ebits[9];
    int out_ascii_arr[MAX_STR];
    char outputString[MAX_STR];

    printf("\n--- BYTE STUFFING CONFIGURATION ---\n");
    // Standard flag mapping rule configuration
    flagByte = 0b01111110;
    printf("Flag Byte is standard: 01111110\n");

    printf("Enter Escape byte (8 bits): ");
    scanf("%s", ebits);
    escByte = (int)bin_to_ascii(ebits);

    printf("Enter the data input string: ");
    scanf(" %[^\n]", inputString);

    // 1. Map input string to ASCII integers
    str_to_ascii(inputString, ascii_arr, &str_len);
    numBytes = str_len;

    // 2. Map logical 'F' and 'E' characters to their underlying binary layers
    for (i = 0; i < numBytes; i++) {
        if (inputString[i] == 'F') {
            dataBytes[i] = flagByte;
        } else if (inputString[i] == 'E') {
            dataBytes[i] = escByte;
        } else {
            dataBytes[i] = ascii_arr[i];
        }
    }

    printf("\n--- TRANSMITTER SIDE ---\n");
    printf("Flag = "); printByteArray(flagByte);
    printf("\nEsc  = "); printByteArray(escByte);
    printf("\n");
    printBytesAsBinary("Original data: ", dataBytes, numBytes);

    // ---- Byte stuffing logic ----
    j = 0;
    for (i = 0; i < numBytes; i++) {
        if (dataBytes[i] == flagByte || dataBytes[i] == escByte) {
            stuffedBytes[j++] = escByte;
            stuffedBytes[j++] = dataBytes[i];
        } else {
            stuffedBytes[j++] = dataBytes[i];
        }
    }
    stuffedLen = j;
    printBytesAsBinary("Stuffed data : ", stuffedBytes, stuffedLen);

    // ---- Framing logic ----
    k = 0;
    framedBytes[k++] = flagByte;
    for (i = 0; i < stuffedLen; i++) {
        framedBytes[k++] = stuffedBytes[i];
    }
    framedBytes[k++] = flagByte;
    framedLen = k;
    printBytesAsBinary("Framed data  : ", framedBytes, framedLen);

    // ---- Validate frame structures ----
    if (framedBytes[0] != flagByte || framedBytes[framedLen - 1] != flagByte) {
        printf("\nError: frame missing flags\n");
        return;
    }

    printf("\n--- RECEIVER SIDE ---\n");

    // ---- Destuffing logic ----
    m = 0;
    for (i = 1; i < framedLen - 1; i++) {
        if (framedBytes[i] == escByte) {
            if (i + 1 >= framedLen - 1) {
                printf("Dangling esc byte with no following byte\n");
                error = 1;
                break;
            }
            i++;
            destuffedBytes[m++] = framedBytes[i];
        } else if (framedBytes[i] == flagByte) {
            printf("Error: unexpected flag inside frame\n");
            error = 1;
            break;
        } else {
            destuffedBytes[m++] = framedBytes[i];
        }
    }
    destuffedLen = m;

    if (!error) {
        printBytesAsBinary("Destuffed    : ", destuffedBytes, destuffedLen);

        // 3. Map binary blocks back to human-visible text representations
        for (i = 0; i < destuffedLen; i++) {
            if (destuffedBytes[i] == flagByte) {
                out_ascii_arr[i] = (int)'F';
            } else if (destuffedBytes[i] == escByte) {
                out_ascii_arr[i] = (int)'E';
            } else {
                out_ascii_arr[i] = destuffedBytes[i];
            }
        }

        // 4. Translate character map array to plain output text string
        ascii_to_str(out_ascii_arr, destuffedLen, outputString);
        printf("Output Text  : %s\n", outputString);

        // ---- Final Byte Match Verification ----
        if (destuffedLen == numBytes) {
            match = 1;
            for (i = 0; i < numBytes; i++) {
                if (destuffedBytes[i] != dataBytes[i]) { match = 0; break; }
            }
            printf("\nVerification : %s\n", match ? "SUCCESS (Data matches perfectly)" : "FAIL (Mismatch error)");
        } else {
            printf("\nLength mismatch after destuff\n");
        }
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
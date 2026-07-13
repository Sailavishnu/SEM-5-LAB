#include <stdio.h>
#define MAX 100

int dataBits[MAX];
int dataBytes[MAX], stuffedBytes[MAX], framedBytes[MAX], destuffedBytes[MAX];
int flagByte, escByte;

int bitsToByte(int bits[]) {
    int val = 0;
    for (int i = 0; i < 8; i++)
        val = val * 2 + bits[i];
    return val;
}

void printByteArray(int byte) {
    for (int i = 7; i >= 0; i--)
        printf("%d", (byte >> i) & 1);
}

void printBytesAsBinary(char *label, int arr[], int n) {
    printf("%s", label);
    for (int i = 0; i < n; i++) {
        printByteArray(arr[i]);
        printf(" ");
    }
    printf("\n");
}

int main() {
    int n;
    int fbits[8], ebits[8];

    printf("Enter flag Byte (8 bits): ");
    for (int i = 0; i < 8; i++)
        scanf("%d", &fbits[i]);
    flagByte = bitsToByte(fbits);

    printf("Enter Escape byte (8 bits): ");
    for (int i = 0; i < 8; i++)
        scanf("%d", &ebits[i]);
    escByte = bitsToByte(ebits);

    printf("Enter number of data bits (mult of 8): ");
    scanf("%d", &n);

    printf("Enter Binary data: ");
    for (int i = 0; i < n; i++)
        scanf("%d", &dataBits[i]);

    int numBytes = n / 8;
    for (int i = 0; i < numBytes; i++) {
        int tmp[8];
        for (int b = 0; b < 8; b++)
            tmp[b] = dataBits[i * 8 + b];
        dataBytes[i] = bitsToByte(tmp);
    }

    printf("\nFlag = ");
    printByteArray(flagByte);
    printf("\nEsc  = ");
    printByteArray(escByte);
    printf("\n");

    printBytesAsBinary("Original data: ", dataBytes, numBytes);

    // ---- Byte stuffing ----
    int j = 0;
    for (int i = 0; i < numBytes; i++) {
        if (dataBytes[i] == flagByte || dataBytes[i] == escByte) {
            stuffedBytes[j++] = escByte;
            stuffedBytes[j++] = dataBytes[i];
        } else {
            stuffedBytes[j++] = dataBytes[i];
        }
    }
    int stuffedLen = j;
    printBytesAsBinary("Stuffed data: ", stuffedBytes, stuffedLen);

    // ---- Framing ----
    int k = 0;
    framedBytes[k++] = flagByte;
    for (int i = 0; i < stuffedLen; i++)
        framedBytes[k++] = stuffedBytes[i];
    framedBytes[k++] = flagByte;
    int framedLen = k;

    printBytesAsBinary("Framed data: ", framedBytes, framedLen);

    // ---- Validate frame flags ----
    int error = 0;
    if (framedBytes[0] != flagByte || framedBytes[framedLen - 1] != flagByte) {
        printf("\nError: frame missing flag\n");
        return 0;
    }

    // ---- Destuffing ----
    int m = 0;
    for (int i = 1; i < framedLen - 1; i++) {
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

    if (!error) {
        printBytesAsBinary("Destuffed Data: ", destuffedBytes, m);

        if (m == numBytes) {
            int match = 1;
            for (int i = 0; i < numBytes; i++) {
                if (destuffedBytes[i] != dataBytes[i]) {
                    match = 0;
                    break;
                }
            }
            printf("%s\n", match ? "Destuffing successful" : "Mismatch error destuffing");
        } else {
            printf("Length mismatch after destuff\n");
        }
    }

    return 0;
}





/*
All test cases pass. Here's the summary:

**Bit Stuffing — flag = `01111110`**

| Test | Input | Original | Stuffed | Result |
|---|---|---|---|---|
| 1 | `1 1 1 1 1 0 1 1 1 1 1 1 1 0` | `11111011111110` | `1111100111110110` | ✅ one 0 inserted after each run of five 1s |
| 2 | `1 0 1 0 1 0 1 0` | `10101010` | `10101010` | ✅ no stuffing (no 5 consecutive 1s) |
| 3 | `1 1 1 1 1 1 1 1 1 1 1 1` | `111111111111` | `11111011111011` | ✅ two 0s inserted (two runs of five 1s) |

**Byte Stuffing — flag = `01111110`, esc = `11011011`**

| Test | Data bytes | Contains | Stuffed result | Result |
|---|---|---|---|---|
| 1 | `01111110 11011011` | flag byte | esc+flag, esc+esc | ✅ correct escaping |
| 2 | `11011011 00000001` | esc byte | esc+esc, plain byte | ✅ correct escaping |
| 3 | `00000001 00000010` | neither | unchanged | ✅ no stuffing needed |

All 6 runs show correct stuffing, framing, destuffing, and a final match confirmation. Want me to also add specific test cases with deliberately broken/corrupted frames (to show the error-detection paths)?
*/s
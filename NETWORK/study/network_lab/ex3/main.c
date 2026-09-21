#include <stdio.h>
#include "hamming.h"

int main(void) {
    int choice, parityType;
    char input[MAX];

    printf("=== HAMMING CODE PROGRAM ===\n");
    printf("1. Encode data (generate Hamming code)\n2. Check received code for error and correct it\n");
    printf("Enter choice: ");
    scanf("%d", &choice);
    printf("Enter parity type (0 = Even, 1 = Odd): ");
    scanf("%d", &parityType);

    if (choice == 1) {
        printf("Enter data bits (e.g. 1011010): ");
        scanf("%s", input);
        generateHammingCode(input, parityType);
    } else if (choice == 2) {
        printf("Enter received code (position N ... position 1, e.g. 10101010111): ");
        scanf("%s", input);
        checkReceivedCode(input, parityType);
    } else
        printf("Invalid choice.\n");
    return 0;
}

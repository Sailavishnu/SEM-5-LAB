#include <stdio.h>
#include "ex2.h"

int main(void) {
    int choice;
    do {
        printf("\n================ ERROR DETECTION TECHNIQUES ================\n");
        printf(" 1. Parity\n 2. CRC\n 3. Checksum\n 4. Exit\n");
        printf("==============================================================\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1: parity();             break;
            case 2: crc_technique();      break;
            case 3: checksum_technique(); break;
            case 4: printf("Exiting...\n"); break;
            default: printf("Invalid choice, try again.\n");
        }
    } while (choice != 4);
    return 0;
}

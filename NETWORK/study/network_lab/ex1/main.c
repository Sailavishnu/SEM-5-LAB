#include <stdio.h>
#include "ex1.h"

int main(void) {
    int choice;
    while (1) {
        printf("\n1. Bit Stuffing\n2. Byte Stuffing\n3. Exit\n");
        printf("Enter your choice (1, 2, or 3): ");
        scanf("%d", &choice);
        switch (choice) {
            case 1: bitStuffing();  break;
            case 2: byteStuffing(); break;
            case 3: printf("\nExiting program\n"); return 0;
            default: printf("Invalid selection. Please enter 1, 2, or 3.\n");
        }
    }
}

#include <stdio.h>
#include "window.h"

int main(void) {
    int choice, n, w, lost;

    do {
        printf("========== SLIDING WINDOW MENU ==========\n");
        printf("  1. Stop-and-Wait\n  2. Go-Back-N\n  3. Selective Repeat\n  4. Exit\n");
        printf("==========================================\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        printf("\n");

        if (choice == 4) {
            printf("Exiting...\n");
            break;
        }
        if (choice < 1 || choice > 4) {
            printf("Invalid choice! Try again.\n\n");
            continue;
        }

        printf("No. of frames  : ");
        scanf("%d", &n);
        printf("Frame to lose  : ");
        scanf("%d", &lost);
        w = 1;                                 /* Stop-and-Wait has no window size */
        if (choice != 1) {
            printf("Window size (N): ");
            scanf("%d", &w);
        }

        if (n <= 0 || n > MAX || w <= 0 || w > n) {
            printf("\nInvalid input configurations!\n\n");
            continue;
        }
        if (lost >= n) {
            printf("\nInvalid lost frame number!\n\n");
            continue;
        }

        switch (choice) {
            case 1: stopAndWait(n, lost);      break;
            case 2: goBackN(n, w, lost);       break;
            case 3: selectiveRepeat(n, w, lost); break;
        }
    } while (choice != 4);
    return 0;
}

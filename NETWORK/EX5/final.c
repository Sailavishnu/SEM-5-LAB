#include <stdio.h>
#include <stdlib.h>

#define MAX 50

/* Function for Stop-and-Wait (Window Size is implicitly 1) */
void stopAndWait(int frames, int lostFrame) {
    int Sn = 0; // Sequence number pointer
    int step = 1, total = 0;

    printf("\n================ STOP-AND-WAIT ================\n");
    printf("+------+-------+---------+------------+\n");
    printf("| Step | Frame | Status  | Window     |\n");
    printf("+------+-------+---------+------------+\n");

    while (Sn < frames) {
        total++;
        printf("\n"); // Clear line-by-line separation spacing

        if (Sn == lostFrame) {
            printf("| %-4d | %-5d | %-7s | [%d]        |\n", step++, Sn, "LOST", Sn);
            printf("      -> Frame %d lost! Timer started... ⏰\n", Sn);
            printf("      -> *** TIMEOUT ***. Resending same frame.\n");
            lostFrame = -1; // Clear trigger so it succeeds on retry
        } else {
            printf("| %-4d | %-5d | %-7s | [%d]        |\n", step++, Sn, "ACK", Sn);
            printf("      -> Frame %d successfully delivered.\n", Sn);
            printf("      -> ACK %d received. Moving to next frame.\n", Sn + 1);
            Sn++; // Move forward to the next frame sequentially
        }
    }

    printf("\n+------+-------+---------+------------+\n");
    printf("Frames delivered : %d\n", frames);
    printf("Total transmits  : %d\n", total);
    printf("=============================================\n\n");
}

/* Function for Go-Back-N */
void goBackN(int frames, int N, int lostFrame) {
    int Sb = 0; // Send Base
    int Sm;     // Send Max (End of window)
    int Sn;     // Sequence Number pointer
    int step = 1, total = 0;

    printf("\n================ GO-BACK-N ================\n");
    printf("+------+-------+---------+------------+\n");
    printf("| Step | Frame | Status  | Window     |\n");
    printf("+------+-------+---------+------------+\n");

    while (Sb < frames) {
        Sm = Sb + N - 1; // Calculate current window bounds
        int lost_occurred = 0;
        int original_Sb = Sb;

        printf("\n"); // Vertical block padding spacing

        // Try to transmit the current window pipeline
        for (Sn = Sb; Sn <= Sm && Sn < frames; Sn++) {
            total++;
            if (Sn == lostFrame) {
                printf("| %-4d | %-5d | %-7s | [%d - %d] |\n",
                       step++, Sn, "LOST", original_Sb, (Sm < frames-1 ? Sm : frames-1));
                printf("      -> Frame %d lost! Receiver rejects subsequent pipeline frames.\n", Sn);
                lostFrame = -1; // Clear trigger so it succeeds on retry
                lost_occurred = 1;
                break; // Stop transmitting the rest of the window right now
            } else {
                printf("| %-4d | %-5d | %-7s | [%d - %d] |\n",
                       step++, Sn, "ACK", original_Sb, (Sm < frames-1 ? Sm : frames-1));
                
                // Slide the base forward step-by-step for healthy frames
                if (!lost_occurred) {
                    Sb++;
                }
            }
        }

        if (lost_occurred) {
            printf("      -> Timeout! Go-Back-N triggered. Resending window from frame %d...\n", Sb);
        } else {
            if (Sb < frames) {
                printf("      -> Window successfully moved to [%d - %d]\n", Sb, (Sb + N - 1 < frames - 1 ? Sb + N - 1 : frames - 1));
            }
        }
    }

    printf("\n+------+-------+---------+------------+\n");
    printf("Frames delivered : %d\n", frames);
    printf("Total transmits  : %d\n", total);
    printf("=============================================\n\n");
}

/* Function for Selective Repeat */
void selectiveRepeat(int frames, int N, int lostFrame) {
    int Sb = 0; // Send Base
    int Sm;     // Send Max
    int step = 1, total = 0;
    
    // Track status of each individual frame: 0 = Unsent/Lost, 1 = ACKed
    int *acked = (int *)calloc(frames, sizeof(int));

    printf("\n============ SELECTIVE REPEAT ============\n");
    printf("+------+--------+---------+------------+\n");
    printf("| Step | Packet | Status  | Window     |\n");
    printf("+------+--------+---------+------------+\n");

    while (Sb < frames) {
        Sm = Sb + N - 1;
        printf("\n"); // Vertical block padding spacing

        // Transmission phase for the current window pipeline
        for (int Sn = Sb; Sn <= Sm && Sn < frames; Sn++) {
            if (acked[Sn] == 1) continue; // Skip if already successfully ACKed before

            total++;

            if (Sn == lostFrame) {
                printf("| %-4d | %-6d | %-7s | [%d - %d] |\n", 
                       step++, Sn, "LOST", Sb, (Sm < frames-1 ? Sm : frames-1));
                printf("      -> Packet %d lost. Will be selectively retransmitted later.\n", Sn);
                lostFrame = -1; // Clear trigger so it succeeds on retry
            } else {
                printf("| %-4d | %-6d | %-7s | [%d - %d] |\n", 
                       step++, Sn, "ACK", Sb, (Sm < frames-1 ? Sm : frames-1));
                acked[Sn] = 1;
                printf("      -> Packet %d delivered and buffered by receiver.\n", Sn);
            }
        }

        // Sliding Window phase: Slide Sb forward as far as continuous ACKs allow
        int oldSb = Sb;
        while (Sb < frames && acked[Sb] == 1) {
            Sb++;
        }

        if (Sb > oldSb && Sb < frames) {
            int newSm = Sb + N - 1;
            printf("      -> Window base ACKed. Moving window to [%d - %d]\n", Sb, (newSm < frames-1 ? newSm : frames-1));
        }
    }

    printf("\n+------+--------+---------+------------+\n");
    printf("Frames delivered : %d\n", frames);
    printf("Total transmits  : %d\n", total);
    printf("===========================================\n\n");
    
    free(acked);
}

/* Main Function */
int main() {
    int choice;
    int n, w, lost;

    do {
        printf("========== SLIDING WINDOW MENU ==========\n");
        printf("  1. Stop-and-Wait\n");
        printf("  2. Go-Back-N\n");
        printf("  3. Selective Repeat\n");
        printf("  4. Exit\n");
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

        // Don't ask for window size if Stop-and-Wait is picked (it is always 1)
        if (choice != 1) {
            printf("Window size (N): ");
            scanf("%d", &w);
        } else {
            w = 1;
        }

        // Basic sanity validation guards
        if (n <= 0 || n > MAX || w <= 0 || w > n) {
            printf("\nInvalid input configurations!\n\n");
            continue;
        }
        if (lost >= n) {
            printf("\nInvalid lost frame number!\n\n");
            continue;
        }

        switch (choice) {
            case 1: stopAndWait(n, lost); break;
            case 2: goBackN(n, w, lost); break;
            case 3: selectiveRepeat(n, w, lost); break;
        }
    } while (choice != 4);

    return 0;
}

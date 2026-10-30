#include <stdio.h>
#include "window.h"

/* send one frame, wait for its ACK; on loss the timer expires and the same frame is resent */
void stopAndWait(int frames, int lostFrame) {
    int Sn = 0, step = 1, total = 0;

    printf("\n================ STOP-AND-WAIT ================\n");
    printf("+------+-------+---------+------------+\n");
    printf("| Step | Frame | Status  | Window     |\n");
    printf("+------+-------+---------+------------+\n");

    while (Sn < frames) {
        int lost = (Sn == lostFrame);
        total++;
        printf("\n| %-4d | %-5d | %-7s | [%d]        |\n", step++, Sn, lost ? "LOST" : "ACK", Sn);
        if (lost) {
            printf("      -> Frame %d lost! Timer started... ⏰\n", Sn);
            printf("      -> *** TIMEOUT ***. Resending same frame.\n");
            lostFrame = -1;                    /* the resent frame gets through */
        } else {
            printf("      -> Frame %d successfully delivered.\n", Sn);
            printf("      -> ACK %d received. Moving to next frame.\n", Sn + 1);
            Sn++;
        }
    }

    printf("\n+------+-------+---------+------------+\n");
    printf("Frames delivered : %d\n", frames);
    printf("Total transmits  : %d\n", total);
    printf("=============================================\n\n");
}

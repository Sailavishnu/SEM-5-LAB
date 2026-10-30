#include <stdio.h>
#include "window.h"

/* window of N frames; when a frame is lost the sender goes back and resends from it */
void goBackN(int frames, int N, int lostFrame) {
    int Sb = 0, step = 1, total = 0;

    printf("\n================ GO-BACK-N ================\n");
    printf("+------+-------+---------+------------+\n");
    printf("| Step | Frame | Status  | Window     |\n");
    printf("+------+-------+---------+------------+\n");

    while (Sb < frames) {
        int Sm = Sb + N - 1, first = Sb, last = MIN(Sm, frames - 1), lost = 0;
        printf("\n");

        for (int Sn = Sb; Sn <= Sm && Sn < frames && !lost; Sn++) {
            total++;
            lost = (Sn == lostFrame);
            printf("| %-4d | %-5d | %-7s | [%d - %d] |\n", step++, Sn, lost ? "LOST" : "ACK", first, last);
            if (lost) {
                printf("      -> Frame %d lost! Receiver rejects subsequent pipeline frames.\n", Sn);
                lostFrame = -1;
            } else
                Sb++;                          /* ACKed: window base moves forward */
        }

        if (lost)
            printf("      -> Timeout! Go-Back-N triggered. Resending window from frame %d...\n", Sb);
        else if (Sb < frames)
            printf("      -> Window successfully moved to [%d - %d]\n", Sb, MIN(Sb + N - 1, frames - 1));
    }

    printf("\n+------+-------+---------+------------+\n");
    printf("Frames delivered : %d\n", frames);
    printf("Total transmits  : %d\n", total);
    printf("=============================================\n\n");
}

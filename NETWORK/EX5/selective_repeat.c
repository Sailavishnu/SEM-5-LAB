#include <stdio.h>
#include <stdlib.h>
#include "window.h"

/* only the lost packet is resent; the receiver buffers the others (acked[] marks them) */
void selectiveRepeat(int frames, int N, int lostFrame) {
    int Sb = 0, step = 1, total = 0;
    int *acked = calloc(frames, sizeof(int));

    printf("\n============ SELECTIVE REPEAT ============\n");
    printf("+------+--------+---------+------------+\n");
    printf("| Step | Packet | Status  | Window     |\n");
    printf("+------+--------+---------+------------+\n");

    while (Sb < frames) {
        int Sm = Sb + N - 1, last = MIN(Sm, frames - 1), oldSb = Sb;
        printf("\n");

        for (int Sn = Sb; Sn <= Sm && Sn < frames; Sn++) {
            int lost = (Sn == lostFrame);
            if (acked[Sn])
                continue;
            total++;
            printf("| %-4d | %-6d | %-7s | [%d - %d] |\n", step++, Sn, lost ? "LOST" : "ACK", Sb, last);
            if (lost) {
                printf("      -> Packet %d lost. Will be selectively retransmitted later.\n", Sn);
                lostFrame = -1;
            } else {
                acked[Sn] = 1;
                printf("      -> Packet %d delivered and buffered by receiver.\n", Sn);
            }
        }

        while (Sb < frames && acked[Sb])       /* slide over everything that is already ACKed */
            Sb++;
        if (Sb > oldSb && Sb < frames)
            printf("      -> Window base ACKed. Moving window to [%d - %d]\n", Sb, MIN(Sb + N - 1, frames - 1));
    }

    printf("\n+------+--------+---------+------------+\n");
    printf("Frames delivered : %d\n", frames);
    printf("Total transmits  : %d\n", total);
    printf("===========================================\n\n");
    free(acked);
}

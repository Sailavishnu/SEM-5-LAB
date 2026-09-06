#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void goBackN() {
    int N, frames, lostFrame, total = 0;
    int Rn, Sb, Sm, Sn, step = 1;

    printf("Window size (N): ");
    scanf("%d", &N);
    printf("No. of frames  : ");
    scanf("%d", &frames);
    printf("Frame to lose  : ");
    scanf("%d", &lostFrame);

    Rn = 0;
    Sb = 0;
    Sm = N - 1;

    printf("\n================ GO-BACK-N ================\n");
    printf("+------+-------+---------+------------+\n");
    printf("| Step | Frame | Status  | Window     |\n");
    printf("+------+-------+---------+------------+\n");

    while (Rn < frames) {
        for (Sn = Sb; Sn <= Sm && Sn < frames; Sn++) {
            total++;
            if (Sn == lostFrame) {
                printf("| %-4d | %-5d | %-7s | [%d - %d]%*s|\n",
                       step++, Sn, "LOST", Sb, Sm, 4, "");
                printf("      -> Frame %d lost. Sender will resend whole window.\n", Sn);
                lostFrame = -1;
                break;
            } else {
                printf("| %-4d | %-5d | %-7s | [%d - %d]%*s|\n",
                       step++, Sn, "ACK", Sb, Sm, 4, "");
                Rn++;
            }
        }
        if (Rn > Sb) {
            int oldSb = Sb, oldSm = Sm;
            Sm = Sm + (Rn - Sb);
            Sb = Rn;
            printf("      -> Window moved from [%d-%d] to [%d-%d]\n", oldSb, oldSm, Sb, Sm);
        }
    }

    printf("+------+-------+---------+------------+\n");
    printf("Frames delivered : %d\n", frames);
    printf("Total transmits  : %d\n", total);
    printf("=============================================\n\n");
}

void selectiveRepeat() {
    int frames, total = 0;
    int NP, PN, step = 1;
    srand(time(0));

    printf("No. of frames  : ");
    scanf("%d", &frames);

    NP = 0;
    PN = 0;

    printf("\n============ SELECTIVE REPEAT ============\n");
    printf("+------+--------+---------+\n");
    printf("| Step | Packet | Status  |\n");
    printf("+------+--------+---------+\n");

    while (NP < frames) {
        if (NP > PN) PN++;
        total++;

        int ok = rand() % 4 != 0;
        if (ok && PN == NP) {
            printf("| %-4d | %-6d | %-7s |\n", step++, PN, "ACK");
            printf("      -> Packet %d delivered. Moving to next packet.\n", PN);
            NP++;
        } else {
            printf("| %-4d | %-6d | %-7s |\n", step++, PN, "LOST");
            printf("      -> Packet %d lost. Resending same packet only.\n", PN);
        }
    }

    printf("+------+--------+---------+\n");
    printf("Frames delivered : %d\n", frames);
    printf("Total transmits  : %d\n", total);
    printf("===========================================\n\n");
}

int main() {
    int choice;

    do {
        printf("========== SLIDING WINDOW MENU ==========\n");
        printf("  1. Go-Back-N\n");
        printf("  2. Selective Repeat\n");
        printf("  3. Exit\n");
        printf("==========================================\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        printf("\n");

        switch (choice) {
            case 1: goBackN(); break;
            case 2: selectiveRepeat(); break;
            case 3: printf("Exiting...\n"); break;
            default: printf("Invalid choice\n\n");
        }
    } while (choice != 3);

    return 0;
}
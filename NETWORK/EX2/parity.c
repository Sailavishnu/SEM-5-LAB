#include <stdio.h>
#include <string.h>
#include "ex2.h"

/* scheme 0 = even parity, 1 = odd parity */
static int ask_parity_scheme(void) {
    int choice;
    printf("Choose parity scheme -> 1: Even Parity   2: Odd Parity : ");
    scanf("%d", &choice);
    return choice == 2;
}

/* even: total number of 1s must become even; odd: must become odd */
static char parity_bit(int ones, int scheme) {
    return '0' + ((ones % 2) ^ scheme);
}

static int no_error(int total_ones, int scheme) {
    return total_ones % 2 == scheme;
}

/* asks how many bits to flip, then the position of each; records the flipped positions */
static void simulate_multi_error(char *bits, int *flipped, int *nflips) {
    int len = strlen(bits), k, count = 0;
    printf("\nSimulate transmission error(s)\n");
    printf("How many bits do you want to flip? (0 for NO error): ");
    scanf("%d", &k);
    if (k <= 0)
        printf("No error introduced.\n");
    for (int i = 0; i < k; i++) {
        int pos;
        printf("  Enter bit position #%d to flip (0 to %d): ", i + 1, len - 1);
        scanf("%d", &pos);
        if (pos >= 0 && pos < len) {
            bits[pos] = (bits[pos] == '0') ? '1' : '0';
            flipped[count++] = pos;
            printf("    -> Bit at position %d flipped.\n", pos);
        } else
            printf("    -> Invalid position %d, ignored.\n", pos);
    }
    *nflips = count;
}

/* Message is split into 7-bit frames, each with its own parity bit.
   Parity detects only an ODD number of flips inside one frame. */
void parity(void) {
    char input[MAX_STR], msg[MAX_BITS], transmitted[MAX_BITS], received[MAX_BITS];
    char frame_str[9], recovered[MAX_BITS], recovered_str[MAX_STR];
    int flipped[MAX_BITS], nflips = 0;
    int len, scheme, t = 0, frame_no = 1, rlen = 0, error_found = 0, silent = 0;

    printf("\n---- PARITY (7-bit frames) : SENDER SIDE ----\n");
    printf("Enter string message (or b<binary> for binary): ");
    scanf("%s", input);
    scheme = ask_parity_scheme();

    input_to_bits(input, msg);
    len = strlen(msg);
    printf("Original message (m) in binary              : %s\n", msg);
    printf("Message split into 7-bit frames, each followed by its own parity bit:\n");

    for (int i = 0; i < len; i += 7, frame_no++) {
        int fsize = (len - i < 7) ? len - i : 7, ones = 0;
        for (int j = 0; j < fsize; j++) {
            transmitted[t + j] = msg[i + j];
            ones += msg[i + j] == '1';
        }
        transmitted[t + fsize] = parity_bit(ones, scheme);
        copy_bits(frame_str, transmitted + t, fsize + 1);
        printf("  Frame %d (%d data bits) : %s   (parity bit = %c)\n",
               frame_no, fsize, frame_str, transmitted[t + fsize]);
        t += fsize + 1;
    }
    transmitted[t] = '\0';
    printf("Data to be sent (all frames concatenated)   : %s\n", transmitted);

    printf("\n---- PARITY : RECEIVER SIDE ----\n");
    strcpy(received, transmitted);
    simulate_multi_error(received, flipped, &nflips);
    printf("Received data                                : %s\n", received);
    printf("\nNote: a frame's parity only catches an ODD number of bit flips inside\n"
           "that frame. An EVEN number of flips (2, 4, ...) inside the same frame\n"
           "cancels out and the parity check will wrongly say the frame is OK.\n");

    printf("\nChecking each frame:\n");
    frame_no = 1;
    for (int i = 0, pos = 0; i < len; i += 7, frame_no++) {
        int fsize = (len - i < 7) ? len - i : 7, ones = 0, injected = 0;
        for (int j = 0; j < fsize; j++) {
            ones += received[pos + j] == '1';
            recovered[rlen++] = received[pos + j];
        }
        int detected = !no_error(ones + (received[pos + fsize] == '1'), scheme);

        for (int f = 0; f < nflips; f++)          /* how many flips landed in this frame */
            if (flipped[f] >= pos && flipped[f] <= pos + fsize)
                injected++;

        copy_bits(frame_str, received + pos, fsize + 1);
        printf("  Frame %d : %s  -> %s", frame_no, frame_str, detected ? "MISMATCH (error detected)" : "OK");
        if (injected == 0)
            printf("\n");
        else if (injected % 2 == 1)
            printf("   [%d bit(s) flipped in this frame - ODD - correctly DETECTED]\n", injected);
        else {
            printf("   [%d bit(s) flipped in this frame - EVEN - NOT DETECTED, parity still matches!]\n", injected);
            silent = 1;
        }
        error_found |= detected;
        pos += fsize + 1;
    }
    recovered[rlen] = '\0';

    if (error_found) {
        printf("\nResult : ERROR DETECTED\n");
        if (silent)
            printf("(Note: some frame(s) above were also silently corrupted by an EVEN\n"
                   "number of flips and did not trigger a mismatch on their own.)\n");
        return;
    }
    printf("\nResult : NO ERROR DETECTED\n");
    bits_to_str(recovered, recovered_str);
    printf("Recovered message (string)                   : %s\n", recovered_str);
    if (silent)
        printf("WARNING: %d bit(s) were actually flipped during transmission, but every\n"
               "affected frame had an EVEN flip count, so parity could not catch it.\n"
               "The 'recovered' message above may NOT be the original message!\n", nflips);
}

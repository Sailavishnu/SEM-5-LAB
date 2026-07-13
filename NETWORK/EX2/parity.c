/* ============================================================
   Parity Generator and Checker (EVEN PARITY ONLY)
   - Converts each character of a message to ASCII -> Binary
   - Sender  : generates parity bit P using XOR, sends Data = M + P
   - Receiver: checks Data and reports NO ERROR / ERROR DETECTED
   ============================================================ */

#include <stdio.h>
#include <string.h>

#define BITS 8   /* ASCII value represented using 8 bits */

/* ---------------------------------------------------------
   Function: toBinary
   Converts an ASCII integer value into an 8-bit binary array
   (MSB first, stored in bin[0..7])
   --------------------------------------------------------- */
void toBinary(int val, int bin[]) {
    for (int i = BITS - 1; i >= 0; i--) {
        bin[i] = val % 2;
        val = val / 2;
    }
}

/* ---------------------------------------------------------
   Function: xorParity
   XORs all bits in the array together.
   Result = 0  -> even number of 1s
   Result = 1  -> odd number of 1s
   --------------------------------------------------------- */
int xorParity(int bin[], int len) {
    int result = 0;
    for (int i = 0; i < len; i++)
        result ^= bin[i];
    return result;
}

int main() {
    char msg[100];

    printf("Enter message: ");
    fgets(msg, sizeof(msg), stdin);
    msg[strcspn(msg, "\n")] = '\0';   /* remove trailing newline */

    int len = strlen(msg);

    /* ---- Process each character of the message ---- */
    for (int i = 0; i < len; i++) {

        int ascii = (int)msg[i];
        int M[BITS];              /* original message bits */
        toBinary(ascii, M);

        /* ================= SENDER SIDE ================= */

        int ones = xorParity(M, BITS);   /* 0 = even 1s, 1 = odd 1s in M */
        int P;

        /* EVEN PARITY RULE: total 1s (M + P) must be even */
        if (ones == 0)
            P = 0;   /* M already has even 1s, no need to add */
        else
            P = 1;   /* M has odd 1s, add 1 to make total even */

        int data[BITS + 1];    /* Data = M (8 bits) + P (1 bit) */
        for (int j = 0; j < BITS; j++)
            data[j] = M[j];
        data[BITS] = P;

        /* ---- Display sender side details ---- */
        printf("\nChar: %c | ASCII: %d\n", msg[i], ascii);
        printf("Message bits (M) : ");
        for (int j = 0; j < BITS; j++) printf("%d", M[j]);
        printf("\nParity bit (P)   : %d\n", P);
        printf("Data sent (M+P)  : ");
        for (int j = 0; j <= BITS; j++) printf("%d", data[j]);
        printf("\n");

        /* ================ RECEIVER SIDE ================= */

        /* NOTE: To simulate a transmission error, uncomment below
           and flip any bit before checking:
           data[BITS] ^= 1;   */

        int check = xorParity(data, BITS + 1);
        /* check = 0 -> total 1s received is EVEN -> NO ERROR
           check = 1 -> total 1s received is ODD  -> ERROR DETECTED */

        if (check == 0)
            printf("Receiver: even 1s -> NO ERROR\n");
        else
            printf("Receiver: odd 1s  -> ERROR DETECTED\n");
    }

    return 0;
}
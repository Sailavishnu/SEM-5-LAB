#ifndef HAMMING_H
#define HAMMING_H

/*  Position :  ... 11 10  9  8  7  6  5  4  3  2  1
    Type     :  ... D7 D6 D5 P4 D4 D3 D2 P3 D1 P2 P1
    Parity bit Pk sits at position 2^(k-1); all other positions hold data. */

#define MAX 100

extern int  code[MAX];       /* bit value at each position (1-indexed) */
extern int  filled[MAX];     /* is that position's value known yet?    */
extern char label[MAX][4];   /* "D7", "P2" ... label of each position  */

/* common.c */
int  isPowerOf2(int pos);
void assignLabels(int totalLen, int p);
void displayFrame(int totalLen);

/* encode.c */
void generateHammingCode(const char *data, int parityType);

/* decode.c */
void checkReceivedCode(const char *input, int parityType);

#endif

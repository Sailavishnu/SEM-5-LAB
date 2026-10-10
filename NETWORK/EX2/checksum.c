#include <stdio.h>
#include <string.h>
#include "ex2.h"

static unsigned int bits_to_uint(const char *bits, int n) {
    unsigned int v = 0;
    for (int i = 0; i < n; i++)
        v = (v << 1) | (bits[i] - '0');
    return v;
}

static void uint_to_bits(unsigned int v, int n, char *bits) {
    for (int i = n - 1; i >= 0; i--, v >>= 1)
        bits[i] = (v & 1) ? '1' : '0';
    bits[n] = '\0';
}

static unsigned int mask_of(int n) {
    return (n >= 32) ? 0xFFFFFFFFu : (1u << n) - 1;
}

/* 1's complement addition: carry out of the top bit is added back (end-around carry) */
static unsigned int add_1s_complement(unsigned int a, unsigned int b, int n) {
    unsigned int mask = mask_of(n), sum = a + b;
    if (sum > mask)
        sum = (sum & mask) + 1;
    return sum & mask;
}

/* prints every n-bit block and returns their 1's complement sum */
static unsigned int sum_blocks(const char *bits, int nblocks, int n) {
    unsigned int sum = 0;
    char block[33];
    for (int i = 0; i < nblocks; i++) {
        copy_bits(block, bits + i * n, n);
        printf("  Block %d : %s\n", i + 1, block);
        sum = add_1s_complement(sum, bits_to_uint(block, n), n);
    }
    return sum;
}

void checksum_technique(void) {
    char input[MAX_STR], bits[MAX_BITS], transmitted[MAX_BITS], received[MAX_BITS];
    char sum_bits[33], chk_bits[33], recovered_bits[MAX_BITS], recovered_str[MAX_STR];
    int n, len;
    unsigned int sum, checksum;

    printf("\n---- CHECKSUM : SENDER SIDE ----\n");
    printf("Enter string message (or b<binary> for binary): ");
    scanf("%s", input);
    input_to_bits(input, bits);

    printf("Enter block size n (0 for default n = 8): ");
    scanf("%d", &n);
    if (n <= 0)
        n = 8;

    len = strlen(bits);
    memset(bits + len, '0', (n - len % n) % n);   /* pad with zeros to a multiple of n */
    len += (n - len % n) % n;
    bits[len] = '\0';
    printf("Data (padded to multiple of %d)   : %s\n", n, bits);

    printf("Blocks:\n");
    sum = sum_blocks(bits, len / n, n);
    uint_to_bits(sum, n, sum_bits);
    printf("Sum of all blocks                : %s\n", sum_bits);

    checksum = ~sum & mask_of(n);
    uint_to_bits(checksum, n, chk_bits);
    printf("Checksum (complement of sum)     : %s\n", chk_bits);

    strcpy(transmitted, bits);
    strcat(transmitted, chk_bits);
    printf("Data to be transmitted (data+chk) : %s\n", transmitted);

    printf("\n---- CHECKSUM : RECEIVER SIDE ----\n");
    strcpy(received, transmitted);
    simulate_error(received);
    printf("Received data                    : %s\n", received);

    printf("Blocks (data blocks + checksum block):\n");
    sum = sum_blocks(received, strlen(received) / n, n);
    uint_to_bits(sum, n, sum_bits);
    printf("Sum of all blocks                : %s\n", sum_bits);

    checksum = ~sum & mask_of(n);
    uint_to_bits(checksum, n, chk_bits);
    printf("Complement of sum (result)       : %s\n", chk_bits);

    if (checksum != 0) {
        printf("Reason : Complement of sum is NOT all zeros (%s) -> mismatch!\n", chk_bits);
        printf("Result : ERROR DETECTED\n");
        return;
    }
    printf("Reason : Complement of sum is all zeros -> as expected.\n");
    printf("Result : NO ERROR DETECTED\n");
    copy_bits(recovered_bits, received, len);      /* original data length, before checksum block */
    bits_to_str(recovered_bits, recovered_str);
    printf("Recovered message (string)        : %s\n", recovered_str);
}

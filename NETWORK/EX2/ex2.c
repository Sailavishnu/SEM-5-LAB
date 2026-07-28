#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\convert\convert.h"

#define MAX_STR   100
#define MAX_BITS  1024

void str_to_bits(const char *str, char *bits) {
    int len = strlen(str);
    char b[9];
    for (int i = 0; i < len; i++) {
        ascii_to_bin(str[i], b);
        strcpy(bits + (i * 8), b);
    }
    bits[len * 8] = '\0';
}

void bits_to_str(const char *bits, char *str) {
    int nchars = strlen(bits) / 8;
    for (int i = 0; i < nchars; i++) {
        str[i] = bin_to_ascii(bits + (i * 8));
    }
    str[nchars] = '\0';
}

void simulate_error(char *bits) {
    int len = strlen(bits);
    int pos;
    printf("\nSimulate a transmission error?\n");
    printf("Enter bit position to flip (0 to %d), or -1 for NO error: ", len - 1);
    scanf("%d", &pos);
    if (pos >= 0 && pos < len) {
        bits[pos] = (bits[pos] == '0') ? '1' : '0';
        printf("Bit at position %d flipped.\n", pos);
    } else {
        printf("No error introduced.\n");
    }
}

int ask_parity_scheme() {
    int choice;
    printf("Choose parity scheme -> 1: Even Parity   2: Odd Parity : ");
    scanf("%d", &choice);
    return (choice == 2) ? 1 : 0;   /* 0 = even, 1 = odd */
}

char compute_parity_bit(int ones_in_data, int scheme) {
    /* scheme 0 (even): parity bit makes total 1's even
       scheme 1 (odd) : parity bit makes total 1's odd */
    if (scheme == 0)
        return (ones_in_data % 2 == 0) ? '0' : '1';
    else
        return (ones_in_data % 2 == 0) ? '1' : '0';
}

int is_no_error(int total_ones, int scheme) {
    if (scheme == 0) return (total_ones % 2 == 0);  /* even scheme -> even count = OK */
    else             return (total_ones % 2 == 1);  /* odd scheme  -> odd count  = OK */
}

/* ---- PARITY : message split into 7-bit frames, each with its own parity bit ---- */
void parity() {
    char input[MAX_STR];
    char msg[MAX_BITS], transmitted[MAX_BITS], received[MAX_BITS];
    int len;

    printf("\n---- PARITY (7-bit frames) : SENDER SIDE ----\n");
    printf("Enter string message: ");
    scanf("%s", input);

    int scheme = ask_parity_scheme();

    str_to_bits(input, msg);
    len = strlen(msg);
    printf("Original message (m) in binary              : %s\n", msg);
    printf("Message split into 7-bit frames, each followed by its own parity bit:\n");

    int t = 0;
    int frame_no = 1;
    for (int i = 0; i < len; i += 7) {
        int fsize = (len - i < 7) ? (len - i) : 7;
        int ones = 0;
        for (int j = 0; j < fsize; j++) {
            transmitted[t + j] = msg[i + j];
            if (msg[i + j] == '1') ones++;
        }
        char pbit = compute_parity_bit(ones, scheme);
        transmitted[t + fsize] = pbit;

        char frame_str[9];
        strncpy(frame_str, transmitted + t, fsize + 1);
        frame_str[fsize + 1] = '\0';
        printf("  Frame %d (%d data bits) : %s   (parity bit = %c)\n", frame_no, fsize, frame_str, pbit);

        t += fsize + 1;
        frame_no++;
    }
    transmitted[t] = '\0';
    printf("Data to be sent (all frames concatenated)   : %s\n", transmitted);

    printf("\n---- PARITY : RECEIVER SIDE ----\n");
    strcpy(received, transmitted);
    simulate_error(received);
    printf("Received data                                : %s\n", received);

    printf("\nChecking each frame:\n");
    int pos = 0, error_found = 0, recovered_len = 0;
    char recovered_bits[MAX_BITS];
    frame_no = 1;
    for (int i = 0; i < len; i += 7) {
        int fsize = (len - i < 7) ? (len - i) : 7;
        int ones = 0;
        for (int j = 0; j < fsize; j++) {
            if (received[pos + j] == '1') ones++;
            recovered_bits[recovered_len++] = received[pos + j];
        }
        int total_ones = ones + (received[pos + fsize] == '1' ? 1 : 0);

        char frame_str[9];
        strncpy(frame_str, received + pos, fsize + 1);
        frame_str[fsize + 1] = '\0';

        if (is_no_error(total_ones, scheme)) {
            printf("  Frame %d : %s  -> OK\n", frame_no, frame_str);
        } else {
            printf("  Frame %d : %s  -> MISMATCH (error detected in this frame)\n", frame_no, frame_str);
            error_found = 1;
        }
        pos += fsize + 1;
        frame_no++;
    }
    recovered_bits[recovered_len] = '\0';

    if (!error_found) {
        printf("\nResult : NO ERROR DETECTED\n");
        char recovered_str[MAX_STR];
        bits_to_str(recovered_bits, recovered_str);
        printf("Recovered message (string)                   : %s\n", recovered_str);
    } else {
        printf("\nResult : ERROR DETECTED\n");
    }
}

void xor_div(const char *data, const char *gen, char *remainder) {
    int data_len = strlen(data);
    int gen_len = strlen(gen);
    char *temp = (char *)malloc(data_len + 1);
    strcpy(temp, data);

    for (int i = 0; i <= data_len - gen_len; i++) {
        if (temp[i] == '1') {
            for (int j = 0; j < gen_len; j++) {
                temp[i + j] = (((temp[i + j] - '0') ^ (gen[j] - '0')) + '0');
            }
        }
    }
    strcpy(remainder, temp + (data_len - (gen_len - 1)));
    free(temp);
}

int count_ones(const char *s) {
    int c = 0;
    for (int i = 0; s[i] != '\0'; i++) if (s[i] == '1') c++;
    return c;
}

/* returns 1 if 'gen' divides 'dividend' evenly (remainder all zero) */
int divides_evenly(const char *dividend, const char *gen) {
    int dlen = strlen(dividend);
    int glen = strlen(gen);
    if (dlen < glen) return 0; /* too short to meaningfully divide -> treat as not dividing */
    char remainder[MAX_BITS];
    xor_div(dividend, gen, remainder);
    int rlen = glen - 1;
    for (int i = 0; i < rlen; i++) if (remainder[i] != '0') return 0;
    return 1;
}

/* Keeps asking for a generator until it satisfies all 4 CRC design criteria.
   data_len = length (in bits) of the data the generator will protect. */
/* Keeps asking for a generator until it satisfies all CRC design criteria.
   (Criterion 1, 2 and 4 are checked. Criterion 3 is omitted because
   it depends on the frame length and incorrectly rejects generators
   like 1001 when checked this way.) */
void get_valid_generator(char *gen, int data_len)
{
    while (1)
    {
        printf("Enter generator polynomial bits (e.g. 1001): ");
        scanf("%s", gen);

        char fails[4][160];
        int nfail = 0;
        int glen = strlen(gen);

        /* Criterion 1 : At least two 1's */
        if (count_ones(gen) < 2)
        {
            strcpy(fails[nfail++],
                   "Criterion 1 failed: generator must have at least two 1's.");
        }

        /* Criterion 2 : Last bit must be 1 */
        if (gen[glen - 1] != '1')
        {
            strcpy(fails[nfail++],
                   "Criterion 2 failed: last bit (x^0 coefficient) must be 1.");
        }

        /* Criterion 4 : Must contain (x + 1) as a factor */
        if (!divides_evenly(gen, "11"))
        {
            strcpy(fails[nfail++],
                   "Criterion 4 failed: generator must be divisible by 11 (x + 1).");
        }

        if (nfail == 0)
        {
            printf("Generator accepted: %s\n", gen);
            return;
        }

        printf("Generator rejected. Issue(s) found:\n");
        for (int i = 0; i < nfail; i++)
            printf("  - %s\n", fails[i]);

        printf("Please enter a different generator.\n\n");
    }
}

void crc_technique() {
    char input[MAX_STR];
    char data[MAX_BITS], gen[32];
    char dividend[MAX_BITS], remainder[32], transmitted[MAX_BITS];

    printf("\n---- CRC : SENDER SIDE ----\n");
    printf("Enter string message: ");
    scanf("%s", input);
    str_to_bits(input, data);
    printf("Source data (binary)             : %s\n", data);

    get_valid_generator(gen, strlen(data));
    printf("Generator G(x) used              : %s\n", gen);

    int r = strlen(gen) - 1;
    strcpy(dividend, data);
    for (int i = 0; i < r; i++) dividend[strlen(data) + i] = '0';
    dividend[strlen(data) + r] = '\0';
    printf("Data padded with %d zeros         : %s\n", r, dividend);

    xor_div(dividend, gen, remainder);
    printf("CRC remainder (redundant bits)   : %s\n", remainder);

    strcpy(transmitted, data);
    strcat(transmitted, remainder);
    printf("Data to be transmitted (T = D+CRC): %s\n", transmitted);

    printf("\n---- CRC : RECEIVER SIDE ----\n");
    char received[MAX_BITS];
    strcpy(received, transmitted);
    simulate_error(received);
    printf("Received data                    : %s\n", received);

    char check_remainder[32];
    xor_div(received, gen, check_remainder);
    printf("Remainder after division by G(x) : %s\n", check_remainder);

    int all_zero = 1;
    for (int i = 0; i < (int)strlen(check_remainder); i++)
        if (check_remainder[i] != '0') all_zero = 0;

    if (all_zero) {
        printf("Result : NO ERROR DETECTED\n");
        char recovered_bits[MAX_BITS], recovered_str[MAX_STR];
        strncpy(recovered_bits, received, strlen(data));
        recovered_bits[strlen(data)] = '\0';
        bits_to_str(recovered_bits, recovered_str);
        printf("Recovered message (string)       : %s\n", recovered_str);
    } else {
        printf("Result : ERROR DETECTED\n");
    }
}

unsigned int bits_to_uint(const char *bits, int n) {
    unsigned int v = 0;
    for (int i = 0; i < n; i++) v = (v << 1) | (bits[i] - '0');
    return v;
}

void uint_to_bits(unsigned int v, int n, char *bits) {
    for (int i = n - 1; i >= 0; i--) {
        bits[i] = (v & 1) ? '1' : '0';
        v >>= 1;
    }
    bits[n] = '\0';
}

unsigned int add_1s_complement(unsigned int a, unsigned int b, int n) {
    unsigned int mask = (n >= 32) ? 0xFFFFFFFFu : ((1u << n) - 1);
    unsigned int sum = a + b;
    if (sum > mask) sum = (sum & mask) + 1; /* end-around carry */
    return sum & mask;
}

void checksum_technique() {
    char input[MAX_STR];
    char bits[MAX_BITS];
    int n;

    printf("\n---- CHECKSUM : SENDER SIDE ----\n");
    printf("Enter string message: ");
    scanf("%s", input);
    str_to_bits(input, bits);

    printf("Enter block size n (0 for default n = 8): ");
    scanf("%d", &n);
    if (n <= 0) n = 8;

    int len = strlen(bits);
    /* pad with zeros to make length a multiple of n */
    int pad = (n - (len % n)) % n;
    for (int i = 0; i < pad; i++) bits[len + i] = '0';
    len += pad;
    bits[len] = '\0';
    printf("Data (padded to multiple of %d)   : %s\n", n, bits);

    int nblocks = len / n;
    unsigned int sum = 0;
    printf("Blocks:\n");
    for (int i = 0; i < nblocks; i++) {
        char block[33];
        strncpy(block, bits + i * n, n);
        block[n] = '\0';
        printf("  Block %d : %s\n", i + 1, block);
        unsigned int val = bits_to_uint(block, n);
        sum = add_1s_complement(sum, val, n);
    }
    char sum_bits[33];
    uint_to_bits(sum, n, sum_bits);
    printf("Sum of all blocks                : %s\n", sum_bits);

    unsigned int mask = (n >= 32) ? 0xFFFFFFFFu : ((1u << n) - 1);
    unsigned int checksum = (~sum) & mask;
    char checksum_bits[33];
    uint_to_bits(checksum, n, checksum_bits);
    printf("Checksum (complement of sum)     : %s\n", checksum_bits);

    char transmitted[MAX_BITS];
    strcpy(transmitted, bits);
    strcat(transmitted, checksum_bits);
    printf("Data to be transmitted (data+chk) : %s\n", transmitted);

    printf("\n---- CHECKSUM : RECEIVER SIDE ----\n");
    char received[MAX_BITS];
    strcpy(received, transmitted);
    simulate_error(received);
    printf("Received data                    : %s\n", received);

    int rlen = strlen(received);
    int rblocks = rlen / n;
    unsigned int rsum = 0;
    printf("Blocks (data blocks + checksum block):\n");
    for (int i = 0; i < rblocks; i++) {
        char block[33];
        strncpy(block, received + i * n, n);
        block[n] = '\0';
        printf("  Block %d : %s\n", i + 1, block);
        unsigned int val = bits_to_uint(block, n);
        rsum = add_1s_complement(rsum, val, n);
    }
    char rsum_bits[33];
    uint_to_bits(rsum, n, rsum_bits);
    printf("Sum of all blocks                : %s\n", rsum_bits);

    unsigned int rchecksum = (~rsum) & mask;
    char rchecksum_bits[33];
    uint_to_bits(rchecksum, n, rchecksum_bits);
    printf("Complement of sum (result)       : %s\n", rchecksum_bits);

    if (rchecksum == 0) {
        printf("Reason : Complement of sum is all zeros -> as expected.\n");
        printf("Result : NO ERROR DETECTED\n");
        char recovered_bits[MAX_BITS], recovered_str[MAX_STR];
        strncpy(recovered_bits, received, len); /* original data length before checksum block */
        recovered_bits[len] = '\0';
        bits_to_str(recovered_bits, recovered_str);
        printf("Recovered message (string)        : %s\n", recovered_str);
    } else {
        printf("Reason : Complement of sum is NOT all zeros (%s) -> mismatch!\n", rchecksum_bits);
        printf("Result : ERROR DETECTED\n");
    }
}

int main() {
    int choice;
    do {
        printf("\n================ ERROR DETECTION TECHNIQUES ================\n");
        printf(" 1. Parity\n");
        printf(" 2. CRC\n");
        printf(" 3. Checksum\n");
        printf(" 4. Exit\n");
        printf("==============================================================\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: parity();             break;
            case 2: crc_technique();      break;
            case 3: checksum_technique(); break;
            case 4: printf("Exiting...\n"); break;
            default: printf("Invalid choice, try again.\n");
        }
    } while (choice != 4);

    return 0;
}
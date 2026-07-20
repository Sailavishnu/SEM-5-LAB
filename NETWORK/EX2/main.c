/* ============================================================
   NETWORK LAB - ERROR DETECTION TECHNIQUES (Menu Driven)
   Covers : 1) 1D Parity  2) 2D Parity  3) CRC  4) Checksum
   Uses convert.h / convert.c for ASCII <-> Binary conversion
   Input  : plain string (e.g. "Hi")
   Output : intermediate binary shown at every step, final
            result shown as string (where applicable) + error
            status (No Error / Error Detected)
   ============================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "..\convert\convert.h"

#define MAX_STR   100
#define MAX_BITS  1024

/* ---------------------------------------------------------
   Helper : string -> concatenated bit stream (uses convert.h)
   --------------------------------------------------------- */
void str_to_bits(const char *str, char *bits) {
    int len = strlen(str);
    char b[9];
    for (int i = 0; i < len; i++) {
        ascii_to_bin(str[i], b);
        strcpy(bits + (i * 8), b);
    }
    bits[len * 8] = '\0';
}

/* ---------------------------------------------------------
   Helper : bit stream (multiple of 8) -> string (uses convert.h)
   --------------------------------------------------------- */
void bits_to_str(const char *bits, char *str) {
    int nchars = strlen(bits) / 8;
    for (int i = 0; i < nchars; i++) {
        str[i] = bin_to_ascii(bits + (i * 8));
    }
    str[nchars] = '\0';
}

/* ---------------------------------------------------------
   Helper : let user optionally flip one bit to simulate error
   --------------------------------------------------------- */
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

/* ---------------------------------------------------------
   Helper : parity scheme (0 = EVEN, 1 = ODD)
   EVEN scheme -> total count of 1's (data+parity) must be EVEN  for No Error
   ODD  scheme -> total count of 1's (data+parity) must be ODD   for No Error
   --------------------------------------------------------- */
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

/* ============================================================
   1) 1D PARITY  (user chooses EVEN or ODD parity)
   ============================================================ */
void parity_1D() {
    char input[MAX_STR];
    char msg[MAX_BITS], data_to_send[MAX_BITS], received[MAX_BITS];
    int ones = 0, len;

    printf("\n---- 1D PARITY : SENDER SIDE ----\n");
    printf("Enter string message: ");
    scanf("%s", input);

    int scheme = ask_parity_scheme();   /* 0 = even, 1 = odd */

    str_to_bits(input, msg);
    len = strlen(msg);
    printf("Original message  (m) in binary : %s\n", msg);

    for (int i = 0; i < len; i++) if (msg[i] == '1') ones++;
    char parity = compute_parity_bit(ones, scheme);
    printf("Parity bit (p) [%s parity]      : %c\n", scheme == 0 ? "EVEN" : "ODD", parity);

    strcpy(data_to_send, msg);
    data_to_send[len] = parity;
    data_to_send[len + 1] = '\0';
    printf("Data to be sent    (m + p)      : %s\n", data_to_send);

    printf("\n---- 1D PARITY : RECEIVER SIDE ----\n");
    strcpy(received, data_to_send);
    simulate_error(received);
    printf("Received data                   : %s\n", received);

    int rlen = strlen(received);
    int rones = 0;
    for (int i = 0; i < rlen; i++) if (received[i] == '1') rones++;
    printf("Total 1's in received data       : %d (%s)\n", rones, (rones % 2 == 0) ? "even" : "odd");

    if (is_no_error(rones, scheme)) {
        printf("Reason  : No. of 1's is %s, which matches %s parity -> as expected.\n",
               (rones % 2 == 0) ? "even" : "odd", scheme == 0 ? "EVEN" : "ODD");
        printf("Result : NO ERROR DETECTED\n");
        char recovered_bits[MAX_BITS], recovered_str[MAX_STR];
        strncpy(recovered_bits, received, rlen - 1);
        recovered_bits[rlen - 1] = '\0';
        bits_to_str(recovered_bits, recovered_str);
        printf("Recovered message (string)      : %s\n", recovered_str);
    } else {
        printf("Reason  : No. of 1's is %s, but %s parity expects %s count -> mismatch!\n",
               (rones % 2 == 0) ? "even" : "odd",
               scheme == 0 ? "EVEN" : "ODD",
               scheme == 0 ? "an EVEN" : "an ODD");
        printf("Result : ERROR DETECTED\n");
    }
}

/* ============================================================
   2) 2D PARITY  (row parity + column parity, even parity)
      Each character (8 bits) is treated as one row.
   ============================================================ */
void parity_2D() {
    char input[MAX_STR];
    char bits[MAX_BITS];
    int rows, cols = 8;

    printf("\n---- 2D PARITY : SENDER SIDE ----\n");
    printf("Enter string message: ");
    scanf("%s", input);

    int scheme = ask_parity_scheme();   /* 0 = even, 1 = odd */

    str_to_bits(input, bits);
    rows = strlen(bits) / cols;

    char matrix[50][9]; /* up to 50 rows(chars), 8 data bits + row parity */
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++)
            matrix[i][j] = bits[i * cols + j];
    }

    /* row parity -> column index 'cols' */
    for (int i = 0; i < rows; i++) {
        int ones = 0;
        for (int j = 0; j < cols; j++) if (matrix[i][j] == '1') ones++;
        matrix[i][cols] = compute_parity_bit(ones, scheme);
    }

    /* column parity -> extra row 'rows', including corner bit */
    char col_parity[9];
    for (int j = 0; j <= cols; j++) {
        int ones = 0;
        for (int i = 0; i < rows; i++) ones += (matrix[i][j] == '1');
        col_parity[j] = compute_parity_bit(ones, scheme);
    }
    col_parity[cols + 1] = '\0';

    printf("2D Matrix with Row & Column Parity (last col = row parity, last row = col parity):\n");
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j <= cols; j++) printf("%c", matrix[i][j]);
        printf("\n");
    }
    printf("%s   <-- column parity row (corner = parity of parities)\n", col_parity);

    /* build transmitted stream: all data rows + row parity, then col parity row */
    char transmitted[MAX_BITS];
    int t = 0;
    for (int i = 0; i < rows; i++)
        for (int j = 0; j <= cols; j++) transmitted[t++] = matrix[i][j];
    for (int j = 0; j <= cols; j++) transmitted[t++] = col_parity[j];
    transmitted[t] = '\0';
    printf("Data to be sent (flattened)      : %s\n", transmitted);

    printf("\n---- 2D PARITY : RECEIVER SIDE ----\n");
    char received[MAX_BITS];
    strcpy(received, transmitted);
    simulate_error(received);
    printf("Received data                    : %s\n", received);

    /* rebuild matrix from received stream */
    char rmatrix[51][9];
    int idx = 0;
    for (int i = 0; i <= rows; i++)
        for (int j = 0; j <= cols; j++) rmatrix[i][j] = received[idx++];

    int error_found = 0;
    /* check row parities */
    for (int i = 0; i < rows; i++) {
        int ones = 0;
        for (int j = 0; j <= cols; j++) ones += (rmatrix[i][j] == '1');
        if (!is_no_error(ones, scheme)) {
            error_found = 1;
            printf("Row %d mismatch    : No. of 1's is %s (%d), but %s parity expects %s count.\n",
                   i, (ones % 2 == 0) ? "even" : "odd", ones,
                   scheme == 0 ? "EVEN" : "ODD", scheme == 0 ? "an EVEN" : "an ODD");
        }
    }
    /* check column parities (including the parity row itself) */
    for (int j = 0; j <= cols; j++) {
        int ones = 0;
        for (int i = 0; i <= rows; i++) ones += (rmatrix[i][j] == '1');
        if (!is_no_error(ones, scheme)) {
            error_found = 1;
            printf("Column %d mismatch : No. of 1's is %s (%d), but %s parity expects %s count.\n",
                   j, (ones % 2 == 0) ? "even" : "odd", ones,
                   scheme == 0 ? "EVEN" : "ODD", scheme == 0 ? "an EVEN" : "an ODD");
        }
    }

    if (!error_found) {
        printf("Reason : Every row and column has the correct %s count of 1's -> as expected.\n",
               scheme == 0 ? "EVEN" : "ODD");
        printf("Result : NO ERROR DETECTED\n");
        char recovered_bits[MAX_BITS], recovered_str[MAX_STR];
        int p = 0;
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++) recovered_bits[p++] = rmatrix[i][j];
        recovered_bits[p] = '\0';
        bits_to_str(recovered_bits, recovered_str);
        printf("Recovered message (string)       : %s\n", recovered_str);
    } else {
        printf("Result : ERROR DETECTED\n");
    }
}

/* ============================================================
   3) CRC  (Cyclic Redundancy Check - modulo 2 binary division)
   ============================================================ */
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

void crc_technique() {
    char input[MAX_STR];
    char data[MAX_BITS], gen[32];
    char dividend[MAX_BITS], remainder[32], transmitted[MAX_BITS];

    printf("\n---- CRC : SENDER SIDE ----\n");
    printf("Enter string message: ");
    scanf("%s", input);
    str_to_bits(input, data);
    printf("Source data (binary)             : %s\n", data);

    printf("Enter generator polynomial bits (e.g. 1001), or press 0 for default 1001: ");
    scanf("%s", gen);
    if (strcmp(gen, "0") == 0) strcpy(gen, "1001");
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

/* ============================================================
   4) CHECKSUM  (1's complement addition, default block n = 8)
   ============================================================ */
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

/* ============================================================
   MAIN MENU
   ============================================================ */
int main() {
    int choice;
    do {
        printf("\n================ ERROR DETECTION TECHNIQUES ================\n");
        printf(" 1. 1D Parity\n");
        printf(" 2. 2D Parity\n");
        printf(" 3. CRC\n");
        printf(" 4. Checksum\n");
        printf(" 5. Exit\n");
        printf("==============================================================\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: parity_1D();          break;
            case 2: parity_2D();          break;
            case 3: crc_technique();      break;
            case 4: checksum_technique(); break;
            case 5: printf("Exiting...\n"); break;
            default: printf("Invalid choice, try again.\n");
        }
    } while (choice != 5);

    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ex2.h"

/* binary (XOR) long division; remainder = last (len(gen) - 1) bits */
static void xor_div(const char *data, const char *gen, char *remainder) {
    int data_len = strlen(data), gen_len = strlen(gen);
    char *temp = malloc(data_len + 1);
    strcpy(temp, data);
    for (int i = 0; i <= data_len - gen_len; i++) {
        if (temp[i] == '1')
            for (int j = 0; j < gen_len; j++)
                temp[i + j] = (temp[i + j] == gen[j]) ? '0' : '1';
    }
    strcpy(remainder, temp + data_len - (gen_len - 1));
    free(temp);
}

static int count_ones(const char *s) {
    int c = 0;
    for (int i = 0; s[i]; i++)
        c += s[i] == '1';
    return c;
}

/* 1 if gen divides dividend with an all-zero remainder */
static int divides_evenly(const char *dividend, const char *gen) {
    char remainder[MAX_BITS];
    if (strlen(dividend) < strlen(gen))
        return 0;
    xor_div(dividend, gen, remainder);
    return strspn(remainder, "0") == strlen(gen) - 1;
}

/* keeps asking until the generator satisfies all 4 CRC design criteria */
static void get_valid_generator(char *gen, int data_len) {
    while (1) {
        char fails[4][160];
        int nfail = 0, glen, n, bad_t = -1;

        printf("Enter generator polynomial bits (e.g. 1001), or press 0 for default 1001: ");
        scanf("%s", gen);
        if (strcmp(gen, "0") == 0)
            strcpy(gen, "1001");
        glen = strlen(gen);
        n = data_len + glen - 1;               /* length of data + redundancy stream */

        if (count_ones(gen) < 2)
            strcpy(fails[nfail++], "Criterion 1 failed: generator must have at least two terms (at least two 1's).");
        if (gen[glen - 1] != '1')
            strcpy(fails[nfail++], "Criterion 2 failed: coefficient of x^0 (the last bit) must be 1.");
        if (!divides_evenly(gen, "11"))
            strcpy(fails[nfail++], "Criterion 4 failed: generator must have the factor (x + 1), i.e. it must be evenly divisible by 11.");

        for (int t = 2; t < n && bad_t < 0; t++) {     /* G(x) must not divide x^t + 1 */
            char pattern[MAX_BITS + 8];
            if (t + 1 < glen)
                continue;
            memset(pattern, '0', t + 1);
            pattern[0] = pattern[t] = '1';
            pattern[t + 1] = '\0';
            if (divides_evenly(pattern, gen))
                bad_t = t;
        }
        if (bad_t >= 0)
            sprintf(fails[nfail++], "Criterion 3 failed: generator divides x^%d + 1, which it should not.", bad_t);

        if (nfail == 0) {
            printf("Generator accepted: %s\n", gen);
            return;
        }
        printf("Generator rejected. Issue(s) found:\n");
        for (int i = 0; i < nfail; i++)
            printf("  - %s\n", fails[i]);
        printf("Please enter a different generator.\n");
    }
}

void crc_technique(void) {
    char input[MAX_STR], data[MAX_BITS], gen[32], dividend[MAX_BITS], remainder[32];
    char transmitted[MAX_BITS], received[MAX_BITS], check[32], recovered_bits[MAX_BITS], recovered_str[MAX_STR];
    int dlen, r;

    printf("\n---- CRC : SENDER SIDE ----\n");
    printf("Enter string message (or b<binary> for binary): ");
    scanf("%s", input);
    input_to_bits(input, data);
    dlen = strlen(data);
    printf("Source data (binary)             : %s\n", data);

    get_valid_generator(gen, dlen);
    printf("Generator G(x) used              : %s\n", gen);

    r = strlen(gen) - 1;                        /* append r zeros, divide, keep remainder */
    strcpy(dividend, data);
    memset(dividend + dlen, '0', r);
    dividend[dlen + r] = '\0';
    printf("Data padded with %d zeros         : %s\n", r, dividend);

    xor_div(dividend, gen, remainder);
    printf("CRC remainder (redundant bits)   : %s\n", remainder);

    strcpy(transmitted, data);
    strcat(transmitted, remainder);
    printf("Data to be transmitted (T = D+CRC): %s\n", transmitted);

    printf("\n---- CRC : RECEIVER SIDE ----\n");
    strcpy(received, transmitted);
    simulate_error(received);
    printf("Received data                    : %s\n", received);

    xor_div(received, gen, check);
    printf("Remainder after division by G(x) : %s\n", check);

    if (strspn(check, "0") != strlen(check)) {
        printf("Result : ERROR DETECTED\n");
        return;
    }
    printf("Result : NO ERROR DETECTED\n");
    copy_bits(recovered_bits, received, dlen);
    bits_to_str(recovered_bits, recovered_str);
    printf("Recovered message (string)       : %s\n", recovered_str);
}

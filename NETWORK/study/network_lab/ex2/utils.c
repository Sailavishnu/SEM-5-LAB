#include <stdio.h>
#include <string.h>
#include "../convert/convert.h"
#include "ex2.h"

static void str_to_bits(const char *str, char *bits) {
    int len = strlen(str);
    for (int i = 0; i < len; i++)
        ascii_to_bin(str[i], bits + i * 8);
    bits[len * 8] = '\0';
}

/* "b1011" -> binary input (b prefix), anything else -> text converted to ASCII bits */
void input_to_bits(const char *input, char *bits) {
    if (input[0] == 'b' && (input[1] == '0' || input[1] == '1'))
        strcpy(bits, input + 1);
    else
        str_to_bits(input, bits);
}

void bits_to_str(const char *bits, char *str) {
    int nchars = strlen(bits) / 8;
    for (int i = 0; i < nchars; i++)
        str[i] = bin_to_ascii(bits + i * 8);
    str[nchars] = '\0';
}

/* copy n characters and terminate the string */
void copy_bits(char *dst, const char *src, int n) {
    strncpy(dst, src, n);
    dst[n] = '\0';
}

void simulate_error(char *bits) {
    int len = strlen(bits), pos;
    printf("\nSimulate a transmission error?\n");
    printf("Enter bit position to flip (0 to %d), or -1 for NO error: ", len - 1);
    scanf("%d", &pos);
    if (pos >= 0 && pos < len) {
        bits[pos] = (bits[pos] == '0') ? '1' : '0';
        printf("Bit at position %d flipped.\n", pos);
    } else
        printf("No error introduced.\n");
}

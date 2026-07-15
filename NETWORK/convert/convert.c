#include "convert.h"
#include <string.h>

void ascii_to_bin(char ascii, char *bin_str) {
        for (int i = 7; i >= 0; i--) {
                bin_str[7 - i] = (ascii & (1 << i)) ? '1' : '0';
        }
        bin_str[8] = '\0';
}

char bin_to_ascii(const char *bin_str) {
        char ascii = 0;
        for (int i = 0; i < 8; i++) {
                if (bin_str[i] == '1') {
                        ascii |= (1 << (7 - i));
                }
        }
        return ascii;
}

void str_to_ascii(const char *str, int *ascii_arr, int *len) {
        *len = strlen(str);
        for (int i = 0; i < *len; i++) {
                ascii_arr[i] = (int)str[i];
        }
}

void ascii_to_str(const int *ascii_arr, int len, char *str) {
        for (int i = 0; i < len; i++) {
                str[i] = (char)ascii_arr[i];
        }
        str[len] = '\0';
}

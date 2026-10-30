#include <string.h>
#include "convert.h"

void ascii_to_bin(char ascii, char *bin_str) {
    for (int i = 0; i < 8; i++)
        bin_str[i] = (ascii & (1 << (7 - i))) ? '1' : '0';
    bin_str[8] = '\0';
}

char bin_to_ascii(const char *bin_str) {
    int value = 0;
    for (int i = 0; i < 8; i++)
        value = (value << 1) | (bin_str[i] == '1');
    return (char)value;
}

void str_to_ascii(const char *str, int *ascii_arr, int *len) {
    *len = strlen(str);
    for (int i = 0; i < *len; i++)
        ascii_arr[i] = (int)str[i];
}

void ascii_to_str(const int *ascii_arr, int len, char *str) {
    for (int i = 0; i < len; i++)
        str[i] = (char)ascii_arr[i];
    str[len] = '\0';
}

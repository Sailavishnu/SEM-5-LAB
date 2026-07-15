#ifndef CONVERT_H
#define CONVERT_H

/* 1. Converts an ASCII char to an 8-bit binary string (e.g., 'A' -> "01000001") */
void ascii_to_bin(char ascii, char *bin_str);

/* 2. Converts an 8-bit binary string back to an ASCII char (e.g., "01000001" -> 'A') */
char bin_to_ascii(const char *bin_str);

/* 3. Converts a full string to an array of ASCII numbers (e.g., "Hi" -> {72, 105}) */
void str_to_ascii(const char *str, int *ascii_arr, int *len);

/* 4. Converts an array of ASCII numbers back to a string (e.g., {72, 105} -> "Hi") */
void ascii_to_str(const int *ascii_arr, int len, char *str);

#endif
#ifndef CONVERT_H
#define CONVERT_H

void ascii_to_bin(char ascii, char *bin_str);                 /* 'A' -> "01000001"  */
char bin_to_ascii(const char *bin_str);                       /* "01000001" -> 'A'  */
void str_to_ascii(const char *str, int *ascii_arr, int *len); /* string -> ASCII ints */
void ascii_to_str(const int *ascii_arr, int len, char *str);  /* ASCII ints -> string */

#endif










                







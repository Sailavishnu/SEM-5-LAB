#ifndef EX2_H
#define EX2_H

#define MAX_STR  100
#define MAX_BITS 1024

/* shared helpers (utils.c) */
void input_to_bits(const char *input, char *bits);
void bits_to_str(const char *bits, char *str);
void copy_bits(char *dst, const char *src, int n);
void simulate_error(char *bits);

/* techniques */
void parity(void);
void crc_technique(void);
void checksum_technique(void);

#endif

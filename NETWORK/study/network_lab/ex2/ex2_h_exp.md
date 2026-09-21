# ex2.h — Header for experiment 2 (Parity / CRC / Checksum)

## What the file is about

Shared header for the whole of `ex2/`. It provides:

- two size constants (`MAX_STR`, `MAX_BITS`) used by every technique file,
- the prototypes of the four helper functions implemented in `utils.c`,
- the prototypes of the three technique functions called from `main.c`.

Included by: `main.c`, `utils.c`, `parity.c`, `crc.c`, `checksum.c`.

---

## Test cases

| # | Type | Situation | Result | Why |
|---|------|-----------|--------|-----|
| 1 | Positive | `gcc *.c ../convert/convert.c -o ex2` | builds; every file sees the same `MAX_BITS` and prototypes | one header, five includers. |
| 2 | Positive | `parity.c` declares `char msg[MAX_BITS]` | array of 1024 chars | macro substitution at compile time. |
| 3 | Positive | header included twice in one file | fine | include guard `EX2_H`. |
| 4 | Negative | change `MAX_BITS` to `8` and run parity on `Hi` (16 bits) | buffer overflow (undefined behaviour) | the constant sizes every bit buffer; the code does not check message length against it. |
| 5 | Negative | remove the `simulate_error` prototype and compile `crc.c` | "implicit declaration of function" error | `crc.c` calls it but only learns about it from this header. |
| 6 | Negative | build without `utils.c` (`gcc main.c parity.c crc.c checksum.c ../convert/convert.c`) | `undefined reference to input_to_bits` … | the header promises the functions; the linker can't find their bodies. |
| 7 | Edge | a message of 100 characters (the `MAX_STR` limit) | 800 bits + parity/CRC/checksum bits still < 1024 → OK | the constants were chosen with headroom: 100 chars × 8 = 800, plus at most ~115 parity bits or 32 CRC/checksum bits. |
| 8 | Edge | typing 150 characters | `scanf("%s", input)` in the techniques has **no** width limit → overflows the 100-byte `input` | `MAX_STR` sizes the buffer but the `scanf` calls in `parity.c`/`crc.c`/`checksum.c` don't use `%99s` (unlike ex1). Keep inputs short. |

---

## Line by line

```c
#ifndef EX2_H
#define EX2_H
```
Include guard.

```c
#define MAX_STR  100
```
Maximum characters of typed text (buffer size for `input[]`, `recovered_str[]`).

```c
#define MAX_BITS 1024
```
Maximum length of any bit string (`msg`, `transmitted`, `received`, `dividend`, …). 100 chars × 8 = 800 bits
plus redundancy fits. `MAX_BITS` is also used as the buffer size for the CRC remainder inside `divides_evenly`.

```c
/* shared helpers (utils.c) */
void input_to_bits(const char *input, char *bits);
```
Text or `b…` input → bit string. See `utils_exp.md`.

```c
void bits_to_str(const char *bits, char *str);
```
Bit string → text (8 bits per char).

```c
void copy_bits(char *dst, const char *src, int n);
```
Copy `n` chars and terminate.

```c
void simulate_error(char *bits);
```
Ask for one position and flip it. Used by CRC and checksum (parity has its own multi-flip version, private to `parity.c`).

```c
/* techniques */
void parity(void);
void crc_technique(void);
void checksum_technique(void);
```
The three menu entries. Defined in `parity.c`, `crc.c`, `checksum.c` respectively; called from `main.c`.
Named `crc_technique`/`checksum_technique` (rather than plain `crc`/`checksum`) to avoid clashing with local
variable names like `checksum` inside the functions.

```c
#endif
```
End of guard.

Note: functions that are only used inside one file (`xor_div`, `parity_bit`, `sum_blocks`, …) are declared
`static` in their own `.c` file and deliberately **not** listed here — the header exposes only what other
files need.

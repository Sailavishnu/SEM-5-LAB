# hamming.h — Header for experiment 3 (Hamming code)

## What the file is about

Declares everything that is **shared between `common.c`, `encode.c`, `decode.c` and `main.c`**:

- the position layout (as a comment),
- the size constant `MAX`,
- the three global arrays (`code`, `filled`, `label`) as `extern`,
- the prototypes of the shared helpers and of the two entry points.

The important new concept here is **`extern`**: it declares a global variable that is *defined* in another
file (`common.c`), so all four `.c` files share one copy.

---

## Test cases

| # | Type | Situation | Result | Why |
|---|------|-----------|--------|-----|
| 1 | Positive | `gcc *.c -o ex3` | links; `encode.c` writing `code[5]` is visible to `common.c`'s `displayFrame` | `extern` declares, `common.c` defines once — one shared array. |
| 2 | Negative | remove `extern` from `int code[MAX];` in the header | `multiple definition of 'code'` at link time (with modern gcc, which defaults to `-fno-common`) | each of the four `.c` files would now *define* its own `code` array. |
| 3 | Negative | delete the definitions from `common.c` (keep the header) | `undefined reference to 'code'` | `extern` alone allocates no storage. |
| 4 | Negative | encode 100 data bits | p = 7, totalLen = 107 > `MAX` → out-of-bounds writes | `MAX` bounds the arrays; `main.c`'s `input[MAX]` also limits input to 99 chars. |
| 5 | Edge | a data label like `"D92"` (largest possible with `MAX = 100`) | 3 chars + `'\0'` = 4 bytes, fits in `label[MAX][4]` exactly | the `4` was chosen for letter + 2 digits + terminator; a 3-digit number would overflow. |
| 6 | Positive | include the header twice | fine | guard `HAMMING_H`. |

---

## Line by line

```c
#ifndef HAMMING_H
#define HAMMING_H
```
Include guard.

```c
/*  Position :  ... 11 10  9  8  7  6  5  4  3  2  1
    Type     :  ... D7 D6 D5 P4 D4 D3 D2 P3 D1 P2 P1
    Parity bit Pk sits at position 2^(k-1); all other positions hold data. */
```
Documentation comment — the single most important fact of the whole experiment: parity bits at powers of
two, positions counted from 1 on the right.

```c
#define MAX 100
```
Maximum code-word length (and input buffer size in `main.c`). Enough for 92 data bits + 7 parity bits.

```c
extern int  code[MAX];       /* bit value at each position (1-indexed) */
```
"There is a global `int code[100]` somewhere" (it is in `common.c`). Index 0 is unused.

```c
extern int  filled[MAX];     /* is that position's value known yet?    */
```
1 = value known, 0 = still an empty parity slot (only affects display).

```c
extern char label[MAX][4];   /* "D7", "P2" ... label of each position  */
```
2-D char array: 100 strings of up to 3 characters each.

```c
/* common.c */
int  isPowerOf2(int pos);
void assignLabels(int totalLen, int p);
void displayFrame(int totalLen);
```
Helpers shared by encoder and decoder (see `common_exp.md`).

```c
/* encode.c */
void generateHammingCode(const char *data, int parityType);
```
Sender entry point, called by `main.c` for choice 1.

```c
/* decode.c */
void checkReceivedCode(const char *input, int parityType);
```
Receiver entry point, called by `main.c` for choice 2.

```c
#endif
```

The private step functions (`findParityBits`, `placeDataBits`, `calculateParity`, `detectError`,
`correctError`, `extractData`) are `static` in their own files and deliberately absent here.

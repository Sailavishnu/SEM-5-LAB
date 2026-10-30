# checksum.c — Internet-style 1's-complement Checksum

## What the file is about

A **checksum** protects data by adding up fixed-size blocks. This program uses the scheme found in IP/TCP/UDP
headers, generalised to any block size *n*:

Sender:
1. Pad the bit string with zeros so its length is a multiple of *n*.
2. Split into *n*-bit blocks and add them with **1's-complement addition** (any carry out of the top bit is
   added back into the bottom — "end-around carry").
3. The **checksum** is the 1's complement (bitwise NOT) of that sum. Append it to the data.

Receiver:
1. Add all blocks *including* the checksum block the same way.
2. Complement the result. **All zeros → no error**; anything else → error.

Why it works: sum + complement(sum) is always all-1s in 1's complement arithmetic, and NOT(all-1s) = all-0s.

---

## Test cases (whole program)

Compile `gcc *.c ../convert/convert.c -o ex2`, choose `3`.

### TC1 — Positive: `Hi`, block size `0` (→ default 8), no error (`-1`)

```
Data (padded to multiple of 8)   : 0100100001101001
Blocks:
  Block 1 : 01001000
  Block 2 : 01101001
Sum of all blocks                : 10110001
Checksum (complement of sum)     : 01001110
Data to be transmitted (data+chk) : 010010000110100101001110
---- CHECKSUM : RECEIVER SIDE ----
Received data                    : 010010000110100101001110
Blocks (data blocks + checksum block):
  Block 1 : 01001000
  Block 2 : 01101001
  Block 3 : 01001110
Sum of all blocks                : 11111111
Complement of sum (result)       : 00000000
Reason : Complement of sum is all zeros -> as expected.
Result : NO ERROR DETECTED
Recovered message (string)        : Hi
```

How: 72 + 105 = 177 = `10110001` (no carry out of 8 bits). NOT → `01001110` (78). Receiver: 72 + 105 + 78 =
255 = `11111111`; NOT → 0 → OK. The first 16 bits are returned as `Hi`.

### TC2 — Negative (detected): `Hi`, n = 8, flip bit 5

```
Received data                    : 010011000110100101001110
  Block 1 : 01001100   ← was 01001000
Sum of all blocks                : 00000100
Complement of sum (result)       : 11111011
Reason : Complement of sum is NOT all zeros (11111011) -> mismatch!
Result : ERROR DETECTED
```

How: block 1 became 76; 76 + 105 + 78 = 259 → exceeds 255 → end-around carry: (259 & 255) + 1 = 3 + 1 = 4 =
`00000100`; NOT → `11111011` ≠ 0 → error.

### TC3 — Positive with padding: `b1011010111`, n = 4

```
Data (padded to multiple of 4)   : 101101011100
  Block 1 : 1011
  Block 2 : 0101
  Block 3 : 1100
Sum of all blocks                : 1101
Checksum (complement of sum)     : 0010
Data to be transmitted (data+chk) : 1011010111000010
… receiver: Sum 1111, Complement 0000 → NO ERROR DETECTED
```

How: 10 bits → pad 2 zeros → 12 bits → three 4-bit blocks. 11 + 5 = 16 > 15 → (16 & 15) + 1 = 0 + 1 = 1;
1 + 12 = 13 = `1101`. NOT (4-bit) = `0010`. Receiver: 13 + 2 = 15 = `1111` → NOT = `0000`.

### TC4 — Edge: end-around carry with all ones, `b11111111`, n = 4

```
  Block 1 : 1111
  Block 2 : 1111
Sum of all blocks                : 1111
Checksum (complement of sum)     : 0000
```

How: 15 + 15 = 30 > 15 → (30 & 15) + 1 = 14 + 1 = 15 = `1111`. Complement `0000`. Receiver adds 0 → still
`1111` → complement `0000` → OK. Shows that in 1's complement, "all ones" behaves like a second zero.

### TC5 — Edge: error inside the **checksum** block (flip bit 20 of the 24-bit `Hi` frame)

```
  Block 3 : 01000110   ← was 01001110
Sum of all blocks                : 11110111
Complement of sum (result)       : 00001000
Result : ERROR DETECTED
```

### TC6 — Negative (theoretical weakness): two flips that cancel

E.g. flipping bit 0 of block 1 (`0→1`, +128) and bit 0 of block 2 (`0→1`, +128): sum changes by 256 →
after end-around carry that is +1, **detected**. But flipping the *same* bit position in one block from 0→1
and in another block from 1→0 leaves the sum unchanged and is **not** detected. The menu only lets you flip
one bit, so this cannot be reproduced interactively — it's why CRC is stronger.

### TC7 — Edge: block size larger than the message, e.g. `b101`, n = 8

Padded to `10100000`; one block; sum = block; checksum = NOT block; receiver: block + NOT(block) = all 1s → OK.

### TC8 — Edge: negative block size (`-3`)

`if (n <= 0) n = 8;` → treated as the default 8.

### TC9 — Negative: block size > 32 (e.g. `40`)

`mask_of(40)` returns `0xFFFFFFFF` (32-bit mask) and `bits_to_uint` shifts a 32-bit `unsigned int` 40
times — bits fall off the top; `block[33]` in `sum_blocks` can only hold 32 chars + `'\0'`, so `copy_bits`
would overflow it. The program is designed for n ≤ 32; larger values are undefined behaviour.

---

## Function 1: `bits_to_uint`

```c
static unsigned int bits_to_uint(const char *bits, int n)
```
**What:** parses the first `n` characters of a bit string into a number. **Input:** bit string, count.
**Output:** unsigned value (e.g. `"1011"`, 4 → 11). **Why:** blocks must be numbers to add them.
**Where called:** `sum_blocks()`.

```c
    unsigned int v = 0;
    for (int i = 0; i < n; i++)
        v = (v << 1) | (bits[i] - '0');
    return v;
```
Shift-and-OR, exactly like `bin_to_ascii` in `convert.c`, but for `n` bits and unsigned. `bits[i] - '0'`
converts the character `'0'`/`'1'` to the integer 0/1 (ASCII 48/49 minus 48).

---

## Function 2: `uint_to_bits`

```c
static void uint_to_bits(unsigned int v, int n, char *bits)
```
**What:** number → `n`-character bit string (MSB first). **Where called:** to print the sum and checksum (4 places in `checksum_technique()`).

```c
    for (int i = n - 1; i >= 0; i--, v >>= 1)
        bits[i] = (v & 1) ? '1' : '0';
    bits[n] = '\0';
```
Fill from the **right** end: the lowest bit of `v` goes to `bits[n-1]`, then `v >>= 1` drops that bit and the
next one goes to `bits[n-2]`, and so on. The comma operator lets both `i--` and `v >>= 1` run every iteration.
Terminate.

---

## Function 3: `mask_of`

```c
static unsigned int mask_of(int n)
```
**What:** returns an integer with the lowest `n` bits set (`n = 4` → `0b1111` = 15; `n = 8` → 255).
**Why:** used to keep sums inside `n` bits and to complement only `n` bits.
**Where called:** `add_1s_complement()`, and the two `~sum & mask_of(n)` lines.

```c
    return (n >= 32) ? 0xFFFFFFFFu : (1u << n) - 1;
```
`1u << n` = 2ⁿ; minus 1 = n ones. Shifting a 32-bit value by 32 is undefined in C, so n ≥ 32 is special-cased
to the all-ones constant. The `u` suffix makes the literal unsigned.

---

## Function 4: `add_1s_complement`

```c
static unsigned int add_1s_complement(unsigned int a, unsigned int b, int n)
```
**What:** `a + b` in `n`-bit 1's-complement arithmetic (end-around carry). **Where called:** `sum_blocks()`.

```c
    unsigned int mask = mask_of(n), sum = a + b;
    if (sum > mask)
        sum = (sum & mask) + 1;
    return sum & mask;
```
Add normally. If the result doesn't fit in `n` bits (`sum > mask`), there was a carry out of the top: keep the
low `n` bits (`sum & mask`) and add 1 (the carry wrapped around). Because `a` and `b` are each at most
`mask`, `a + b ≤ 2·mask`, so `(sum & mask) + 1` never exceeds `mask` and at most one wrap is needed; the
final `& mask` is a safety net that simply guarantees an n-bit result on both branches. See TC3/TC4 for
numeric traces.

---

## Function 5: `sum_blocks`

```c
static unsigned int sum_blocks(const char *bits, int nblocks, int n)
```
**What:** prints each block and returns the 1's-complement sum of all of them. **Input:** bit string, number of blocks, block size. **Where called:** sender (data blocks) and receiver (data + checksum blocks) in `checksum_technique()`.

```c
    unsigned int sum = 0;
    char block[33];
    for (int i = 0; i < nblocks; i++) {
        copy_bits(block, bits + i * n, n);
        printf("  Block %d : %s\n", i + 1, block);
        sum = add_1s_complement(sum, bits_to_uint(block, n), n);
    }
    return sum;
```
Block `i` starts at offset `i*n`. `copy_bits` (from `utils.c`) copies `n` chars into `block` and terminates
it so it can be printed. Convert to a number and accumulate with end-around carry.

---

## Function 6: `checksum_technique` (main flow)

```c
void checksum_technique(void)
```
**Where called:** `main.c` case 3. Declared in `ex2.h`.

```c
    char input[MAX_STR], bits[MAX_BITS], transmitted[MAX_BITS], received[MAX_BITS];
    char sum_bits[33], chk_bits[33], recovered_bits[MAX_BITS], recovered_str[MAX_STR];
    int n, len;
    unsigned int sum, checksum;
```
`sum_bits`/`chk_bits` hold up to 32-bit values as text. `n` = block size, `len` = padded bit length.

```c
    printf("\n---- CHECKSUM : SENDER SIDE ----\n");
    printf("Enter string message (or b<binary> for binary): ");
    scanf("%s", input);
    input_to_bits(input, bits);
    printf("Enter block size n (0 for default n = 8): ");
    scanf("%d", &n);
    if (n <= 0)
        n = 8;
```
Read message → bits; read block size with a default.

```c
    len = strlen(bits);
    memset(bits + len, '0', (n - len % n) % n);   /* pad with zeros to a multiple of n */
    len += (n - len % n) % n;
    bits[len] = '\0';
    printf("Data (padded to multiple of %d)   : %s\n", n, bits);
```
Padding maths: `len % n` = leftover bits in the last partial block; `n - that` = zeros needed; the outer `% n`
turns a full `n` (when `len % n == 0`) into 0, so an already-aligned message gets no padding. `memset` writes
that many `'0'` chars after the data; `len` is updated; terminate.

Example: 10 bits, n = 4 → 10 % 4 = 2 → 4 − 2 = 2 → 2 % 4 = 2 zeros. 16 bits, n = 8 → 0 → 8 → 8 % 8 = 0 zeros.

```c
    printf("Blocks:\n");
    sum = sum_blocks(bits, len / n, n);
    uint_to_bits(sum, n, sum_bits);
    printf("Sum of all blocks                : %s\n", sum_bits);
```
Sum the `len / n` blocks and show it in binary.

```c
    checksum = ~sum & mask_of(n);
    uint_to_bits(checksum, n, chk_bits);
    printf("Checksum (complement of sum)     : %s\n", chk_bits);
```
`~sum` flips **all 32** bits of the unsigned int; `& mask_of(n)` keeps only the low `n` — that's an n-bit 1's complement.

```c
    strcpy(transmitted, bits);
    strcat(transmitted, chk_bits);
    printf("Data to be transmitted (data+chk) : %s\n", transmitted);
```
Frame = data ‖ checksum.

```c
    printf("\n---- CHECKSUM : RECEIVER SIDE ----\n");
    strcpy(received, transmitted);
    simulate_error(received);
    printf("Received data                    : %s\n", received);
```
Channel (single optional flip from `utils.c`).

```c
    printf("Blocks (data blocks + checksum block):\n");
    sum = sum_blocks(received, strlen(received) / n, n);
    uint_to_bits(sum, n, sum_bits);
    printf("Sum of all blocks                : %s\n", sum_bits);
    checksum = ~sum & mask_of(n);
    uint_to_bits(checksum, n, chk_bits);
    printf("Complement of sum (result)       : %s\n", chk_bits);
```
Receiver repeats the same addition over **all** blocks (now one more than the sender had) and complements.

```c
    if (checksum != 0) {
        printf("Reason : Complement of sum is NOT all zeros (%s) -> mismatch!\n", chk_bits);
        printf("Result : ERROR DETECTED\n");
        return;
    }
    printf("Reason : Complement of sum is all zeros -> as expected.\n");
    printf("Result : NO ERROR DETECTED\n");
    copy_bits(recovered_bits, received, len);      /* original data length, before checksum block */
    bits_to_str(recovered_bits, recovered_str);
    printf("Recovered message (string)        : %s\n", recovered_str);
```
Non-zero → error. Zero → take the first `len` bits (padded data; padding zeros do not form a full extra
character for 8-bit text, and for `b…` inputs they are simply extra zeros) and turn them into text.

---

## `#include` lines

- `<stdio.h>` — I/O. `<string.h>` — `strlen`, `strcpy`, `strcat`, `memset`. `"ex2.h"` — constants and `utils.c` helpers (`input_to_bits`, `bits_to_str`, `copy_bits`, `simulate_error`).

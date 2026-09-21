# utils.c (ex2) — Shared helpers for Parity / CRC / Checksum

## What the file is about

All three error-detection techniques in experiment 2 need the same small jobs:

1. turn whatever the user typed into a **bit string** (`"0100100001101001"`),
2. turn a bit string back into **text**,
3. copy a slice of bits into a separate, properly terminated string,
4. let the user **flip one bit** to simulate a transmission error.

Rather than repeating this in `parity.c`, `crc.c` and `checksum.c`, it is written once here.
Unlike ex1, this experiment keeps bits as **characters** `'0'`/`'1'` inside normal C strings — that makes
`strlen`, `strcpy`, `strcat`, `printf("%s")` all work directly on bit streams.

A special input convention: if the message starts with `b` followed by a `0` or `1` (e.g. `b1011`), the part
after the `b` is used as raw bits instead of being treated as text.

---

## Test cases (whole file)

| # | Type | Call / input | Result | How |
|---|------|--------------|--------|-----|
| 1 | Positive | `input_to_bits("Hi", bits)` | `"0100100001101001"` (16 bits) | not a `b`-prefix → `str_to_bits`: `H`=72=`01001000`, `i`=105=`01101001`, concatenated. |
| 2 | Positive | `input_to_bits("b1011", bits)` | `"1011"` | starts with `b` and `input[1]` is `'1'` → `strcpy(bits, input + 1)` copies from the second character onward. |
| 3 | Positive | `bits_to_str("0100100001101001", s)` | `"Hi"` | 16 / 8 = 2 chars; each 8-bit group → `bin_to_ascii`. |
| 4 | Positive | `copy_bits(dst, "0100100001101001", 8)` | `dst = "01001000"` | `strncpy` copies 8 chars, then `dst[8] = '\0'`. |
| 5 | Positive | `simulate_error(bits)` on 24-bit string, user types `5` | bit 5 toggled, prints `Bit at position 5 flipped.` | `0 <= 5 < 24` → `bits[5]` changed `'0'`↔`'1'`. |
| 6 | Edge | `simulate_error`, user types `-1` | `No error introduced.` | `-1 >= 0` is false. |
| 7 | Negative | `simulate_error`, user types `24` (one past the end of a 24-bit string) | `No error introduced.` | `24 < 24` is false — the upper bound check prevents an out-of-range write. |
| 8 | Edge | `input_to_bits("b", bits)` (just a `b`) | treated as **text**: `b`=98 → `"01100010"` | `input[1]` is `'\0'`, which is neither `'0'` nor `'1'`, so the prefix rule doesn't apply. |
| 9 | Edge | `input_to_bits("bat", bits)` | text: `"011000100110000101110100"` | `input[1]` = `'a'` → not a bit → text path. So words starting with `b` are safe unless followed by 0/1. |
| 10 | Edge | `input_to_bits("b10x1", bits)` | `"10x1"` copied verbatim | no validation of the bits after `b`; downstream functions treat `x` as a 0 bit (`== '1'` false). |
| 11 | Edge | `bits_to_str("1011", s)` (fewer than 8 bits) | `s = ""` | `4 / 8 = 0` chars (integer division). This is why parity on `b1011` prints an empty recovered string. |
| 12 | Edge | `bits_to_str("010010000110100101", s)` (18 bits) | `"Hi"` — the extra 2 bits are ignored | `18 / 8 = 2`. |
| 13 | Negative | `copy_bits(dst, src, n)` with `n` larger than `strlen(src)` | `strncpy` pads with `'\0'` up to `n`, so `dst` is just `src`; no crash as long as `dst` has `n+1` bytes | standard `strncpy` behaviour. |

---

## Function 1: `str_to_bits` (private)

```c
static void str_to_bits(const char *str, char *bits)
```

**What:** text → concatenated 8-bit binary of every character.
**Input:** `str` = text; `bits` = output buffer (needs `8*len + 1` bytes).
**Output:** `bits` filled, e.g. `"Hi"` → `"0100100001101001"`.
**Why:** all three techniques operate on the message's bits.
**Where called:** only by `input_to_bits()` below. `static` hides it from other files.

```c
    int len = strlen(str);
```
Number of characters.

```c
    for (int i = 0; i < len; i++)
        ascii_to_bin(str[i], bits + i * 8);
```
For character `i`, write its 8 bits directly at offset `i*8` of the output (`bits + i*8` is a pointer into the
big buffer). `ascii_to_bin` (from `convert.c`) also writes a `'\0'` at position 8 of *its* slice, but the next
iteration overwrites that with the next character's first bit, so the pieces join up.

```c
    bits[len * 8] = '\0';
```
Final terminator after the last character's 8 bits (the last `ascii_to_bin` already put one there, but this
makes the intent explicit and handles `len = 0`).

---

## Function 2: `input_to_bits`

```c
void input_to_bits(const char *input, char *bits)
```

**What:** decides whether the user gave raw bits (`b…`) or text, and produces the bit string either way.
**Input:** `input` = what the user typed; `bits` = output buffer.
**Output:** `bits`.
**Why:** lets the lab be run both with readable text (`Hi`) and with the textbook-style bit examples (`b1011010111`).
**Where called:** `parity()` in `parity.c`, `crc_technique()` in `crc.c`, `checksum_technique()` in `checksum.c` — always right after reading the message.

```c
    if (input[0] == 'b' && (input[1] == '0' || input[1] == '1'))
        strcpy(bits, input + 1);
```
Prefix rule: first char `b` **and** second char a binary digit → copy everything from index 1 (skipping the `b`).
`input + 1` is pointer arithmetic = "the string starting at the second character".

```c
    else
        str_to_bits(input, bits);
```
Otherwise treat as text.

---

## Function 3: `bits_to_str`

```c
void bits_to_str(const char *bits, char *str)
```

**What:** bit string → text, 8 bits per character.
**Input:** `bits` (any length), `str` output buffer.
**Output:** `str`. Leftover bits (not a full group of 8) are dropped.
**Why:** after a successful check, each technique prints the "Recovered message (string)".
**Where called:** end of `parity()`, `crc_technique()`, `checksum_technique()`.

```c
    int nchars = strlen(bits) / 8;
```
Integer division: how many complete bytes there are (test 11, 12).

```c
    for (int i = 0; i < nchars; i++)
        str[i] = bin_to_ascii(bits + i * 8);
```
`bits + i*8` points at the start of group `i`; `bin_to_ascii` reads exactly 8 chars from there.

```c
    str[nchars] = '\0';
```
Terminate.

---

## Function 4: `copy_bits`

```c
void copy_bits(char *dst, const char *src, int n)
```

**What:** copies the first `n` characters of `src` into `dst` and terminates it — i.e. "take a slice".
**Input:** `dst` (buffer of `n+1`), `src`, `n`.
**Output:** `dst` as a valid C string.
**Why:** `strncpy` alone does **not** add a `'\0'` when the source is at least `n` long; forgetting that is a
classic bug, so it's wrapped here. Used to extract one frame, one block, or the data part of a received stream.
**Where called:** `parity()` (frame strings), `checksum.c` `sum_blocks()` (each block) and `checksum_technique()` (data part), `crc_technique()` (data part).

```c
    strncpy(dst, src, n);
    dst[n] = '\0';
```
Copy `n` chars; write the terminator explicitly.

---

## Function 5: `simulate_error`

```c
void simulate_error(char *bits)
```

**What:** asks for one bit position and toggles that bit in place (or does nothing for `-1` / invalid).
**Input:** `bits` — the transmitted string to be corrupted; keyboard: an integer.
**Output:** `bits` possibly modified; a message on screen.
**Why:** to demonstrate that CRC and checksum catch a single-bit error. (Parity has its own multi-bit version, `simulate_multi_error` in `parity.c`, because it needs to demonstrate even-count errors.)
**Where called:** `crc_technique()` and `checksum_technique()` on the receiver side.

```c
    int len = strlen(bits), pos;
    printf("\nSimulate a transmission error?\n");
    printf("Enter bit position to flip (0 to %d), or -1 for NO error: ", len - 1);
    scanf("%d", &pos);
```
Show the valid range (`0 … len-1`) and read the choice.

```c
    if (pos >= 0 && pos < len) {
        bits[pos] = (bits[pos] == '0') ? '1' : '0';
        printf("Bit at position %d flipped.\n", pos);
    } else
        printf("No error introduced.\n");
```
Bounds check, then toggle using a ternary: `'0'` → `'1'`, anything else (`'1'`) → `'0'`. Out of range (including `-1`) → no change.

---

## `#include` lines

- `<stdio.h>` — `printf`, `scanf`.
- `<string.h>` — `strlen`, `strcpy`, `strncpy`.
- `"../convert/convert.h"` — `ascii_to_bin`, `bin_to_ascii`.
- `"ex2.h"` — the prototypes of the four public functions here, plus `MAX_*` constants.

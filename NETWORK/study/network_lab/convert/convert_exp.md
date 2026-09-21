# convert.c — ASCII ⇄ Binary ⇄ String conversion helpers

This file is a small **shared helper library**. It does not print anything and does not take input from the user.
It only converts data between three forms that the experiments need:

| Form | Example | Used for |
|------|---------|----------|
| Text string | `"Hi"` | what the user types |
| ASCII integer array | `{72, 105}` | one number per character |
| 8-bit binary string | `"01001000"` | the bits that are actually "transmitted" |

`ex1` (bit/byte stuffing) and `ex2` (parity/CRC/checksum) both include `../convert/convert.h` and call these
functions. `ex3`, `ex4`, `ex5` do **not** use this file.

Background you must know: every character has an **ASCII code** (a number 0–255). `'A'` = 65, `'B'` = 66,
`'a'` = 97, `'0'` = 48, space = 32, `'~'` = 126. Any number 0–255 can be written with exactly **8 binary digits**
(bits). 65 = `01000001` because 64 + 1 = 65 and the 8 bit places are worth 128, 64, 32, 16, 8, 4, 2, 1.

---

## Test cases (whole file)

The functions are pure (no I/O), so every test case is "call the function with X, expect Y".

### Positive cases

| # | Call | Result | How the code gets there |
|---|------|--------|-------------------------|
| 1 | `ascii_to_bin('A', buf)` | `buf = "01000001"` | `'A'` = 65 = 64 + 1. The loop tests bit 7 down to bit 0: 128? no → `'0'`; 64? yes → `'1'`; 32,16,8,4,2? no → `'0'`; 1? yes → `'1'`. |
| 2 | `ascii_to_bin('H', buf)` | `"01001000"` | 72 = 64 + 8 → bits 6 and 3 set. |
| 3 | `ascii_to_bin('~', buf)` | `"01111110"` | 126 = 64+32+16+8+4+2. This is the HDLC flag byte used in ex1. |
| 4 | `bin_to_ascii("01000001")` | `'A'` (65) | value builds up left to right: 0,0,1,2,4,8,16,32,65 → 65 → `'A'`. |
| 5 | `bin_to_ascii("01101001")` | `'i'` (105) | 64+32+8+1 = 105. |
| 6 | `str_to_ascii("Hi", arr, &len)` | `arr = {72,105}`, `len = 2` | `strlen` gives 2, each char is cast to its ASCII number. |
| 7 | `ascii_to_str({72,105}, 2, s)` | `s = "Hi"` | each number cast back to a char, then a `'\0'` terminator is added. |
| 8 | Round trip: `ascii_to_bin` then `bin_to_ascii` on every char 0–255 | returns the same value | the two functions are exact inverses: one splits a number into 8 bits, the other rebuilds it from the same 8 bits. |

### Edge cases

| # | Call | Result | Why |
|---|------|--------|-----|
| 9 | `ascii_to_bin('\0', buf)` (value 0) | `"00000000"` | no bit is set, all eight tests give `'0'`. |
| 10 | `ascii_to_bin((char)255, buf)` | `"11111111"` | If `char` is signed, 255 is stored as −1, which in two's complement is all 1 bits, so the `&` test succeeds for every mask — same answer either way. |
| 11 | `bin_to_ascii("00000000")` | `'\0'` (0) | never ORs in a 1. |
| 12 | `bin_to_ascii("11111111")` | `(char)255` (prints as −1 if `char` is signed) | value = 255; cast to `char`. |
| 13 | `str_to_ascii("", arr, &len)` | `len = 0`, `arr` untouched | `strlen("")` is 0, loop body never runs. |
| 14 | `ascii_to_str(arr, 0, s)` | `s = ""` | loop does nothing, `s[0] = '\0'`. |
| 15 | `str_to_ascii("hi there", …)` | `{104,105,32,116,104,101,114,101}`, len 8 | space is a normal character (32). |

### Negative / misuse cases (what happens if you break the contract)

| # | Call | Result | Why |
|---|------|--------|-----|
| 16 | `bin_to_ascii("0100001")` (only 7 chars) | reads `bin_str[7]`, which is the `'\0'` terminator → treated as `'0'` → result is `0100001`**0** = 66 = `'B'` instead of `'A'`. | The function always reads exactly 8 characters; it never checks the length. Caller must pass 8 bits. |
| 17 | `bin_to_ascii("0100000X")` | `'X'` ≠ `'1'` so it counts as 0 → 64 = `'@'` | Anything that is not the character `'1'` is treated as a 0 bit. No error is raised. |
| 18 | `ascii_to_bin('A', buf)` with `char buf[8]` (too small) | writes `buf[8]` = out of bounds → undefined behaviour | It needs **9** bytes: 8 bits + terminator. Every caller in the project uses `char bin_str[9]`. |
| 19 | `ascii_to_str(arr, len, str)` where `arr[i] = 300` | `(char)300` = 44 = `','` | values above 255 are silently truncated to their low 8 bits. |

---

## Function 1: `ascii_to_bin`

```c
void ascii_to_bin(char ascii, char *bin_str)
```

**What it does:** converts ONE character into its 8-bit binary text, MSB (most significant bit) first.

**Input:**
- `ascii` — the character (e.g. `'A'`).
- `bin_str` — a caller-provided buffer of at least 9 chars that will be filled.

**Output:** nothing is returned; `bin_str` becomes e.g. `"01000001"` (8 characters + `'\0'`).

**Why it is needed:** the experiments work on bit streams (stuffing, parity, CRC), but the user types text.
This is the bridge from "one letter" to "eight bits".

**Where it is called:**
- `ex1/bit_stuffing.c` → `bitStuffing()` (each character of the input string is turned into 8 data bits).
- `ex2/utils.c` → `str_to_bits()` (builds the whole message bit string).

**Line by line:**

```c
void ascii_to_bin(char ascii, char *bin_str) {
```
Function header. `char ascii` is passed by value (a copy). `char *bin_str` is a pointer, so the function can write into the caller's array.

```c
    for (int i = 0; i < 8; i++)
```
Runs 8 times, `i` = 0 … 7, one iteration per output bit. `i = 0` is the leftmost character of the result.

```c
        bin_str[i] = (ascii & (1 << (7 - i))) ? '1' : '0';
```
This is the whole algorithm in one line, read it from the inside out:
- `7 - i` : when `i = 0` this is 7, when `i = 7` this is 0. So we walk from bit 7 (value 128) down to bit 0 (value 1).
- `1 << (7 - i)` : "shift 1 left by that many places" = a **mask** with only one bit set. For `i = 0` it is `10000000` (128), for `i = 1` it is `01000000` (64), … for `i = 7` it is `00000001` (1).
- `ascii & mask` : bitwise AND keeps only that one bit of `ascii`. Result is non-zero if the bit is 1, zero if the bit is 0.
- `? '1' : '0'` : ternary operator — non-zero → store the **character** `'1'`, zero → the character `'0'`.
  Note it stores characters (`'1'` = ASCII 49), not numbers, because the output is a printable string.

Example `'A'` (65 = `01000001`):

| i | 7−i | mask | 65 & mask | char stored |
|---|-----|------|-----------|-------------|
| 0 | 7 | 128 | 0 | `'0'` |
| 1 | 6 | 64 | 64 | `'1'` |
| 2 | 5 | 32 | 0 | `'0'` |
| 3 | 4 | 16 | 0 | `'0'` |
| 4 | 3 | 8 | 0 | `'0'` |
| 5 | 2 | 4 | 0 | `'0'` |
| 6 | 1 | 2 | 0 | `'0'` |
| 7 | 0 | 1 | 1 | `'1'` |

→ `"01000001"`.

```c
    bin_str[8] = '\0';
```
C strings must end with a NUL character so that `printf("%s")`, `strlen`, `strcpy` etc. know where the string stops. Without this the caller would read garbage after the 8 bits. This is why the buffer must be **9** bytes.

---

## Function 2: `bin_to_ascii`

```c
char bin_to_ascii(const char *bin_str)
```

**What it does:** the exact reverse of `ascii_to_bin` — reads the first 8 characters of a binary string and returns the character they represent.

**Input:** `bin_str` — pointer to at least 8 characters of `'0'`/`'1'`. `const` promises the function will not modify it.

**Output:** the `char` whose ASCII code equals the binary value (e.g. `"01000001"` → `'A'`).

**Why it is needed:** after the receiver de-stuffs / verifies the bits, the program wants to print the recovered text again. Also used to convert a user-typed 8-bit pattern (like the escape byte `00011011`) into a number.

**Where it is called:**
- `ex1/bit_stuffing.c` → `bitStuffing()` (rebuilds output text from de-stuffed bits).
- `ex1/byte_stuffing.c` → `readFrameByte()` and `byteStuffing()` (turn typed 8-bit patterns into byte values).
- `ex2/utils.c` → `bits_to_str()`.

**Line by line:**

```c
char bin_to_ascii(const char *bin_str) {
    int value = 0;
```
Accumulator that will hold the number being built. Starts at 0. Declared as `int` (not `char`) so there is no risk of overflow/sign issues while building.

```c
    for (int i = 0; i < 8; i++)
        value = (value << 1) | (bin_str[i] == '1');
```
Classic "shift-and-add" binary parsing, left to right:
- `value << 1` : shift everything already collected one place left (multiply by 2), making room for the next bit at the bottom.
- `bin_str[i] == '1'` : a comparison in C evaluates to `1` (true) or `0` (false) — so this expression **is** the bit value.
- `|` : bitwise OR puts that bit into the lowest position.

Trace for `"01000001"`:

| i | char | value before | `<<1` | OR bit | value after |
|---|------|--------------|-------|--------|-------------|
| 0 | 0 | 0 | 0 | 0 | 0 |
| 1 | 1 | 0 | 0 | 1 | 1 |
| 2 | 0 | 1 | 2 | 0 | 2 |
| 3 | 0 | 2 | 4 | 0 | 4 |
| 4 | 0 | 4 | 8 | 0 | 8 |
| 5 | 0 | 8 | 16 | 0 | 16 |
| 6 | 0 | 16 | 32 | 0 | 32 |
| 7 | 1 | 32 | 64 | 1 | 65 |

65 = `'A'`.

Because only `== '1'` is tested, any other character (`'0'`, a letter, even `'\0'`) counts as a 0 bit — see negative test cases 16 and 17.

```c
    return (char)value;
```
Cast the integer to `char` so the caller gets a character. Values 0–255 fit; nothing larger can occur because only 8 bits were read.

---

## Function 3: `str_to_ascii`

```c
void str_to_ascii(const char *str, int *ascii_arr, int *len)
```

**What it does:** turns a whole text string into an array of ASCII numbers and reports how many there are.

**Input:**
- `str` — the text (e.g. `"Hi"`).
- `ascii_arr` — caller's int array to fill.
- `len` — pointer to an int where the length will be stored (an "output parameter").

**Output:** `ascii_arr = {72, 105}`, `*len = 2`.

**Why it is needed:** ex1 works with ints (one per character/byte) rather than raw chars so that special byte values (SOF/EOF/ESC) can be compared as plain integers and printed as 8-bit binary.

**Where it is called:**
- `ex1/bit_stuffing.c` → `bitStuffing()`.
- `ex1/byte_stuffing.c` → `byteStuffing()`.

**Line by line:**

```c
    *len = strlen(str);
```
`strlen` counts characters up to (not including) the `'\0'`. Storing it through the pointer means the caller's variable is updated. This needs `#include <string.h>` (line 1 of the file).

```c
    for (int i = 0; i < *len; i++)
        ascii_arr[i] = (int)str[i];
```
For each character copy its numeric code into the int array. The `(int)` cast is explicit for clarity — a `char` promotes to `int` automatically anyway. `'H'` becomes 72, `'i'` becomes 105.

No terminator is written — an int array has no "end marker"; that is why `len` is returned separately.

---

## Function 4: `ascii_to_str`

```c
void ascii_to_str(const int *ascii_arr, int len, char *str)
```

**What it does:** reverse of `str_to_ascii` — packs an int array back into a printable C string.

**Input:** `ascii_arr` (e.g. `{72,105}`), `len` (2), `str` (buffer of at least `len + 1` chars).

**Output:** `str = "Hi"`.

**Why it is needed:** the receiver side of ex1 rebuilds the message as ints and wants to print it with `printf("%s")`.

**Where it is called:**
- `ex1/bit_stuffing.c` → end of `bitStuffing()` ("Output Text").
- `ex1/byte_stuffing.c` → end of `byteStuffing()`.

**Line by line:**

```c
    for (int i = 0; i < len; i++)
        str[i] = (char)ascii_arr[i];
```
Copy each number as a character. The `(char)` cast throws away anything above 8 bits (see test case 19).

```c
    str[len] = '\0';
```
Add the string terminator right after the last character so the result is a valid C string.

---

## The `#include` lines

```c
#include <string.h>
```
Needed for `strlen()` used in `str_to_ascii`.

```c
#include "convert.h"
```
Pulls in the prototypes so the compiler can verify that each definition here matches the declaration the other files see. Quotes (not `< >`) mean "look in this folder first".

# crc.c — Cyclic Redundancy Check (CRC)

## What the file is about

**CRC** treats the message bits as a polynomial over GF(2) (arithmetic where addition = XOR and there are no
carries). The sender and receiver agree on a **generator polynomial** G(x) of degree *r* (written as *r+1* bits).

Sender:
1. Append *r* zeros to the data D.
2. Divide by G using **XOR long division**; keep the *r*-bit remainder (the CRC).
3. Transmit T = D followed by the remainder.

Receiver:
1. Divide the received T by G.
2. Remainder all zeros → no error; anything else → error.

Why it works: T was built so that it is exactly divisible by G. Any error pattern E that is not a multiple of
G changes the remainder.

This file also **validates the generator** against four textbook design criteria before using it, which is
why the default `1001` from the prompt is always rejected (see TC3) — a deliberate teaching point.

---

## Test cases (whole program)

Compile `gcc *.c ../convert/convert.c -o ex2`, choose `2`.

### TC1 — Positive: `Hi`, generator `100000111`, no error (`-1`)

```
Source data (binary)             : 0100100001101001
Generator accepted: 100000111
Data padded with 8 zeros         : 010010000110100100000000
CRC remainder (redundant bits)   : 11101011
Data to be transmitted (T = D+CRC): 010010000110100111101011
---- CRC : RECEIVER SIDE ----
Received data                    : 010010000110100111101011
Remainder after division by G(x) : 00000000
Result : NO ERROR DETECTED
Recovered message (string)       : Hi
```

How: generator has 9 bits → r = 8 → 8 zeros appended → `xor_div` → remainder `11101011`. Receiver divides
`D+CRC` by the same G → zero remainder → the first 16 bits are the data → `Hi`.

### TC2 — Negative (detected): same, flip bit 5

```
Bit at position 5 flipped.
Received data                    : 010011000110100111101011
Remainder after division by G(x) : 01010100
Result : ERROR DETECTED
```

How: a single-bit error is E(x) = x^k, which is never divisible by a generator with ≥ 2 terms → non-zero remainder.

### TC3 — Negative: default generator `0` → `1001` is rejected; then `1011`; then `100000111`

```
Enter generator …: Generator rejected. Issue(s) found:
  - Criterion 3 failed: generator divides x^3 + 1, which it should not.
Enter generator …: Generator rejected. Issue(s) found:
  - Criterion 4 failed: generator must have the factor (x + 1), i.e. it must be evenly divisible by 11.
Enter generator …: Generator accepted: 100000111
```

How: `1001` **is** x^3 + 1, so of course it divides x^3 + 1 (criterion 3). `1011` = x^3 + x + 1 has an odd
number of terms (3), and a polynomial is divisible by (x + 1) exactly when it has an even number of terms
(evaluate at x = 1) → fails criterion 4. `100000111` = x^8 + x^2 + x + 1: 4 terms (even) ✓, ends in 1 ✓,
divides no x^t + 1 for t up to frame length ✓ → accepted.

### TC4 — Negative: generator `11`

```
  - Criterion 3 failed: generator divides x^2 + 1, which it should not.
```
How: x^2 + 1 = (x + 1)² in GF(2), so `11` divides it. (After a rejection the loop asks again.)

### TC5 — Negative: generator `1000`

```
  - Criterion 1 failed: … at least two 1's.
  - Criterion 2 failed: coefficient of x^0 (the last bit) must be 1.
  - Criterion 4 failed: … divisible by 11.
  - Criterion 3 failed: generator divides x^2 + 1 …
```
How: only one 1 (crit 1); ends in 0 (crit 2); 1 term is odd (crit 4). All failures are collected and printed
together so the user sees every problem at once.

### TC6 — Positive with raw bits: `b1101`, generator `100000111`

```
Data padded with 8 zeros         : 110100000000
CRC remainder (redundant bits)   : 00100011
Data to be transmitted (T = D+CRC): 110100100011
Remainder after division by G(x) : 00000000
Result : NO ERROR DETECTED
Recovered message (string)       :
```
Hand trace of the division (`temp` = 12 chars, `gen_len` = 9, loop `i` = 0…3):

| i | temp[i] | action | temp after |
|---|---------|--------|------------|
| 0 | 1 | XOR `100000111` into cols 0–8 | `010100111000` |
| 1 | 1 | XOR into cols 1–9 | `000100100100` |
| 2 | 0 | skip | `000100100100` |
| 3 | 1 | XOR into cols 3–11 | `000000100011` |

Remainder = last 8 chars = `00100011` ✓. (Recovered string is empty because 4 bits < 1 character.)

### TC7 — Edge: error in the CRC part itself (flip bit 20 of the 24-bit `Hi` frame)

Remainder non-zero → `ERROR DETECTED`. The check bits are protected just like the data.

### TC8 — Edge: input exhausted while the generator is rejected

If the program is fed from a file/pipe and the generator is invalid, `scanf` keeps failing and the loop
prints the rejection forever (there is no "give up" exit). Interactively, just type a valid generator.
`run.md` §4 mentions this.

### TC9 — Edge: two-bit burst that G cannot catch

Not reachable from the menu (only one flip is offered), but theoretically an error pattern that is a
multiple of G — e.g. flipping *exactly* the bits of G at some offset — yields a zero remainder and slips
through. Criteria 3 & 4 are there to make such patterns as rare as possible.

---

## Function 1: `xor_div`

```c
static void xor_div(const char *data, const char *gen, char *remainder)
```

**What:** binary (mod-2) long division; returns the remainder.
**Input:** `data` (dividend bit string), `gen` (divisor), `remainder` (buffer for `gen_len - 1` bits + `'\0'`).
**Output:** `remainder` filled. `data` is not modified (a copy is used).
**Why:** it is *the* CRC operation, used by the sender (to compute the CRC), the receiver (to verify), and the
generator checker (to test divisibility).
**Where called:** `divides_evenly()`, and twice in `crc_technique()`.

```c
    int data_len = strlen(data), gen_len = strlen(gen);
    char *temp = malloc(data_len + 1);
    strcpy(temp, data);
```
Work on a heap copy so the caller's string stays intact (`data` is `const`). `+1` for the terminator.

```c
    for (int i = 0; i <= data_len - gen_len; i++) {
        if (temp[i] == '1')
            for (int j = 0; j < gen_len; j++)
                temp[i + j] = (temp[i + j] == gen[j]) ? '0' : '1';
    }
```
The long division. `i` is the current leftmost column; it can advance until the generator would run off the
end (`data_len - gen_len`). If the bit at column `i` is `1`, XOR the generator into columns `i … i+gen_len-1`
(same → `'0'`, different → `'1'` is XOR on characters). If the bit is `0`, the "quotient bit" is 0 and nothing
is subtracted — just move on. After each step column `i` is guaranteed to be `0`. (See TC6 trace.)

```c
    strcpy(remainder, temp + data_len - (gen_len - 1));
    free(temp);
```
The last `gen_len - 1` characters are the remainder (a remainder always has one fewer bit than the divisor).
`temp + data_len - (gen_len-1)` points there. Free the copy.

---

## Function 2: `count_ones`

```c
static int count_ones(const char *s)
```
**What:** number of `'1'` characters in a string. **Where called:** criterion 1 in `get_valid_generator()`.

```c
    int c = 0;
    for (int i = 0; s[i]; i++)
        c += s[i] == '1';
    return c;
```
`s[i]` as a loop condition stops at the `'\0'`. `== '1'` contributes 1 or 0.

---

## Function 3: `divides_evenly`

```c
static int divides_evenly(const char *dividend, const char *gen)
```
**What:** 1 if `gen` divides `dividend` with zero remainder. **Where called:** criterion 4 (`gen ÷ 11`) and criterion 3 (`x^t+1 ÷ gen`).

```c
    char remainder[MAX_BITS];
    if (strlen(dividend) < strlen(gen))
        return 0;
```
A shorter dividend is smaller than the divisor → cannot be a multiple (and `xor_div`'s loop bound would go negative).

```c
    xor_div(dividend, gen, remainder);
    return strspn(remainder, "0") == strlen(gen) - 1;
```
`strspn(s, "0")` = length of the leading run of `'0'` characters. If that run equals the whole remainder
length (`gen_len - 1`), the remainder is all zeros.

---

## Function 4: `get_valid_generator`

```c
static void get_valid_generator(char *gen, int data_len)
```

**What:** reads generators until one passes all four criteria.
**Input:** `gen` (output buffer), `data_len` (number of data bits, needed for criterion 3's range).
**Output:** `gen` holds an accepted generator; messages on screen.
**Why:** to teach the properties a good CRC generator must have and to make the student pick a real one.
**Where called:** once in `crc_technique()`.

```c
    while (1) {
        char fails[4][160];
        int nfail = 0, glen, n, bad_t = -1;
```
Loop until `return`. `fails` collects up to 4 messages (one per criterion) so they can all be printed
together. `bad_t` = the first `t` for which G divides x^t + 1 (−1 = none found).

```c
        printf("Enter generator polynomial bits (e.g. 1001), or press 0 for default 1001: ");
        scanf("%s", gen);
        if (strcmp(gen, "0") == 0)
            strcpy(gen, "1001");
        glen = strlen(gen);
        n = data_len + glen - 1;               /* length of data + redundancy stream */
```
Read; `0` means the default `1001`. `n` = length of the frame that will be transmitted (data + r bits): this
bounds the distance between two errors that criterion 3 must cover.

```c
        if (count_ones(gen) < 2)
            strcpy(fails[nfail++], "Criterion 1 failed: generator must have at least two terms (at least two 1's).");
```
**Criterion 1:** with a single term (x^k) the generator could never detect a single-bit error.

```c
        if (gen[glen - 1] != '1')
            strcpy(fails[nfail++], "Criterion 2 failed: coefficient of x^0 (the last bit) must be 1.");
```
**Criterion 2:** the constant term must be 1 (otherwise G = x·G' and the trailing bit carries no protection).

```c
        if (!divides_evenly(gen, "11"))
            strcpy(fails[nfail++], "Criterion 4 failed: generator must have the factor (x + 1), i.e. it must be evenly divisible by 11.");
```
**Criterion 4:** having (x+1) as a factor guarantees that *any odd number* of bit errors is detected.
`11` is the bit pattern of x + 1. (Numbered 4 in the messages because the textbook lists it fourth; it is
checked here before 3 because it is cheaper.)

```c
        for (int t = 2; t < n && bad_t < 0; t++) {     /* G(x) must not divide x^t + 1 */
            char pattern[MAX_BITS + 8];
            if (t + 1 < glen)
                continue;
            memset(pattern, '0', t + 1);
            pattern[0] = pattern[t] = '1';
            pattern[t + 1] = '\0';
            if (divides_evenly(pattern, gen))
                bad_t = t;
        }
        if (bad_t >= 0)
            sprintf(fails[nfail++], "Criterion 3 failed: generator divides x^%d + 1, which it should not.", bad_t);
```
**Criterion 3:** G must not divide x^t + 1 for any t between 2 and the frame length − 1. Reason: two isolated
single-bit errors t positions apart form the error polynomial x^i (x^t + 1); if G divided x^t + 1 that
double error would be invisible. The loop builds the string `1 0…0 1` (length t + 1), skips lengths shorter
than G (cannot be divisible anyway), and stops at the first offending `t` (`bad_t < 0` in the condition).
`sprintf` formats the message with the actual `t`.

```c
        if (nfail == 0) {
            printf("Generator accepted: %s\n", gen);
            return;
        }
        printf("Generator rejected. Issue(s) found:\n");
        for (int i = 0; i < nfail; i++)
            printf("  - %s\n", fails[i]);
        printf("Please enter a different generator.\n");
    }
```
No failures → done. Otherwise list them all and loop back to the prompt.

---

## Function 5: `crc_technique` (main flow)

```c
void crc_technique(void)
```
**Where called:** `main.c` case 2. Declared in `ex2.h`.

```c
    char input[MAX_STR], data[MAX_BITS], gen[32], dividend[MAX_BITS], remainder[32];
    char transmitted[MAX_BITS], received[MAX_BITS], check[32], recovered_bits[MAX_BITS], recovered_str[MAX_STR];
    int dlen, r;
```
`gen`, `remainder`, `check` are 32 bytes — generators up to 31 bits. `dividend` = data + r zeros.
`check` = receiver's remainder. `r` = degree of G = number of CRC bits.

```c
    printf("\n---- CRC : SENDER SIDE ----\n");
    printf("Enter string message (or b<binary> for binary): ");
    scanf("%s", input);
    input_to_bits(input, data);
    dlen = strlen(data);
    printf("Source data (binary)             : %s\n", data);
```
Read and convert.

```c
    get_valid_generator(gen, dlen);
    printf("Generator G(x) used              : %s\n", gen);
```
Obtain an accepted generator.

```c
    r = strlen(gen) - 1;                        /* append r zeros, divide, keep remainder */
    strcpy(dividend, data);
    memset(dividend + dlen, '0', r);
    dividend[dlen + r] = '\0';
    printf("Data padded with %d zeros         : %s\n", r, dividend);
```
Build D·x^r: copy the data, write `r` `'0'` characters after it (`memset` fills bytes), terminate.

```c
    xor_div(dividend, gen, remainder);
    printf("CRC remainder (redundant bits)   : %s\n", remainder);
    strcpy(transmitted, data);
    strcat(transmitted, remainder);
    printf("Data to be transmitted (T = D+CRC): %s\n", transmitted);
```
Divide, then T = D ‖ remainder (`strcat` appends).

```c
    printf("\n---- CRC : RECEIVER SIDE ----\n");
    strcpy(received, transmitted);
    simulate_error(received);
    printf("Received data                    : %s\n", received);
```
Channel with optional single-bit flip (from `utils.c`).

```c
    xor_div(received, gen, check);
    printf("Remainder after division by G(x) : %s\n", check);
    if (strspn(check, "0") != strlen(check)) {
        printf("Result : ERROR DETECTED\n");
        return;
    }
```
Receiver divides the whole frame. If the leading run of zeros is shorter than the remainder, some bit is 1 → error.

```c
    printf("Result : NO ERROR DETECTED\n");
    copy_bits(recovered_bits, received, dlen);
    bits_to_str(recovered_bits, recovered_str);
    printf("Recovered message (string)       : %s\n", recovered_str);
```
Zero remainder → the first `dlen` bits are the data → text.

---

## `#include` lines

- `<stdio.h>` — I/O and `sprintf`. `<stdlib.h>` — `malloc`/`free`. `<string.h>` — `strlen`, `strcpy`, `strcat`, `strcmp`, `strspn`, `memset`. `"ex2.h"` — constants and helpers.

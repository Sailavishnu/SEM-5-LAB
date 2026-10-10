# parity.c — Parity-bit error detection on 7-bit frames

## What the file is about

**Parity** is the simplest error-detection code. The sender counts the 1s in a group of data bits and adds one
extra bit (the *parity bit*) so that the total number of 1s is:

- **even** (even parity, scheme 0), or
- **odd** (odd parity, scheme 1).

The receiver counts the 1s again (data + parity bit). If the count has the wrong parity, an error is detected.

This program splits the message into **7-bit frames** and gives each frame its own parity bit (like classic
7-bit ASCII + parity = 8 bits). It also lets the user flip **several** bits, to show the well-known weakness:
an **even number of flips inside the same frame cancels out** and goes undetected.

---

## Test cases (whole program)

Compile `gcc *.c ../convert/convert.c -o ex2`, choose `1`.

### TC1 — Positive: `Hi`, even parity, 0 flips

```
Original message (m) in binary              : 0100100001101001
  Frame 1 (7 data bits) : 01001000   (parity bit = 0)
  Frame 2 (7 data bits) : 00110101   (parity bit = 1)
  Frame 3 (2 data bits) : 011   (parity bit = 1)
Data to be sent (all frames concatenated)   : 0100100000110101011
…
How many bits do you want to flip? (0 for NO error): No error introduced.
Checking each frame:
  Frame 1 : 01001000  -> OK
  Frame 2 : 00110101  -> OK
  Frame 3 : 011  -> OK
Result : NO ERROR DETECTED
Recovered message (string)                   : Hi
```

How: 16 bits → frames of 7, 7, 2. Frame 1 `0100100` has two 1s (even) → parity 0. Frame 2 `0011010` has three
1s (odd) → parity 1 makes it even. Frame 3 `01` has one 1 → parity 1. Receiver recounts each frame including
its parity bit: all even → OK. Data bits are re-assembled (`0100100` + `0011010` + `01` = original 16 bits) →
`Hi`.

### TC2 — Negative (detected): `Hi`, even, 1 flip at position 3

```
  Frame 1 : 01011000  -> MISMATCH (error detected)   [1 bit(s) flipped in this frame - ODD - correctly DETECTED]
  Frame 2 : 00110101  -> OK
  Frame 3 : 011  -> OK
Result : ERROR DETECTED
```

How: position 3 is inside frame 1 (positions 0–7). Frame 1 now has three 1s → odd → does not match even
parity → mismatch. The program also knows *where* the flips landed (it stored them in `flipped[]`), so it can
annotate "1 bit flipped – ODD – correctly detected".

### TC3 — Negative (NOT detected — the weakness): `Hi`, even, 2 flips at positions 0 and 1

```
  Frame 1 : 10001000  -> OK   [2 bit(s) flipped in this frame - EVEN - NOT DETECTED, parity still matches!]
  Frame 2 : 00110101  -> OK
  Frame 3 : 011  -> OK
Result : NO ERROR DETECTED
Recovered message (string)                   : �i
WARNING: 2 bit(s) were actually flipped during transmission, but every
affected frame had an EVEN flip count, so parity could not catch it.
```

How: `01` → `10` — one 1 removed, one 1 added, count unchanged → parity still matches. The "recovered" text
is wrong (`10001000` = 136, an unprintable byte, shown as `�`), which the program flags with a warning.

### TC4 — Negative: 2 flips in **different** frames (positions 3 and 9)

```
  Frame 1 : 01011000  -> MISMATCH (error detected)   [1 bit(s) flipped in this frame - ODD - correctly DETECTED]
  Frame 2 : 01110101  -> MISMATCH (error detected)   [1 bit(s) flipped in this frame - ODD - correctly DETECTED]
Result : ERROR DETECTED
```

How: each frame sees only *one* flip → odd → both detected. Parity is per-frame, so an even total is fine as
long as each frame has an odd share.

### TC5 — Positive: raw bits `b1011`, **odd** parity, 0 flips

```
Original message (m) in binary              : 1011
  Frame 1 (4 data bits) : 10110   (parity bit = 0)
Received data                                : 10110
  Frame 1 : 10110  -> OK
Result : NO ERROR DETECTED
Recovered message (string)                   :
```

How: `1011` has three 1s = already odd → parity 0 keeps it odd. Only 4 data bits, so `bits_to_str` produces
zero characters (needs 8 per char) → empty recovered string. That's expected for short raw-bit inputs.

### TC6 — Edge: invalid flip position (`1` flip, position `50` when max is 18)

```
    -> Invalid position 50, ignored.
…
Result : NO ERROR DETECTED
Recovered message (string)                   : Hi
```

How: the range check skips the flip, `flipped[]` stays empty, `nflips = 0`.

### TC7 — Edge: flip the **parity bit** itself (position 7 in `Hi`)

Frame 1 becomes `01001001`: three 1s → mismatch → detected. Parity protects itself too.

### TC8 — Edge: negative number of flips (`-2`)

`No error introduced.`; the `for` loop runs 0 times because `i < k` is false immediately.

---

## Function 1: `ask_parity_scheme`

```c
static int ask_parity_scheme(void)
```
**What:** asks the user 1 (even) or 2 (odd). **Output:** `0` for even, `1` for odd. **Why:** the scheme is needed by both `parity_bit()` and `no_error()`; encoding it as 0/1 makes the arithmetic below trivial. **Where called:** once at the start of `parity()`.

```c
    int choice;
    printf("Choose parity scheme -> 1: Even Parity   2: Odd Parity : ");
    scanf("%d", &choice);
    return choice == 2;
```
`choice == 2` is 1 (true) only if the user typed 2; everything else (1, 0, 99…) is treated as even.

---

## Function 2: `parity_bit`

```c
static char parity_bit(int ones, int scheme)
```
**What:** computes the parity bit to append. **Input:** `ones` = number of 1s in the data bits; `scheme` 0/1. **Output:** the character `'0'` or `'1'`. **Where called:** sender loop in `parity()`.

```c
    return '0' + ((ones % 2) ^ scheme);
```
- `ones % 2` = 1 if the count is odd, 0 if even.
- Even parity (`scheme = 0`): we need the total to be even, so the parity bit must equal `ones % 2` (add a 1 if the count is odd). `x ^ 0 = x`.
- Odd parity (`scheme = 1`): we need the opposite. `x ^ 1` flips it.
- `'0' + 0` = `'0'`, `'0' + 1` = `'1'` — character arithmetic to get a printable digit.

Example: frame 2 of `Hi`, `0011010` has 3 ones. Even: `1 ^ 0 = 1` → `'1'`. Odd: `1 ^ 1 = 0` → `'0'`.

---

## Function 3: `no_error`

```c
static int no_error(int total_ones, int scheme)
```
**What:** receiver-side check. **Input:** count of 1s in data + parity bit; scheme. **Output:** 1 if the parity is as expected, 0 if not. **Where called:** receiver loop in `parity()`.

```c
    return total_ones % 2 == scheme;
```
Even parity expects an even total (`% 2 == 0`); odd parity expects `% 2 == 1`. Since `scheme` is exactly 0 or 1, one comparison covers both.

---

## Function 4: `simulate_multi_error`

```c
static void simulate_multi_error(char *bits, int *flipped, int *nflips)
```
**What:** asks how many bits to flip and which positions, flips them, and **records** the valid positions so the receiver report can say how many flips hit each frame.
**Input:** `bits` (transmitted string, modified in place), `flipped` (output array of positions), `nflips` (output count).
**Why:** the shared `simulate_error()` in `utils.c` flips only one bit; parity needs several to demonstrate the even-flip weakness.
**Where called:** receiver side of `parity()`.

```c
    int len = strlen(bits), k, count = 0;
    printf("\nSimulate transmission error(s)\n");
    printf("How many bits do you want to flip? (0 for NO error): ");
    scanf("%d", &k);
    if (k <= 0)
        printf("No error introduced.\n");
```
Read `k`. Zero or negative → message (the loop below will simply not run).

```c
    for (int i = 0; i < k; i++) {
        int pos;
        printf("  Enter bit position #%d to flip (0 to %d): ", i + 1, len - 1);
        scanf("%d", &pos);
        if (pos >= 0 && pos < len) {
            bits[pos] = (bits[pos] == '0') ? '1' : '0';
            flipped[count++] = pos;
            printf("    -> Bit at position %d flipped.\n", pos);
        } else
            printf("    -> Invalid position %d, ignored.\n", pos);
    }
    *nflips = count;
```
`k` times: read a position; if valid, toggle the bit and remember the position (`count` only grows for valid
ones, so `*nflips` is the number of *actual* flips). Flipping the same position twice would undo it — the
program does not guard against that, it just records both.

---

## Function 5: `parity` (main flow)

```c
void parity(void)
```
**What:** full sender → channel → receiver demo. **Where called:** `main.c` case 1. Declared in `ex2.h`.

### Declarations

```c
    char input[MAX_STR], msg[MAX_BITS], transmitted[MAX_BITS], received[MAX_BITS];
    char frame_str[9], recovered[MAX_BITS], recovered_str[MAX_STR];
    int flipped[MAX_BITS], nflips = 0;
    int len, scheme, t = 0, frame_no = 1, rlen = 0, error_found = 0, silent = 0;
```
- `input` raw text; `msg` its bits; `transmitted` frames + parity bits; `received` copy after noise.
- `frame_str` a scratch buffer to print one frame (max 7+1 = 8 chars + `'\0'`).
- `recovered` data bits with parity removed; `recovered_str` as text.
- `flipped`/`nflips` positions actually flipped.
- `len` = number of message bits; `scheme` 0/1; `t` = write index into `transmitted`; `frame_no` for display;
  `rlen` = write index into `recovered`; `error_found` = any frame mismatched; `silent` = some frame had an
  even, undetected flip count.

### Sender

```c
    printf("\n---- PARITY (7-bit frames) : SENDER SIDE ----\n");
    printf("Enter string message (or b<binary> for binary): ");
    scanf("%s", input);
    scheme = ask_parity_scheme();
    input_to_bits(input, msg);
    len = strlen(msg);
    printf("Original message (m) in binary              : %s\n", msg);
    printf("Message split into 7-bit frames, each followed by its own parity bit:\n");
```
Read message and scheme; convert to bits (text or `b…`); print.

```c
    for (int i = 0; i < len; i += 7, frame_no++) {
```
`i` is the start of the current frame in `msg`; jump 7 at a time; number frames from 1.

```c
        int fsize = (len - i < 7) ? len - i : 7, ones = 0;
```
Frame size is 7, except the last frame which gets whatever is left (`len - i`). For `Hi` (16 bits): 7, 7, 2.

```c
        for (int j = 0; j < fsize; j++) {
            transmitted[t + j] = msg[i + j];
            ones += msg[i + j] == '1';
        }
```
Copy the frame's data bits to the output at position `t` and count the 1s (`== '1'` gives 1 or 0).

```c
        transmitted[t + fsize] = parity_bit(ones, scheme);
```
Append the parity bit right after the data bits.

```c
        copy_bits(frame_str, transmitted + t, fsize + 1);
        printf("  Frame %d (%d data bits) : %s   (parity bit = %c)\n",
               frame_no, fsize, frame_str, transmitted[t + fsize]);
        t += fsize + 1;
    }
    transmitted[t] = '\0';
    printf("Data to be sent (all frames concatenated)   : %s\n", transmitted);
```
Copy the frame (data + parity) into `frame_str` just for printing; advance `t` by `fsize + 1`. After the loop
terminate the string and print the whole transmitted stream (19 bits for `Hi`: 8 + 8 + 3).

### Channel

```c
    printf("\n---- PARITY : RECEIVER SIDE ----\n");
    strcpy(received, transmitted);
    simulate_multi_error(received, flipped, &nflips);
    printf("Received data                                : %s\n", received);
    printf("\nNote: a frame's parity only catches an ODD number of bit flips …");
```
Copy → corrupt → show → print the educational note.

### Receiver

```c
    printf("\nChecking each frame:\n");
    frame_no = 1;
    for (int i = 0, pos = 0; i < len; i += 7, frame_no++) {
        int fsize = (len - i < 7) ? len - i : 7, ones = 0, injected = 0;
```
Same frame walk as the sender. `i` indexes the *data* bits (to know frame sizes), `pos` indexes the
*received* stream (which has one extra bit per frame). `injected` will count how many recorded flips fell in this frame.

```c
        for (int j = 0; j < fsize; j++) {
            ones += received[pos + j] == '1';
            recovered[rlen++] = received[pos + j];
        }
```
Count the 1s in the data bits **and** copy them into `recovered` (stripping the parity bit).

```c
        int detected = !no_error(ones + (received[pos + fsize] == '1'), scheme);
```
Add the parity bit's value to the count and test. `no_error` returns 1 if fine; `!` turns that into
`detected = 0`. Mismatch → `detected = 1`.

```c
        for (int f = 0; f < nflips; f++)          /* how many flips landed in this frame */
            if (flipped[f] >= pos && flipped[f] <= pos + fsize)
                injected++;
```
Frame occupies positions `pos … pos + fsize` (inclusive, the last one is the parity bit). Count recorded
flips in that range. This is "cheating" — a real receiver doesn't know — but it lets the program explain
what happened.

```c
        copy_bits(frame_str, received + pos, fsize + 1);
        printf("  Frame %d : %s  -> %s", frame_no, frame_str, detected ? "MISMATCH (error detected)" : "OK");
        if (injected == 0)
            printf("\n");
        else if (injected % 2 == 1)
            printf("   [%d bit(s) flipped in this frame - ODD - correctly DETECTED]\n", injected);
        else {
            printf("   [%d bit(s) flipped in this frame - EVEN - NOT DETECTED, parity still matches!]\n", injected);
            silent = 1;
        }
```
Print the frame and verdict. Then the annotation: no flips → nothing; odd → detected (as parity guarantees);
even → undetected, and remember that with `silent = 1`.

```c
        error_found |= detected;
        pos += fsize + 1;
    }
    recovered[rlen] = '\0';
```
`|=` ORs the flag in: once any frame is detected, `error_found` stays 1. Move `pos` past this frame (data +
parity). Terminate the recovered bits.

### Verdict

```c
    if (error_found) {
        printf("\nResult : ERROR DETECTED\n");
        if (silent)
            printf("(Note: some frame(s) above were also silently corrupted …)\n");
        return;
    }
```
At least one mismatch → report and stop. If in addition some other frame had an even flip count, say so.

```c
    printf("\nResult : NO ERROR DETECTED\n");
    bits_to_str(recovered, recovered_str);
    printf("Recovered message (string)                   : %s\n", recovered_str);
    if (silent)
        printf("WARNING: %d bit(s) were actually flipped … may NOT be the original message!\n", nflips);
```
No frame mismatched → rebuild the text. If flips did occur but were all even-per-frame, warn that the text
may be wrong (TC3).

---

## `#include` lines

- `<stdio.h>` — I/O. `<string.h>` — `strlen`, `strcpy`. `"ex2.h"` — `MAX_STR`, `MAX_BITS`, and the helpers from `utils.c` (`input_to_bits`, `bits_to_str`, `copy_bits`).

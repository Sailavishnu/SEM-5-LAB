# bit_stuffing.c — Bit Stuffing / De-stuffing with HDLC-style flags

## What the file is about

In bit-oriented framing (HDLC), a frame starts and ends with the **flag** pattern `01111110` (six 1s in a
row). If the *data* itself ever contained six consecutive 1s, the receiver would mistake it for a flag and
cut the frame in the wrong place. **Bit stuffing** prevents this: the sender inserts an extra `0` after every
run of **five** 1s in the data, so six 1s can never appear inside the data part. The receiver does the reverse
(**de-stuffing**): after every five 1s it expects a `0`, removes it, and continues.

This file simulates the whole thing:

1. Convert the typed text to bits (using `../convert/convert.c`).
2. Stuff the bits.
3. Add the flag at both ends (framing).
4. Optionally flip one bit to simulate noise.
5. Receiver checks flags, de-stuffs, compares with original, rebuilds text.

The sample input from `run.md` is `~~z` — chosen because `'~'` = `01111110` is exactly the flag pattern, which
forces stuffing to happen.

---

## Test cases (whole program)

Compile with `gcc *.c ../convert/convert.c -o ex1` inside `ex1/`, run, choose `1`.

### TC1 — Positive: `~~z`, no bit flip

Input: `~~z`, flip? `0`

```
Input String : ~~z
Original Bin : 01111110 01111110 01111010
Stuffed Bin  : 01111101 00111110 10011110 10
Framed Bin   : 01111110 01111101 00111110 10011110 10011111 10
--- RECEIVER SIDE ---
Destuffed data: 01111110 01111110 01111010
Output Text  : ~~z
Data matches perfectly!
```

How: `~` = 126 = `01111110` has six 1s. While scanning, after the 5th consecutive 1 the code appends a `0`
(so `011111` → `0111110`), resets the counter, and the real 6th `1` follows. So `01111110` becomes
`011111 0 10` (9 bits). Three chars = 24 bits become 26 stuffed bits (two runs of five 1s — one in each `~`;
`z` = `01111010` has only four 1s in a row, so no stuff). Add 8-bit flags → 42 bits. Receiver sees both flags,
strips the two stuffed 0s, gets back the original 24 bits, `memcmp` matches → prints text.

### TC2 — Negative: `~~z`, flip index 10 (a data bit)

Input: `~~z`, flip? `1`, index `10`

```
Bit at index 10 has been flipped successfully!
Corrupted Bin: 01111110 01011101 00111110 10011110 10011111 10
--- RECEIVER SIDE ---
Actual data:    01111110 01111101 00111110 10011110 10011111 10
Changed data:   01111110 01011101 00111110 10011110 10011111 10
Both don't match, so message discarded.
```

How: index 10 is the third bit of the first stuffed byte (`01111101` → `01011101`). Flags are intact, so the
receiver de-stuffs. Now the run of five 1s is broken (only `011` then `11101`), so no 0 gets removed there;
the de-stuffed stream has 25 bits instead of 24 (`j != n`) and its contents differ → `receive()` returns 1 →
"discarded".

### TC3 — Positive: a character with no long run of 1s: `A`

```
Original Bin : 01000001
Stuffed Bin  : 01000001
Framed Bin   : 01111110 01000001 01111110
Destuffed data: 01000001
Output Text  : A
```

How: `A` = `01000001` never reaches five 1s, so stuffing changes nothing; framing just wraps it in flags.

### TC4 — Negative: flip index 0 (inside the start flag)

```
Corrupted Bin: 11111110 01000001 01111110
Actual data:    01111110 01000001 01111110
Changed data:   11111110 01000001 01111110
Both don't match, so message discarded.
```

How: the first `for` loop in `receive()` compares every flag bit; `framed[0]` is now 1 but `flag[0]` is 0 →
`error = 1`. The de-stuffing loop is skipped entirely (its condition contains `!error`).

### TC5 — Edge: invalid flip index (e.g. `99` when max is 23)

```
Enter bit index to flip (0 to 23): Invalid index. Proceeding without bit flip.
--- RECEIVER SIDE ---
Destuffed data: 01000001
Output Text  : A
Data matches perfectly!
```

How: the `if (flipIndex >= 0 && flipIndex < framedLen)` guard fails, nothing is flipped, and the receiver
sees a clean frame.

### TC6 — Edge: flipping the stuffed 0 itself

Input `~`, flip index 14 (the stuffed 0 sits at position 8 + 6 = 14 in the frame):
`Framed = 01111110 011111 0 10 01111110`. Flipping that 0 to 1 makes the receiver see six 1s in the data.
In `receive()`, when `ones == 5` the next bit must be 0; it is 1 → `error = framed[i] != 0` becomes 1 →
discarded. This is exactly the check that makes the scheme robust.

### TC7 — Edge: input containing spaces (e.g. `hi there`)

`scanf("%99s", …)` stops at the first whitespace, so only `hi` is processed; `there` is left in the input
buffer and will be consumed by the **next** `scanf` (the "flip?" prompt gets `there`, fails to parse a number,
and `wantToFlip` stays 0). So the program continues without flipping. Not a crash, but a quirk of `%s`.

### TC8 — Edge: flipping bit index 41 (last bit of end flag) on `~~z`

The second half of the flag comparison `framed[framedLen - FLAG_LEN + i] != flag[i]` catches it → discarded.

---

## Constants and global data

```c
#define MAX_STR  100
#define MAX_BITS 1000
#define FLAG_LEN 8
```
- `MAX_STR` — max characters the user may type (buffer size).
- `MAX_BITS` — max bits in any bit array; 100 chars × 8 bits = 800, plus stuffing (worst case +20 %) plus 16 flag bits, still under 1000.
- `FLAG_LEN` — the flag is 8 bits long. A named constant avoids "magic numbers".

```c
static const int flag[FLAG_LEN] = {0, 1, 1, 1, 1, 1, 1, 0};
```
The HDLC flag `01111110` stored one bit per int. `const` = never modified. `static` = visible only inside this
file (so `byte_stuffing.c` could have its own `flag` without a clash).

```c
static int data[MAX_BITS], stuffed[MAX_BITS], destuffed[MAX_BITS];
```
Three bit arrays shared between `bitStuffing()` and `receive()`:
- `data` — original bits of the message,
- `stuffed` — after inserting 0s,
- `destuffed` — what the receiver reconstructs.
They are file-level (not local) because `receive()` needs to read `data` and write `destuffed`, and because
1000-int arrays on the stack three times over is wasteful.

---

## Function 1: `printBits`

```c
static void printBits(const char *label, const int *arr, int n)
```

**What it does:** prints a label followed by `n` bits, inserting a space after every 8 bits for readability.

**Input:** `label` (text prefix), `arr` (bit array of 0/1 ints), `n` (how many bits).

**Output:** one line on the screen, e.g. `Original Bin : 01111110 01111110 01111010`.

**Why needed:** used seven times in this file; putting the formatting in one place keeps the output consistent.

**Where called:** only inside `bitStuffing()` (Original, Stuffed, Framed, Corrupted, Actual, Changed, Destuffed lines).

**Line by line:**

```c
    printf("%s", label);
```
Print the prefix string exactly as given (the caller includes the `": "`).

```c
    for (int i = 0; i < n; i++)
        printf("%d%s", arr[i], ((i + 1) % 8 == 0 && i != n - 1) ? " " : "");
```
For each bit print the digit (`%d` of 0 or 1) and then either a space or nothing:
- `(i + 1) % 8 == 0` — true after the 8th, 16th, 24th … bit;
- `i != n - 1` — but not after the very last bit (avoids a trailing space);
- ternary picks `" "` or `""`.
That is why the framed output reads `01111110 01111101 ... 10` in groups of 8.

```c
    printf("\n");
```
End the line.

---

## Function 2: `receive`

```c
static int receive(const int *framed, int framedLen, int n, int *destuffedLen)
```

**What it does:** simulates the receiver: (a) verifies the start and end flags, (b) removes every stuffed 0,
(c) compares the result with the original `data`.

**Input:**
- `framed` — the received frame (possibly corrupted),
- `framedLen` — its length in bits,
- `n` — number of original data bits (to compare against),
- `destuffedLen` — output parameter: how many bits came out of de-stuffing.

**Output:** returns `1` if **any** error is found (bad flag, missing stuffed 0, wrong length, or content mismatch), `0` if the message is perfect. Also fills the global `destuffed[]`.

**Why needed:** separates the receiver's logic from the sender's so the flow in `bitStuffing()` reads top-to-bottom like the real protocol.

**Where called:** once, in `bitStuffing()` after the optional bit flip.

**Line by line:**

```c
    int error = 0, ones = 0, j = 0;
```
`error` — flag that becomes 1 on the first problem. `ones` — counter of consecutive 1s seen so far. `j` — write index into `destuffed[]`.

```c
    for (int i = 0; i < FLAG_LEN; i++)
        if (framed[i] != flag[i] || framed[framedLen - FLAG_LEN + i] != flag[i])
            error = 1;
```
Checks both flags in one loop. `framed[i]` is bit `i` of the start flag. `framed[framedLen - FLAG_LEN + i]`
is bit `i` of the end flag (the last 8 bits). If either differs from the expected `flag[i]`, mark error.
(TC4 and TC8 hit this.)

```c
    for (int i = FLAG_LEN; !error && i < framedLen - FLAG_LEN; i++) {
```
Walk over only the **payload**: from index 8 up to (not including) the last 8 bits. `!error &&` makes the loop
run zero times if a flag was bad — no point de-stuffing garbage.

```c
        if (ones == 5) {                 /* the bit after five 1s must be a stuffed 0 */
            error = framed[i] != 0;
            ones = 0;
            continue;
        }
```
If we have just seen five 1s, the current bit must be the stuffed 0. `framed[i] != 0` evaluates to 1 (error)
if it is a 1 (TC6), 0 otherwise. Either way reset the run counter and `continue` (skip storing it — the
stuffed bit is *not* data).

```c
        destuffed[j++] = framed[i];
        ones = framed[i] ? ones + 1 : 0;
```
Normal bit: store it, and update the run counter — a 1 extends the run, a 0 resets it to zero. The condition
`framed[i]` is true for 1, false for 0.

```c
    }
    *destuffedLen = j;
```
Report how many bits were recovered.

```c
    return error || j != n || memcmp(destuffed, data, n * sizeof(int)) != 0;
```
Three ways to fail, combined with `||` (short-circuit: stops at the first true):
1. `error` — flag or stuffing violation;
2. `j != n` — wrong number of bits recovered (TC2);
3. `memcmp(...) != 0` — same count but different contents. `memcmp` compares `n * sizeof(int)` raw bytes of
   the two int arrays; 0 means identical.
Any true → return 1 (error). All false → return 0.

---

## Function 3: `bitStuffing` (the main flow)

```c
void bitStuffing(void)
```

**What it does:** runs the entire demo: input → bits → stuff → frame → (corrupt) → receive → output.

**Input:** from keyboard: a string, a yes/no, optionally an index.

**Output:** the console trace shown in the test cases.

**Why needed:** it is the entry point for menu option 1.

**Where called:** `ex1/main.c` → `case 1: bitStuffing();`. Declared in `ex1.h`.

**Line by line:**

```c
    char inputString[MAX_STR], bin_str[9], outputString[MAX_STR];
```
`inputString` — what the user typed. `bin_str` — 9-byte scratch buffer for one character's 8 bits + `'\0'`
(required size for `ascii_to_bin`). `outputString` — rebuilt text at the end.

```c
    int ascii_arr[MAX_STR], out_ascii_arr[MAX_STR], framed[MAX_BITS], backup[MAX_BITS];
```
`ascii_arr` — ASCII codes of input. `out_ascii_arr` — ASCII codes recovered at the receiver. `framed` — the
full frame. `backup` — untouched copy of the frame, so that after corruption we can print "Actual" vs "Changed".

```c
    int str_len, n = 0, ones = 0, stuffedLen = 0, framedLen, destuffedLen;
    int wantToFlip = 0, flipIndex = 0, chars = 0;
```
Counters: `str_len` chars typed, `n` data bits, `ones` consecutive-1 counter for the sender, `stuffedLen`,
`framedLen`, `destuffedLen` lengths of the respective arrays, `wantToFlip`/`flipIndex` user's corruption
choice, `chars` number of characters rebuilt at the end.

```c
    printf("\n--- BIT STUFFING CONFIGURATION ---\n");
    printf("Enter the data input string: ");
    scanf("%99s", inputString);
```
`%99s` reads one whitespace-delimited word and refuses to store more than 99 characters (+1 for `'\0'` = the
100-byte buffer) — this prevents a buffer overflow. Side effect: spaces end the input (TC7).

```c
    str_to_ascii(inputString, ascii_arr, &str_len);
```
Get the ASCII code of each character and the count (from `convert.c`).

```c
    for (int i = 0; i < str_len; i++) {
        ascii_to_bin((char)ascii_arr[i], bin_str);
        for (int b = 0; b < 8; b++)
            data[n++] = bin_str[b] == '1';
    }
```
For every character: convert it to `"01111110"` text, then copy each of the 8 characters into `data[]` as an
integer 0/1 (`bin_str[b] == '1'` is 1 for `'1'`, 0 for `'0'`). `n++` advances the bit count. After this,
`n = str_len * 8`.

```c
    printf("\n--- TRANSMITTER SIDE ---\n");
    printf("Input String : %s\n", inputString);
    printBits("Original Bin : ", data, n);
```
Show what we start with.

```c
    for (int i = 0; i < n; i++) {
        stuffed[stuffedLen++] = data[i];
        ones = data[i] ? ones + 1 : 0;
        if (ones == 5) {
            stuffed[stuffedLen++] = 0;
            ones = 0;
        }
    }
```
**The stuffing algorithm.** For each data bit:
1. copy it to `stuffed[]`;
2. if it is 1, `ones` grows; if 0, `ones` resets;
3. the moment `ones` hits 5, append an extra `0` and reset the counter.
Trace on `01111110`: bits 0,1,1,1,1,1 → after the fifth 1, `ones == 5`, so a 0 is appended → `0111110`. Then the
real bit 1 comes (`ones` becomes 1), then 0 (`ones` = 0). Result `011111010`, 9 bits. Resetting to 0 (not
continuing to count) is important: `1111111111` (ten 1s) must become `11111 0 11111 0`, two stuffs.

```c
    printBits("Stuffed Bin  : ", stuffed, stuffedLen);
```
Show the stuffed stream.

```c
    memcpy(framed, flag, sizeof flag);
    memcpy(framed + FLAG_LEN, stuffed, stuffedLen * sizeof(int));
    memcpy(framed + FLAG_LEN + stuffedLen, flag, sizeof flag);
    framedLen = stuffedLen + 2 * FLAG_LEN;
```
**Framing** = flag + stuffed + flag, built with three `memcpy` calls:
- first copies the 8-int flag to the start (`sizeof flag` = 8 × sizeof(int) bytes);
- second copies the stuffed bits right after it (`framed + FLAG_LEN` is pointer arithmetic: 8 ints further);
- third copies the flag again after the data.
`framedLen` = data + two flags.

```c
    printBits("Framed Bin   : ", framed, framedLen);
    memcpy(backup, framed, framedLen * sizeof(int));
```
Print the frame and keep an intact copy for the "Actual data" line later.

```c
    printf("\nDo you want to flip a bit to simulate a network error? (1 = Yes, 0 = No): ");
    scanf("%d", &wantToFlip);
    if (wantToFlip == 1) {
        printf("Enter bit index to flip (0 to %d): ", framedLen - 1);
        scanf("%d", &flipIndex);
        if (flipIndex >= 0 && flipIndex < framedLen) {
            framed[flipIndex] = !framed[flipIndex];
            printf("Bit at index %d has been flipped successfully!\n", flipIndex);
            printBits("Corrupted Bin: ", framed, framedLen);
        } else
            printf("Invalid index. Proceeding without bit flip.\n");
    }
```
Optional noise injection. `!framed[flipIndex]` turns 0 into 1 and 1 into 0 (logical NOT). The range check
prevents writing outside the array (TC5). Anything other than `1` for `wantToFlip` skips the block.

```c
    printf("\n--- RECEIVER SIDE ---\n");
    if (receive(framed, framedLen, n, &destuffedLen)) {
        printBits("Actual data:    ", backup, framedLen);
        printBits("Changed data:   ", framed, framedLen);
        printf("Both don't match, so message discarded.\n");
        return;
    }
```
Hand the frame to the receiver. Non-zero return = error → show the clean frame vs the received one and stop
(`return` leaves the function, control goes back to the menu).

```c
    printBits("Destuffed data: ", destuffed, destuffedLen);
```
No error: show the recovered bit stream.

```c
    for (int i = 0; i < destuffedLen; i += 8) {     /* every 8 bits -> one character */
        for (int b = 0; b < 8; b++)
            bin_str[b] = '0' + destuffed[i + b];
        bin_str[8] = '\0';
        out_ascii_arr[chars++] = bin_to_ascii(bin_str);
    }
```
Regroup bits into bytes. Outer loop jumps 8 at a time. Inner loop builds the text `"01000001"`:
`'0' + 0` = `'0'`, `'0' + 1` = `'1'` (character arithmetic). Terminate the string, then `bin_to_ascii` turns it
into the character code, stored in `out_ascii_arr`.

```c
    ascii_to_str(out_ascii_arr, chars, outputString);
    printf("Output Text  : %s\n", outputString);
    printf("Data matches perfectly!\n");
```
Convert the codes back into a printable string and show success.

---

## `#include` lines

```c
#include <stdio.h>      // printf, scanf
#include <string.h>     // memcpy, memcmp
#include "../convert/convert.h"   // ascii_to_bin, bin_to_ascii, str_to_ascii, ascii_to_str
#include "ex1.h"        // prototype of bitStuffing (so main.c and this file agree)
```

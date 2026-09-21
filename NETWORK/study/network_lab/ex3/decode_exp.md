# decode.c (ex3) — Hamming code checking and correction (receiver side)

## What the file is about

The receiver gets a code word (typed as a string from position N down to position 1), and:

1. works out how many parity bits it contains (p = smallest with 2^p ≥ totalLen + 1),
2. re-runs every parity check P_k over its group — **including the parity bit itself** this time,
3. each failing check contributes its position value (1, 2, 4, 8 …) to the **syndrome**,
4. the syndrome **is** the position of the wrong bit (0 = no error) — flip it,
5. strips the parity positions and prints the original data.

This works because each position j is covered exactly by the parity bits whose position values add up to j.
Flip position 6 = 4 + 2 → checks P3 (4) and P2 (2) fail → syndrome 4 + 2 = 6.

---

## Test cases (whole program)

Build `gcc *.c -o ex3`, choose `2`.

### TC1 — Positive: `10101010000`, even (the word produced by encoding `1011010`)

```
Checking P4 ... = 0
Checking P3 ... = 0
Checking P2 ... = 0
Checking P1 ... = 0
Syndrome (binary value of check bits) = 0
No error detected.
Original Data: 1011010
```

How: every group has an even number of 1s → every check bit 0 → syndrome 0 → nothing flipped → data bits at
positions 11,10,9,7,6,5,3 read out as `1011010`.

### TC2 — Positive (single error corrected): `10101110000`, even — position 6 flipped

```
Checking P4 ... = 0
Checking P3 ... = 1
Checking P2 ... = 1
Checking P1 ... = 0
Syndrome (binary value of check bits) = 6
Error found at Position 6 -> flipping bit to correct.
Corrected Code … Value:  1  0  1  0  1  0  1  0  0  0  0
Original Data: 1011010
```

Hand trace (positions 11…1 = `1 0 1 0 1 1 1 0 0 0 0`):

| Check | positions covered | bits | count | bit | adds |
|-------|-------------------|------|-------|-----|------|
| P4 (8) | 8,9,10,11 | 0,1,0,1 | 2 | 0 | 0 |
| P3 (4) | 4,5,6,7 | 0,1,1,1 | 3 | 1 | 4 |
| P2 (2) | 2,3,6,7,10,11 | 0,0,1,1,0,1 | 3 | 1 | 2 |
| P1 (1) | 1,3,5,7,9,11 | 0,0,1,1,1,1 | 4 | 0 | 0 |

Syndrome 4 + 2 = 6 → `code[6]` toggled 1 → 0 → the original word is back.

### TC3 — Positive with odd parity: `1011110`, odd (encoding of `1011`)

```
Checking P3 ... = 0 / P2 ... = 0 / P1 ... = 0
Syndrome = 0 → No error detected.
Original Data: 1011
```

How: with odd parity each group must have an odd count; `bit = (count % 2) ^ 1` is 0 when the count is odd.

### TC4 — Positive: parity bit itself corrupted: `1011111`, odd (P1 flipped)

```
Checking P1 ... = 1
Syndrome = 1
Error found at Position 1 -> flipping bit to correct.
Original Data: 1011
```

How: only P1's group changed parity → syndrome 1 → position 1 is P1 → flipped back. Data unaffected either way.

### TC5 — Negative (limitation): **two** errors: `01101010000`, even (positions 11 and 10 flipped)

```
Checking P4 ... = 0, P3 ... = 0, P2 ... = 0, P1 ... = 1
Syndrome = 1
Error found at Position 1 -> flipping bit to correct.
Original Data: 0111010      ← WRONG (original was 1011010)
```

How: syndrome = 11 XOR 10 = `1011 ^ 1010` = `0001` = 1. Hamming can only correct **one** error; two errors
produce a syndrome pointing at an innocent position, and "correcting" it makes a third error. The program
cannot know this happened (a real system adds an overall parity bit — SECDED — to detect the double error).

### TC6 — Edge: syndrome larger than the code length: `10101`, even (5-bit word)

```
Syndrome = 7
Error position out of range - more than 1 bit may be corrupted.
Original Data: 11
```

How: for totalLen = 5, p = 3 so the syndrome can be up to 7, but position 7 doesn't exist → the program
reports that it must be a multi-bit error and leaves the word untouched.

### TC7 — Positive: run.md's sample `10101010111`, even

```
Syndrome = 0 → No error detected.
Original Data: 1011011
```

How: it is a *valid* code word — exactly what encoding `1011011` with even parity produces (P1 = 1, P2 = 1,
P3 = 0, P4 = 0). All four groups have an even count, so no correction happens. Not every
"interesting-looking" word has an error.

### TC8 — Negative: non-binary input `10x01`

`'x' - '0'` = 72 is stored as a bit value; counts become meaningless and the syndrome is garbage. Not validated.

### TC9 — Edge: 1-bit word `1`, even

p = 1 (2 ≥ 2). P1 covers {1}: count 1 → bit 1 → syndrome 1 → "Error at position 1", flipped to 0.
Original data: nothing (position 1 is parity) → prints an empty line. Degenerate but doesn't crash.

---

## Function 1: `detectError`

```c
static int detectError(int totalLen, int p, int parityType)
```
**What:** recomputes all p parity checks and returns the syndrome.
**Input:** `totalLen`, `p`, `parityType`. Reads `code[]`. **Output:** syndrome (0 … 2^p − 1).
**Why:** the core of error detection/location. **Where called:** `checkReceivedCode()`.

```c
    int syndrome = 0;
    printf("\nChecking received code:\n");
    for (int k = p; k >= 1; k--) {
```
Iterate from P_p down to P1 — purely cosmetic, so the printed check bits read as a binary number from the
most significant (P4) to the least (P1).

```c
        int pos = 1 << (k - 1), count = 0, bit;
        if (pos > totalLen)
            continue;
        for (int j = 1; j <= totalLen; j++)
            if (j & pos)
                count += code[j];              /* parity bit itself is included */
```
Same coverage rule as the encoder (`j & pos`), but the parity bit's own position **is** included this time:
at the receiver we check that the *whole* group (data + parity) has the right parity.

```c
        bit = (count % 2) ^ (parityType != 0);
        printf("Checking P%d ... = %d\n", k, bit);
        syndrome += bit * pos;
    }
```
Even parity: the group should have an even count → `count % 2` should be 0; if it's 1 the check fails →
`bit = 1`. Odd parity: XOR with 1 flips the meaning. A failed check adds its position value (`pos`) to the
syndrome: `bit * pos` is either `pos` or 0.

```c
    printf("\nSyndrome (binary value of check bits) = %d\n", syndrome);
    return syndrome;
```

---

## Function 2: `correctError`

```c
static void correctError(int errorPos, int totalLen)
```
**What:** interprets the syndrome, flips the bit if it points to a real position, prints the corrected frame.
**Input:** `errorPos` (the syndrome), `totalLen`. Modifies `code[]`.
**Where called:** `checkReceivedCode()`.

```c
    if (errorPos == 0)
        printf("\nNo error detected.\n");
```
Syndrome 0 → all checks passed.

```c
    else if (errorPos > totalLen)
        printf("\nError position out of range - more than 1 bit may be corrupted.\n");
```
Syndrome points past the end of the word → impossible for a single error → must be ≥ 2 errors (TC6).

```c
    else {
        printf("\nError found at Position %d -> flipping bit to correct.\n", errorPos);
        code[errorPos] = !code[errorPos];
    }
```
Otherwise toggle the bit at that position (`!` turns 0→1, 1→0).

```c
    printf("\nCorrected Code\n");
    displayFrame(totalLen);
    printf("\n");
```
Show the (possibly unchanged) word.

---

## Function 3: `extractData`

```c
static void extractData(int totalLen)
```
**What:** prints the data bits only (skipping parity positions), high position first.
**Where called:** end of `checkReceivedCode()`.

```c
    printf("\nOriginal Data: ");
    for (int j = totalLen; j >= 1; j--)
        if (!isPowerOf2(j))
            printf("%d", code[j]);
    printf("\n");
```
Same order the encoder used to place them, so the output string equals what the sender typed.

---

## Function 4: `checkReceivedCode` (entry point)

```c
void checkReceivedCode(const char *input, int parityType)
```
**What:** parses the received string into `code[]`, then detect → correct → extract.
**Input:** received bit string (position N first), parity type. **Where called:** `main.c` when choice = 2. Declared in `hamming.h`.

```c
    int totalLen = strlen(input), p = 0;
    while ((1 << p) < totalLen + 1)
        p++;
```
Number of parity bits present: the smallest p with 2^p ≥ totalLen + 1 (the same inequality the encoder used,
solved for p given the total). For 11 → p = 4; 7 → 3; 5 → 3; 3 → 2.

```c
    assignLabels(totalLen, p);
    for (int j = totalLen, idx = 0; j >= 1; j--, idx++) {
        code[j] = input[idx] - '0';
        filled[j] = 1;
    }
```
Label positions; copy the string into `code[]` so that `input[0]` lands in the highest position (matching the
encoder's "Data to be transmitted" order). Everything is known, so `filled` = 1 everywhere.

```c
    printf("\nReceived Code\n");
    displayFrame(totalLen);
    correctError(detectError(totalLen, p, parityType), totalLen);
    extractData(totalLen);
```
Show the received word; compute the syndrome and hand it straight to `correctError`; print the data.

---

## `#include` lines

- `<stdio.h>` — `printf`. `<string.h>` — `strlen`. `"hamming.h"` — globals and shared helpers.

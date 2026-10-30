# encode.c (ex3) — Hamming code generation (sender side)

## What the file is about

Given *n* data bits, the encoder:

1. **Step 1** — counts the data bits.
2. **Step 2** — finds the smallest number of parity bits *p* such that `n + p + 1 ≤ 2^p` (each parity
   combination must be able to point at any of the n + p positions, plus "no error").
3. **Step 3** — lays the data bits into the non-power-of-2 positions, leaving positions 1, 2, 4, 8, … empty.
4. **Step 4** — computes each parity bit P_k over every position whose number has bit *k* set
   (P1 covers 1,3,5,7,9,11…; P2 covers 2,3,6,7,10,11…; P3 covers 4–7, 12–15…; P4 covers 8–15…).
5. Prints the final code word from the highest position down to position 1.

Even parity: P_k makes the number of 1s in its group even. Odd parity: odd.

---

## Test cases (whole program)

Build `gcc *.c -o ex3` in `ex3/`, choose `1`.

### TC1 — Positive (run.md sample): data `1011010`, even parity

```
Step 1: Number of data bits (n) = 7
Step 2:
Try p = 1   7 + 1 + 1 <= 2^1  -> No
Try p = 2   7 + 2 + 1 <= 2^2  -> No
Try p = 3   7 + 3 + 1 <= 2^3  -> No
Try p = 4   7 + 4 + 1 <= 2^4  -> Yes
Required parity bits = 4
Total bits = 11
Step 3:
Position :  11 10  9  8  7  6  5  4  3  2  1
Type     :  D7 D6 D5 P4 D4 D3 D2 P3 D1 P2 P1
Value    :   1  0  1  _  1  0  1  _  0  _  _
Step 4: (Even parity)
Calculate P1  positions 1 3 5 7 9 11   values _ 0 1 1 1 1   → P1 = 0
Calculate P2  positions 2 3 6 7 10 11  values _ 0 0 1 0 1   → P2 = 0
Calculate P3  positions 4 5 6 7        values _ 1 0 1       → P3 = 0
Calculate P4  positions 8 9 10 11      values _ 1 0 1       → P4 = 0
Final: 1 0 1 0 1 0 1 0 0 0 0
Data to be transmitted: 10101010000
```

How: 9 ≤ 2, 10 ≤ 4, 11 ≤ 8 all fail; 12 ≤ 16 passes → p = 4, total 11. The typed string `1011010` is placed
left-to-right into positions 11,10,9,7,6,5,3 (skipping 8,4,2,1). P1's group {3,5,7,9,11} = 0,1,1,1,1 → four 1s
(even) → P1 = 0. P2's group {3,6,7,10,11} = 0,0,1,0,1 → two 1s → 0. P3 {5,6,7} = 1,0,1 → 0. P4 {9,10,11} =
1,0,1 → 0. All four happen to be 0 for this input.

### TC2 — Positive: data `1011`, **odd** parity

```
Final Hamming Code
Position :   7  6  5  4  3  2  1
Type     :  D4 D3 D2 P3 D1 P2 P1
Value    :   1  0  1  1  1  1  0
Data to be transmitted: 1011110
```

How: n = 4 → p = 3 (4+3+1 = 8 ≤ 8), total 7. Data → positions 7,6,5,3 = 1,0,1,1.
P1 group {3,5,7} = 1,1,1 → three 1s (odd) → for odd parity P1 = 0 (already odd).
P2 group {3,6,7} = 1,0,1 → two 1s → odd parity needs one more → P2 = 1.
P3 group {5,6,7} = 1,0,1 → two → P3 = 1.

### TC3 — Edge: a single data bit `1`, even parity

```
Position :   3  2  1
Type     :  D1 P2 P1
Value    :   1  1  1
Data to be transmitted: 111
```

How: n = 1 → p = 2 (1+2+1 = 4 ≤ 4). D1 at position 3; P1 covers {3} → 1 one → P1 = 1; P2 covers {3} → P2 = 1.
(This is the 3-bit repetition code.)

### TC4 — Edge: data `11`, even → `11110`

n = 2 → p = 3 (2+3+1 = 6 ≤ 8; p = 2 gives 5 ≤ 4, no). Total 5: positions 5,3 hold 1,1. P1 {3,5} = 1,1 → 0;
P2 {3} = 1 → 1; P3 {5} = 1 → 1. Read out 5..1: `1 1 1 1 0`.

### TC5 — Negative: non-binary characters, e.g. `10x1`

`data[idx] - '0'` for `'x'` gives 72 (ASCII 120 − 48). It is stored in `code[]` as 72, printed as `72` in the
Value row, and `count += 72` distorts the parity (72 is even, so it happens not to change the parity, but
`'y'` = 73 would). No validation is performed — the program expects only `0`/`1`.

### TC6 — Negative: empty input (just press Enter, then type something on the next line)

`scanf("%s")` skips whitespace and waits for a token, so an empty line is simply ignored; there is no way to
pass an empty string. If it could, n = 0 → p = 1 (0+1+1 = 2 ≤ 2) → total 1 → a lone P1 = 0.

### TC7 — Edge: parity type other than 0 or 1 (e.g. `5`)

`parityType != 0` is true → treated as **odd**. The heading prints "Odd parity".

### TC8 — Edge: long input, e.g. 26 data bits

p = 5 (26+5+1 = 32 ≤ 32) → 31 bits, the classic (31,26) Hamming code. Position numbers go to 31 and the
`%3d` columns still line up.

---

## Function 1: `findParityBits`

```c
static int findParityBits(int n)
```
**What:** computes and *shows* the search for the smallest p with `n + p + 1 ≤ 2^p`.
**Input:** `n` data bits. **Output:** `p`. **Why:** with p parity bits you can encode 2^p syndromes; one is
"no error", the rest must cover all n + p positions.
**Where called:** `generateHammingCode()` (Step 2).

```c
    int p = 1;
    printf("\nStep 2:\nFinding parity bits (p)\n");
    for (; n + p + 1 > (1 << p); p++)
        printf("\nTry p = %d\n%d + %d + 1 <= 2^%d  -> No\n", p, n, p, p);
```
Start at p = 1. While the inequality **fails** (`>` instead of `≤`), print "No" and increment. `1 << p` = 2^p.
The `for` has an empty initialiser because `p` is declared above.

```c
    printf("\nTry p = %d\n%d + %d + 1 <= 2^%d  -> Yes\n", p, n, p, p);
    printf("\nRequired parity bits = %d\n", p);
    return p;
```
The first p that exits the loop satisfies the condition → print "Yes" and return.

---

## Function 2: `placeDataBits`

```c
static void placeDataBits(const char *data, int totalLen)
```
**What:** copies the typed bits into the data positions and marks parity positions as empty.
**Input:** `data` string, `totalLen`. Writes globals `code[]`, `filled[]`.
**Why:** Step 3 of the algorithm. **Where called:** `generateHammingCode()`.

```c
    int idx = 0;
    for (int j = totalLen; j >= 1; j--) {
        filled[j] = !isPowerOf2(j);
        code[j] = filled[j] ? data[idx++] - '0' : 0;
    }
```
Walk from the highest position downward so that the **first typed bit goes to the highest position**
(`data[0]` → position `totalLen`), like reading a number left to right. `filled[j]` is 1 for data positions
(`!isPowerOf2`), 0 for parity. For data positions take the next character, convert `'0'/'1'` → 0/1 with
`- '0'`, and advance `idx`; parity positions get a placeholder 0 (not shown thanks to `filled = 0`).

---

## Function 3: `calculateParity`

```c
static void calculateParity(int totalLen, int p, int parityType)
```
**What:** computes every parity bit and shows which positions it covers.
**Input:** `totalLen`, `p`, `parityType` (0 even / other odd). Reads/writes `code[]`, `filled[]`.
**Why:** Step 4. **Where called:** `generateHammingCode()`.

```c
    printf("\nStep 4:\nCalculating parity bits (%s parity)\n", parityType == 0 ? "Even" : "Odd");
    for (int k = 1; k <= p; k++) {
        int pos = 1 << (k - 1), count = 0;
        if (pos > totalLen)
            continue;
```
For parity bit k, its position is 2^(k−1): P1→1, P2→2, P3→4, P4→8. If that position is beyond the code word
(can happen when p was rounded up), skip it.

```c
        printf("\nCalculate P%d\n\nChecking positions\n", k);
        for (int j = 1; j <= totalLen; j++)
            if (j & pos)
                printf("%d ", j);
```
**The coverage rule:** P_k checks every position `j` whose binary representation has bit (k−1) set, i.e.
`j & pos` is non-zero. For pos = 1: odd positions. For pos = 2: 2,3,6,7,10,11. For pos = 4: 4–7, 12–15. For
pos = 8: 8–15. This loop only prints them.

```c
        printf("\n\nValues\n");
        for (int j = 1; j <= totalLen; j++) {
            if (!(j & pos))
                continue;
            if (j == pos)
                printf("_ ");
            else {
                printf("%d ", code[j]);
                count += code[j];
            }
        }
```
Second pass over the same positions: print each covered bit and add it to `count`. The parity position
itself (`j == pos`) is shown as `_` and excluded from the count — it is what we are computing.

```c
        code[pos] = (count % 2) ^ (parityType != 0);   /* even: count%2, odd: opposite */
        filled[pos] = 1;
        printf("\n\nP%d = %d\n", k, code[pos]);
    }
```
`count % 2` is 1 if the covered data has an odd number of 1s. Even parity wants the total even, so P = that
value (add a 1 iff currently odd). Odd parity wants the opposite → XOR with 1. `parityType != 0` gives
exactly 0 or 1, so any non-zero parity type means odd. Mark the slot as filled and print.

---

## Function 4: `generateHammingCode` (entry point)

```c
void generateHammingCode(const char *data, int parityType)
```
**What:** runs Steps 1–4 and prints the transmitted word.
**Input:** the data bit string and parity type. **Output:** console. **Where called:** `main.c` when choice = 1. Declared in `hamming.h`.

```c
    int n = strlen(data), p, totalLen;
    printf("\nStep 1:\nNumber of data bits (n) = %d\n", n);
    p = findParityBits(n);
    totalLen = n + p;
    printf("Total bits = %d\n", totalLen);
```
Steps 1–2.

```c
    assignLabels(totalLen, p);
    placeDataBits(data, totalLen);
    printf("\nStep 3:\nInsert empty parity locations\n");
    displayFrame(totalLen);
```
Name the positions (from `common.c`), place the data, show the frame with `_` holes.

```c
    calculateParity(totalLen, p, parityType);
    printf("\nFinal Hamming Code\n");
    displayFrame(totalLen);
```
Step 4, then the completed frame.

```c
    printf("\n\nData to be transmitted: ");
    for (int j = totalLen; j >= 1; j--)
        printf("%d", code[j]);
    printf("\n");
```
Print the bits from position `totalLen` down to 1 as one string — this is the format the decoder expects as input.

---

## `#include` lines

- `<stdio.h>` — `printf`. `<string.h>` — `strlen`. `"hamming.h"` — `code`, `filled`, `isPowerOf2`, `assignLabels`, `displayFrame`.

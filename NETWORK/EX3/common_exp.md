# common.c (ex3) — Shared state and helpers for the Hamming code

## What the file is about

Experiment 3 implements the **Hamming code**, an error-*correcting* code. A code word is laid out in
positions numbered **1 … totalLen** (1-indexed, not 0-indexed, because the maths of Hamming codes depends on
the binary form of the position number):

```
Position :  ... 11 10  9  8  7  6  5  4  3  2  1
Type     :  ... D7 D6 D5 P4 D4 D3 D2 P3 D1 P2 P1
```

- Positions that are **powers of 2** (1, 2, 4, 8, …) hold **parity bits** P1, P2, P3, P4 …
- All other positions hold **data bits** D1, D2, …

`common.c` holds the three global arrays that describe the code word and the three helpers that both the
encoder (`encode.c`) and the decoder (`decode.c`) need: detecting powers of two, labelling positions, and
printing the frame as a table.

---

## Test cases (whole file)

`common.c` has no `main`; it is exercised through `ex3`. The direct function-level cases:

| # | Type | Call | Result | How |
|---|------|------|--------|-----|
| 1 | Positive | `isPowerOf2(1)` | 1 | `1 & 0 == 0`. |
| 2 | Positive | `isPowerOf2(8)` | 1 | `1000 & 0111 = 0`. |
| 3 | Negative | `isPowerOf2(6)` | 0 | `110 & 101 = 100 ≠ 0`. |
| 4 | Edge | `isPowerOf2(0)` | 0 | the `pos > 0` guard; without it `0 & -1 == 0` would wrongly say yes. |
| 5 | Edge | `isPowerOf2(-4)` | 0 | `pos > 0` fails. |
| 6 | Positive | `assignLabels(11, 4)` | label[11..1] = `D7 D6 D5 P4 D4 D3 D2 P3 D1 P2 P1` | walking from 11 down to 1: non-powers get D7, D6, D5, D4, D3, D2, D1 (dNum counts down from 11−4 = 7); powers 8,4,2,1 get P4, P3, P2, P1. |
| 7 | Positive | `assignLabels(7, 3)` | `D4 D3 D2 P3 D1 P2 P1` | dNum starts at 4. |
| 8 | Edge | `assignLabels(3, 2)` (1 data bit) | `D1 P2 P1` | smallest useful code (n = 1). |
| 9 | Positive | `displayFrame(11)` after `placeDataBits` but before parity | prints `_` under 8, 4, 2, 1 and digits elsewhere | `filled[j]` is 0 for parity positions at that stage. |
| 10 | Positive | `displayFrame(11)` after parity is computed | every column shows a digit | `filled[]` is 1 everywhere. |

Program-level (from `ex3`): encoding `1011010` even → `Data to be transmitted: 10101010000`; decoding
`10101110000` even → `Error found at Position 6`. Full traces are in `encode_exp.md` and `decode_exp.md`.

---

## Global variables

```c
int  code[MAX];
int  filled[MAX];
char label[MAX][4];
```
These are the **definitions** (the header only has `extern` declarations — see `hamming_h_exp.md`).

- `code[j]` — the bit (0/1) at position `j`. Index 0 is unused because positions start at 1.
- `filled[j]` — 1 if position `j` has a known value yet, 0 if it is still an empty parity slot. Only used to
  decide whether `displayFrame` prints a digit or `_`.
- `label[j]` — the text `"P1"`, `"D7"`, … for position `j`. 4 chars: letter + up to 2 digits + `'\0'`
  (`MAX = 100` so labels never exceed `D99`).

They are global because encoder, decoder and the display function all read/write the same code word.

---

## Function 1: `isPowerOf2`

```c
int isPowerOf2(int pos)
```
**What:** tells whether a position number is a power of two, i.e. whether it is a **parity position**.
**Input:** position (1-based). **Output:** 1 (yes) or 0 (no).
**Why:** it is *the* rule of Hamming layout: parity bits live at 1, 2, 4, 8, …
**Where called:** `assignLabels()` here, `placeDataBits()` in `encode.c`, `extractData()` in `decode.c`.

```c
    return pos > 0 && (pos & (pos - 1)) == 0;
```
Classic bit trick. A power of two has exactly one 1-bit: `8 = 1000`. Subtracting 1 flips that bit and sets
every bit below it: `7 = 0111`. AND-ing them gives 0. For any other number (e.g. `6 = 110`, `5 = 101`) the
top 1-bit survives the AND, so the result is non-zero. The `pos > 0` guard excludes 0 (which would pass the
bit test) and negatives.

---

## Function 2: `assignLabels`

```c
void assignLabels(int totalLen, int p)
```
**What:** fills `label[]` with `P…`/`D…` names for every position.
**Input:** `totalLen` = total bits in the code word; `p` = number of parity bits.
**Output:** `label[1..totalLen]` filled.
**Why:** for the "Type :" row of the printed table.
**Where called:** `generateHammingCode()` (encode) and `checkReceivedCode()` (decode).

```c
    int dNum = totalLen - p;
```
Number of data bits. Data positions are numbered from the highest (D_n at the top position) down to D1
(position 3), matching textbook figures.

```c
    for (int j = totalLen; j >= 1; j--) {
```
Walk positions from the top down so `dNum` can count downward.

```c
        if (isPowerOf2(j)) {
            int k = 1;
            while ((1 << k) <= j)          /* k = log2(j) + 1 */
                k++;
            sprintf(label[j], "P%d", k);
        }
```
Parity position: find its index k such that 2^(k−1) = j. The loop counts how many powers of two are ≤ j:
for j = 8: `2 ≤ 8` k=2, `4 ≤ 8` k=3, `8 ≤ 8` k=4, `16 ≤ 8` stops → k = 4 → `"P4"`. For j = 1: `2 ≤ 1` false
immediately → k = 1 → `"P1"`. `sprintf` writes formatted text into the label buffer.

```c
        else
            sprintf(label[j], "D%d", dNum--);
```
Data position: name it D`dNum` and decrement, so the next data position (lower) gets the next-lower number.

---

## Function 3: `displayFrame`

```c
void displayFrame(int totalLen)
```
**What:** prints the code word as three aligned rows: Position, Type, Value.
**Input:** `totalLen`. Reads globals `label`, `code`, `filled`.
**Output:** e.g.

```
Position :
 11 10  9  8  7  6  5  4  3  2  1

Type :
 D7 D6 D5 P4 D4 D3 D2 P3 D1 P2 P1

Value :
  1  0  1  _  1  0  1  _  0  _  _
```
**Why:** the whole point of the lab is to *see* where parity bits go and what they become.
**Where called:** twice in `generateHammingCode()` (before and after parity), twice in `checkReceivedCode()`
/ `correctError()` (received and corrected).

```c
    printf("\nPosition :\n");
    for (int j = totalLen; j >= 1; j--)
        printf("%3d", j);
```
`%3d` = right-align in 3 characters, so every column is the same width whatever the number of digits.
Printed from high to low so the leftmost column is the most significant, like a binary number.

```c
    printf("\n\nType :\n");
    for (int j = totalLen; j >= 1; j--)
        printf("%3s", label[j]);
```
`%3s` right-aligns the 2-character labels in 3-wide columns, keeping them under the position numbers.

```c
    printf("\n\nValue :\n");
    for (int j = totalLen; j >= 1; j--) {
        if (filled[j])
            printf("%3d", code[j]);
        else
            printf("  _");
    }
    printf("\n");
```
The bit if known, otherwise an underscore (two spaces + `_` = width 3) for an empty parity slot.

---

## `#include` lines

- `<stdio.h>` — `printf`, `sprintf`. `"hamming.h"` — `MAX`, the `extern` declarations (so the definitions here match) and the prototypes.

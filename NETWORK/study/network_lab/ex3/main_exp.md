# main.c (ex3) — Entry point for the Hamming code program

## What the file is about

Unlike ex1/ex2/ex4/ex5 this `main` has **no loop**: it asks one question set, runs either the encoder or the
decoder once, and exits. (`run.md`: "asks once, then exits".)

Flow: choice (1 = encode, 2 = check/correct) → parity type (0 even, 1 odd) → the bit string → call the
matching function from `encode.c` or `decode.c`.

---

## Test cases (whole program)

Build `gcc *.c -o ex3` in `ex3/`.

| # | Type | Input | Result | How |
|---|------|-------|--------|-----|
| 1 | Positive | `1`, `0`, `1011010` | full encoding trace, `Data to be transmitted: 10101010000`, exit | `choice == 1` → `generateHammingCode(input, 0)`. |
| 2 | Positive | `2`, `0`, `10101110000` | `Error found at Position 6`, corrected, `Original Data: 1011010` | `choice == 2` → `checkReceivedCode(input, 0)`. |
| 3 | Positive | `1`, `1`, `1011` | odd-parity encoding `1011110` | parity type 1 passed through. |
| 4 | Negative | `5`, `0` | `Invalid choice.` — program ends without asking for bits | neither branch matches → `else`. Note the parity type is asked **before** the choice is validated, so you still have to answer it. |
| 5 | Edge | `1`, `7`, `1011` | treated as **odd** parity | every callee tests `parityType != 0`, so any non-zero value means odd. |
| 6 | Edge | `2`, `0`, then a 120-character string | overflows `input[MAX]` (`MAX = 100`) — undefined behaviour | `scanf("%s")` has no width limit. Keep inputs under 99 characters. |
| 7 | Edge | `1`, `0`, `abc` | runs, but with garbage bit values (see `encode_exp.md` TC5) | no validation of the characters. |
| 8 | Edge | non-numeric choice (`x`) | `scanf` fails, `choice` is uninitialised → almost certainly `Invalid choice.` | `%d` fails on a letter; no loop, so no spinning as in other experiments. |

---

## Function: `main`

```c
int main(void)
```
**What:** collects three inputs and dispatches once. **Output:** exit status 0. **Where called:** program start.

```c
#include <stdio.h>
#include "hamming.h"
```
I/O; `MAX` and the two entry-point prototypes.

```c
int main(void) {
    int choice, parityType;
    char input[MAX];
```
`input` holds either the data bits or the received code word; `MAX` (100) comes from `hamming.h`.

```c
    printf("=== HAMMING CODE PROGRAM ===\n");
    printf("1. Encode data (generate Hamming code)\n2. Check received code for error and correct it\n");
    printf("Enter choice: ");
    scanf("%d", &choice);
    printf("Enter parity type (0 = Even, 1 = Odd): ");
    scanf("%d", &parityType);
```
Menu, choice, parity type. Both options need the parity type, so it is asked up front.

```c
    if (choice == 1) {
        printf("Enter data bits (e.g. 1011010): ");
        scanf("%s", input);
        generateHammingCode(input, parityType);
    }
```
Encoder path: read raw data bits and call `encode.c`.

```c
    else if (choice == 2) {
        printf("Enter received code (position N ... position 1, e.g. 10101010111): ");
        scanf("%s", input);
        checkReceivedCode(input, parityType);
    }
```
Decoder path: read the code word (highest position first, exactly the format the encoder printed) and call `decode.c`.

```c
    else
        printf("Invalid choice.\n");
    return 0;
}
```
Anything else → message. Return 0 in all cases.

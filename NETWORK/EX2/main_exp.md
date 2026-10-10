# main.c (ex2) — Menu driver for Parity / CRC / Checksum

## What the file is about

Entry point of experiment 2. It only prints a menu and calls one of the three technique functions
(`parity()`, `crc_technique()`, `checksum_technique()`), whose prototypes come from `ex2.h`. It uses a
`do … while` loop so the menu is shown at least once and repeats until the user picks `4`.

---

## Test cases (whole program)

Compile: `gcc *.c ../convert/convert.c -o ex2` (inside `ex2/`).

| # | Type | Input | Behaviour | How |
|---|------|-------|-----------|-----|
| 1 | Positive | `1` → parity inputs → `4` | Parity demo, menu again, `Exiting...`, exit | `case 1` → `parity()`; loop condition `choice != 4` true → repeat; `4` → `case 4` prints, then condition false → loop ends → `return 0`. |
| 2 | Positive | `2` → CRC inputs → `4` | CRC demo then exit | `case 2` → `crc_technique()`. |
| 3 | Positive | `3` → checksum inputs → `4` | Checksum demo then exit | `case 3` → `checksum_technique()`. |
| 4 | Positive | `1 … 2 … 3 … 4` | all three in one session | loop keeps running until 4. |
| 5 | Negative | `9` | `Invalid choice, try again.` then menu | `default:` branch; `9 != 4` → loop again. |
| 6 | Negative | `0`, `-5` | same invalid message | not 1–4 → `default`. |
| 7 | Edge | `4` immediately | `Exiting...` and program ends | shortest run. |
| 8 | Edge | a letter (`q`) | `scanf` fails, the letter is never consumed, `choice` is whatever it was → the menu scrolls endlessly (Ctrl+C to stop). If it happens on the very first prompt, `choice` is uninitialised — undefined but in practice usually a garbage value → `default` branch, still looping. | `scanf("%d")` doesn't remove non-numeric input from the buffer. |

---

## Function: `main`

```c
int main(void)
```
**What:** menu loop and dispatcher. **Input:** integers from the keyboard. **Output:** exit status 0.
**Why:** one executable for all three techniques. **Where called:** by the C runtime at program start.

```c
#include <stdio.h>
#include "ex2.h"
```
`printf`/`scanf`; and the technique prototypes.

```c
int main(void) {
    int choice;
    do {
```
`do { } while (cond);` runs the body first, then tests the condition — perfect for "show menu, then decide
whether to show it again".

```c
        printf("\n================ ERROR DETECTION TECHNIQUES ================\n");
        printf(" 1. Parity\n 2. CRC\n 3. Checksum\n 4. Exit\n");
        printf("==============================================================\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
```
Menu text and input.

```c
        switch (choice) {
            case 1: parity();             break;
            case 2: crc_technique();      break;
            case 3: checksum_technique(); break;
            case 4: printf("Exiting...\n"); break;
            default: printf("Invalid choice, try again.\n");
        }
```
Dispatch. Note that `case 4` only prints — it does **not** return; leaving the loop is the job of the
`while` condition below. Each `break` leaves the `switch` (not the loop).

```c
    } while (choice != 4);
    return 0;
}
```
Repeat unless the user chose 4. Then return success to the OS.

Difference from ex1's `main`: ex1 uses `while (1)` + `return` inside `case 3`; ex2 uses `do…while` with the
exit condition in the loop test. Both are common styles; ex2's makes the exit condition visible in one place.

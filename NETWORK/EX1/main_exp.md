# main.c (ex1) — Menu driver for Bit Stuffing / Byte Stuffing

## What the file is about

This is the entry point of experiment 1. It contains **only the menu**: it prints three options, reads the
user's choice, and calls the matching technique function. The techniques themselves live in
`bit_stuffing.c` and `byte_stuffing.c`; their prototypes come from `ex1.h`.

The loop repeats forever until the user chooses `3`.

---

## Test cases (whole program)

Compile: `gcc *.c ../convert/convert.c -o ex1` (inside `ex1/`).

| # | Type | Input sequence | What happens | How the code does it |
|---|------|----------------|--------------|----------------------|
| 1 | Positive | `1` → (bit stuffing inputs) → `3` | Menu, bit stuffing demo runs, menu again, `Exiting program`, program ends | `case 1` calls `bitStuffing()`; after it returns the `while(1)` loop prints the menu again; `case 3` prints and `return 0` leaves `main` (and therefore the program). |
| 2 | Positive | `2` → (byte stuffing inputs) → `3` | Byte stuffing demo, then exit | `case 2` calls `byteStuffing()`. |
| 3 | Positive | `1` → … → `2` → … → `3` | Both demos in one session | the infinite loop lets you run any number of techniques. |
| 4 | Negative | `7` | `Invalid selection. Please enter 1, 2, or 3.` then the menu again | `default:` branch of the `switch`; no `return`, so the loop continues. |
| 5 | Negative | `0` or `-1` | same "Invalid selection" message | any integer not 1/2/3 lands in `default`. |
| 6 | Edge | a letter, e.g. `x` | `scanf("%d")` fails, `choice` keeps its previous (uninitialised or old) value, and the `x` stays in the input buffer → the program **spins**: it prints the menu repeatedly because every `scanf` fails on the same `x`. Press Ctrl+C. | `scanf` returns 0 on a non-number and does **not** consume the bad character. `run.md` §4 mentions this ("Program keeps asking for input in a loop"). |
| 7 | Edge | `3` immediately | `Exiting program` | shortest possible run. |

---

## Function: `main`

```c
int main(void)
```

**What it does:** shows the menu in an endless loop and dispatches to the chosen technique.

**Input:** an integer from the keyboard each time round the loop.

**Output:** the menu text, and the exit code `0` to the operating system when the user chooses 3.

**Why it is needed:** every C program needs exactly one `main`; this one keeps the experiment interactive so
the user can try both techniques without restarting.

**Where it is called:** by the C runtime when the program starts (never called from other code).

**Line by line:**

```c
#include <stdio.h>
```
For `printf` and `scanf`.

```c
#include "ex1.h"
```
Brings in the prototypes `void bitStuffing(void);` and `void byteStuffing(void);` so the compiler knows these
functions exist (they are defined in other `.c` files) and can check the calls.

```c
int main(void) {
    int choice;
```
`choice` will hold the number the user types. `(void)` means `main` takes no command-line arguments.

```c
    while (1) {
```
`1` is always true → infinite loop. The only way out is the `return` inside `case 3`.

```c
        printf("\n1. Bit Stuffing\n2. Byte Stuffing\n3. Exit\n");
        printf("Enter your choice (1, 2, or 3): ");
```
Print the menu. The leading `\n` gives a blank line so the menu is separated from the previous technique's output.

```c
        scanf("%d", &choice);
```
Read an integer into `choice`. `&choice` passes the **address** so `scanf` can write into the variable.

```c
        switch (choice) {
            case 1: bitStuffing();  break;
            case 2: byteStuffing(); break;
```
Dispatch. `break` prevents "fall-through" into the next case. After the called function returns, execution
continues after the `switch`, which is the end of the loop body → the menu is shown again.

```c
            case 3: printf("\nExiting program\n"); return 0;
```
`return 0` ends `main`, which ends the program with success status 0. No `break` needed after `return`
(it would be unreachable).

```c
            default: printf("Invalid selection. Please enter 1, 2, or 3.\n");
        }
    }
}
```
Anything else → message, loop again. Because the loop never ends normally, there is no `return` after it; the
compiler accepts this because `main` returning nothing at end-of-function is permitted in C99+ (and the loop
is infinite anyway).

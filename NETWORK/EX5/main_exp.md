# main.c (ex5) — Menu driver for the Sliding Window protocols

## What the file is about

Entry point of experiment 5. It shows a menu (Stop-and-Wait, Go-Back-N, Selective Repeat, Exit), collects
the simulation parameters — number of frames, which frame to lose, and (for the two windowed protocols) the
window size — **validates them**, and calls the chosen simulation from `stop_and_wait.c`, `go_back_n.c` or
`selective_repeat.c`.

All input validation for the experiment lives here, so the three protocol files can assume sane values.

---

## Test cases (whole program)

Build `gcc *.c -o ex5` in `ex5/`.

| # | Type | Input | Result | How |
|---|------|-------|--------|-----|
| 1 | Positive | `2`, frames `8`, lose `3`, window `4` | Go-Back-N run (run.md sample), then menu again | passes all checks → `goBackN(8, 4, 3)`. |
| 2 | Positive | `1`, frames `4`, lose `2` | Stop-and-Wait run — **no window prompt** | `choice == 1` skips the window question; `w` is forced to 1. |
| 3 | Positive | `3`, `8`, `3`, `4` | Selective Repeat run | `selectiveRepeat(8, 4, 3)`. |
| 4 | Positive | `2`, `5`, `-1`, `2` | no-loss run | `-1 >= n` is false → accepted; the protocol never matches frame −1. |
| 5 | Positive | `4` | `Exiting...` | `break` leaves the loop. |
| 6 | Negative | `2`, frames `0`, lose `1`, window `1` | `Invalid input configurations!` | `n <= 0`. |
| 7 | Negative | `2`, frames `4`, lose `1`, window `6` | `Invalid input configurations!` | `w > n`. |
| 8 | Negative | `2`, frames `4`, lose `5`, window `1` | `Invalid lost frame number!` | `lost >= n`. |
| 9 | Negative | `9` | `Invalid choice! Try again.` | `choice < 1 || choice > 4`. |
| 10 | Negative | frames `51` | `Invalid input configurations!` | `n > MAX` (50). |
| 11 | Edge | window `0` | `Invalid input configurations!` | `w <= 0`. |
| 12 | Edge | lose `-100` | accepted, treated as no loss | only the upper bound is checked. |
| 13 | Edge | letter for a numeric prompt | `scanf` fails, value unchanged/garbage, letter stays buffered → repeated menu (Ctrl+C) | standard `scanf("%d")` problem. |

---

## Function: `main`

```c
int main(void)
```
**What:** menu loop + parameter collection + validation + dispatch. **Output:** exit status 0.

```c
#include <stdio.h>
#include "window.h"
```
I/O; `MAX` and the three protocol prototypes.

```c
int main(void) {
    int choice, n, w, lost;
```
`n` = frames, `w` = window size, `lost` = frame to lose.

```c
    do {
        printf("========== SLIDING WINDOW MENU ==========\n");
        printf("  1. Stop-and-Wait\n  2. Go-Back-N\n  3. Selective Repeat\n  4. Exit\n");
        printf("==========================================\n");
        printf("Enter choice: ");
        scanf("%d", &choice);
        printf("\n");
```
Menu and choice.

```c
        if (choice == 4) {
            printf("Exiting...\n");
            break;
        }
        if (choice < 1 || choice > 4) {
            printf("Invalid choice! Try again.\n\n");
            continue;
        }
```
Exit immediately on 4 (`break` leaves the `do…while`). Any other out-of-range value → message and
`continue` (jump to the loop condition, which is true, so the menu repeats). Handling these *before* asking
for parameters avoids pointless prompts.

```c
        printf("No. of frames  : ");
        scanf("%d", &n);
        printf("Frame to lose  : ");
        scanf("%d", &lost);
        w = 1;                                 /* Stop-and-Wait has no window size */
        if (choice != 1) {
            printf("Window size (N): ");
            scanf("%d", &w);
        }
```
Common parameters, then the window size only for options 2 and 3. `w = 1` is set first so the validation
below works for Stop-and-Wait too (window 1 is always valid when n ≥ 1).

```c
        if (n <= 0 || n > MAX || w <= 0 || w > n) {
            printf("\nInvalid input configurations!\n\n");
            continue;
        }
        if (lost >= n) {
            printf("\nInvalid lost frame number!\n\n");
            continue;
        }
```
Validation: 1 ≤ n ≤ MAX (50, bounds the `calloc` in Selective Repeat and keeps tables readable);
1 ≤ w ≤ n (a window larger than the data makes no sense); `lost` must be a real frame index or negative
(negative = no loss). Any failure → back to the menu.

```c
        switch (choice) {
            case 1: stopAndWait(n, lost);      break;
            case 2: goBackN(n, w, lost);       break;
            case 3: selectiveRepeat(n, w, lost); break;
        }
    } while (choice != 4);
    return 0;
}
```
Dispatch. No `default` is needed because invalid choices were filtered above. The `while (choice != 4)` is
technically redundant with the `break` but documents the exit condition.

---

## `#include` lines

- `<stdio.h>` — `printf`, `scanf`. `"window.h"` — `MAX`, prototypes.

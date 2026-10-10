# main.c (ex4) — Menu driver for the Routing Algorithm Simulator

## What the file is about

Entry point of experiment 4. It owns the network data (`n` and the `graph[][]` cost matrix), reads it from
the user (option 1), and dispatches the other menu options to `matrix.c`, `distance_vector.c` and
`link_state.c`. A guard makes sure options 2–6 cannot run before the matrix has been entered.

---

## Test cases (whole program)

Build `gcc *.c -o ex4` in `ex4/`.

| # | Type | Input | Result | How |
|---|------|-------|--------|-----|
| 1 | Positive | `1`, `4`, the 16 numbers of the run.md matrix | matrix stored, back to menu | `readMatrix` returns 1 → `initialized = 1`. |
| 2 | Positive | then `2` | cost matrix printed | `displayCostMatrix`. |
| 3 | Positive | `3` / `4 A` / `6 A D` / `5 A D 10` | DV tables / LS table / one path / link updated | cases 3–6 delegate. |
| 4 | Positive | `7` | `Program terminated.` | `case 7` prints; `while (menuChoice != 7)` ends the loop. |
| 5 | Negative | `3` **before** `1` | `Please enter the cost matrix first (Option 1)!` and menu again | `menuChoice >= 2 && <= 6 && !initialized` → `continue` skips the `switch`. |
| 6 | Negative | `1`, `0` (or `11`, `-2`) | `Invalid number of routers!`; `n` reset to 0; `initialized` stays as it was | `readMatrix` validates `1 ≤ n ≤ MAX` and returns 0. |
| 7 | Negative | `8` (or `0`) | `Invalid choice! Please try again.` | `default`. |
| 8 | Edge | `1`, `4`, with a non-zero diagonal (e.g. `5` at A→A) | stored as 0 | `if (i == j) graph[i][j] = 0;` overrides whatever was typed. |
| 9 | Edge | re-enter option 1 after a valid matrix with an invalid `n` | old matrix is kept but `n` becomes 0, `initialized` remains 1 → options 2–6 print empty tables | `readMatrix` sets `*n = 0` on failure but `main` only sets `initialized` on success and never clears it. Re-run option 1 with a valid n to recover. |
| 10 | Edge | letter instead of number for a menu choice | `scanf` fails, `menuChoice` keeps its old value; the letter stays in the buffer → endless menu loop (Ctrl+C) | standard `scanf("%d")` behaviour. |
| 11 | Edge | `1`, `10`, 100 numbers | maximum size accepted | `n > MAX` check passes for 10. |

---

## Function 1: `readMatrix`

```c
static int readMatrix(int *n, int graph[MAX][MAX])
```
**What:** asks for the number of routers and the full n × n cost matrix.
**Input:** `n` (output pointer), `graph` (output array); keyboard.
**Output:** 1 on success, 0 if `n` is invalid. **Why:** menu option 1. **Where called:** `main()` case 1.

```c
    printf("\nEnter number of routers (maximum %d): ", MAX);
    scanf("%d", n);
```
`n` is already a pointer, so no `&`.

```c
    if (*n <= 0 || *n > MAX) {
        printf("\nInvalid number of routers!\n");
        *n = 0;
        return 0;
    }
```
Validate `1 … MAX` (10). On failure zero it and return 0 so `main` does not mark the data as initialised.

```c
    printf("\nEnter the cost matrix row by row. Enter %d for no connection.\n\n", INF);
    for (int i = 0; i < *n; i++) {
        for (int j = 0; j < *n; j++) {
            scanf("%d", &graph[i][j]);
            if (i == j)
                graph[i][j] = 0;
        }
    }
    return 1;
```
Read n × n integers (whitespace-separated, so the user can type rows on separate lines or all on one).
Force the diagonal to 0 — a router is at distance 0 from itself regardless of what was typed (TC8).

---

## Function 2: `main`

```c
int main(void)
```
**What:** menu loop with an "initialised" guard. **Output:** exit status 0.

```c
    int n = 0, graph[MAX][MAX], menuChoice, initialized = 0;
```
`n` and `graph` are the shared network state, passed to every function. `initialized` records whether option 1 has succeeded.

```c
    do {
        printf("\n========================================\n");
        printf("      ROUTING ALGORITHM SIMULATOR\n");
        printf("========================================\n");
        printf("1. Enter/Change Full Network Cost Matrix\n2. Display Cost Matrix\n3. Distance Vector Routing\n"
               "4. Link State Routing\n5. Change Cost of Specific Router Pair\n"
               "6. Find Shortest Distance/Path for Specific Pair\n7. Exit\n");
        printf("\nEnter your choice: ");
        scanf("%d", &menuChoice);
```
Menu. Adjacent string literals (`"…\n" "…\n"`) are joined by the compiler into one string.

```c
        if (menuChoice >= 2 && menuChoice <= 6 && !initialized) {
            printf("\nPlease enter the cost matrix first (Option 1)!\n");
            continue;
        }
```
The guard: all options that *use* the matrix are blocked until it exists. `continue` in a `do…while` jumps
to the condition check (`menuChoice != 7`, true) and the menu repeats.

```c
        switch (menuChoice) {
            case 1: if (readMatrix(&n, graph)) initialized = 1; break;
            case 2: displayCostMatrix(n, graph);  break;
            case 3: runDistanceVector(n, graph);  break;
            case 4: runLinkState(n, graph);       break;
            case 5: updateLinkCost(n, graph);     break;
            case 6: findShortestRoute(n, graph);  break;
            case 7: printf("\nProgram terminated.\n"); break;
            default: printf("\nInvalid choice! Please try again.\n");
        }
    } while (menuChoice != 7);
    return 0;
```
Dispatch table. Only a successful `readMatrix` sets `initialized`. Arrays are passed by pointer in C, so
`graph` is shared, not copied — `updateLinkCost` modifies `main`'s matrix directly.

---

## `#include` lines

- `<stdio.h>` — I/O. `"router.h"` — `MAX`, `INF` and all prototypes.

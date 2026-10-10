# matrix.c (ex4) — Cost-matrix helpers (read a router letter, print, update a link)

## What the file is about

Experiment 4 represents the network as an **n × n cost matrix** `graph[i][j]` = cost of the direct link from
router *i* to router *j*; `INF` (999) means "no direct link"; the diagonal is 0. Routers are named by letters
`A`, `B`, `C`… (index 0, 1, 2…).

This file contains the utilities that every menu option needs:

- `readRouter` — ask for a router letter,
- `printMatrix` — print any n × n matrix with letter headings and `INF`,
- `displayCostMatrix` — titled wrapper around `printMatrix` (menu option 2),
- `updateLinkCost` — change the cost of one link in both directions (menu option 5).

---

## Test cases (whole file)

Using the `run.md` sample (4 routers):

```
0 2 999 1
2 0 3 999
999 3 0 4
1 999 4 0
```

### TC1 — Positive: option `2` (display)

```
            COST MATRIX
	A	B	C	D
A	0	2	INF	1
B	2	0	3	INF
C	INF	3	0	4
D	1	INF	4	0
```
How: `printMatrix` prints a header row of letters, then each row with its letter, replacing 999 by `INF`.

### TC2 — Positive: option `5`, `A`, `D`, new cost `10`

```
Previous cost between A and D: 1
Enter new cost (Enter 999 for INF/no connection): 
Cost matrix updated successfully between A and D!
```
Afterwards option 6 `A`→`D` gives cost 9 via `A -> B -> C -> D` instead of 1: the direct link is now more
expensive than the detour (2 + 3 + 4 = 9 < 10). How: `graph[0][3] = graph[3][0] = 10`.

### TC3 — Positive: option `5`, set a link to `999`

The link disappears; routing algorithms treat 999 as "no link" (`!= INF` checks).

### TC4 — Negative: option `5`, `A`, `A`

`Cost from a router to itself is always 0.` — refused. How: `r1 == r2` check.

### TC5 — Negative: option `5`, `Z`, `A` (letter outside `A–D`)

`Invalid router letters!` How: `'Z' - 'A'` = 25 ≥ n → range check fails.

### TC6 — Negative: lowercase `a`

`'a' - 'A'` = 32 → out of range → `Invalid router letters!`. Only capitals are accepted.

### TC7 — Edge: option `5` before option `1`

Never reaches this file: `main.c` blocks options 2–6 until the matrix is entered.

### TC8 — Edge: n = 10 (maximum)

Prompts show `(A-J)`; the matrix prints 10 columns (wide but correct).

### TC9 — Edge: new cost negative (e.g. `-5`)

Accepted without validation; Dijkstra/DV assume non-negative costs, so results become unreliable (a negative
link can make "shortest" paths loop in DV). Not guarded by the program.

---

## Function 1: `readRouter`

```c
char readRouter(const char *prompt, int n)
```
**What:** prints a prompt that mentions the last valid letter, reads one character.
**Input:** `prompt` — a format string containing one `%c` (the macros `SRC_PROMPT` / `DEST_PROMPT` from `router.h`); `n` — number of routers.
**Output:** the character typed (not validated here — callers check the range).
**Why:** four places ask for a router (`updateLinkCost` ×2, `runLinkState`, `findShortestRoute` ×2); one helper keeps the prompt consistent.
**Where called:** `updateLinkCost()` here; `runLinkState()` and `findShortestRoute()` in `link_state.c`.

```c
    char c;
    printf(prompt, 'A' + n - 1);
```
`'A' + n - 1` is the last router's letter (n = 4 → `'D'`). It is substituted into the `%c` of the prompt,
e.g. `Enter source router (A-D): `. Using a variable as the format string is fine here because the string is
a compile-time constant from the header, not user input.

```c
    scanf(" %c", &c);
    return c;
```
The leading space in `" %c"` skips any leftover whitespace/newline from the previous input — without it the
`%c` would read the `\n` left by the last `scanf("%d")`.

---

## Function 2: `printMatrix`

```c
void printMatrix(int n, int m[MAX][MAX])
```
**What:** prints an n × n matrix with `A B C…` column and row headings, tab-separated, `INF` for 999.
**Input:** `n`, matrix `m`. **Output:** console table.
**Why:** used both for the input cost matrix and for the final distance matrix of Distance Vector.
**Where called:** `displayCostMatrix()` here, `runDistanceVector()` in `distance_vector.c`.

```c
    printf("\n\t");
    for (int i = 0; i < n; i++)
        printf("%c\t", 'A' + i);
    printf("\n");
```
Header row: a tab (empty top-left corner) then each letter followed by a tab.

```c
    for (int i = 0; i < n; i++) {
        printf("%c\t", 'A' + i);
        for (int j = 0; j < n; j++) {
            if (m[i][j] == INF)
                printf("INF\t");
            else
                printf("%d\t", m[i][j]);
        }
        printf("\n");
    }
```
Each row starts with its letter, then n cells. `INF` is printed as the word so the user doesn't have to
remember that 999 is special.

Parameter note: `int m[MAX][MAX]` — a 2-D array parameter must state all dimensions except the first, so the
compiler knows each row is `MAX` ints wide when computing `m[i][j]`.

---

## Function 3: `displayCostMatrix`

```c
void displayCostMatrix(int n, int graph[MAX][MAX])
```
**What:** menu option 2 — prints a heading and then the matrix. **Where called:** `main.c` case 2.

```c
    printf("\n========================================\n");
    printf("            COST MATRIX\n");
    printf("========================================\n");
    printMatrix(n, graph);
```
Just decoration + delegation.

---

## Function 4: `updateLinkCost`

```c
void updateLinkCost(int n, int graph[MAX][MAX])
```
**What:** menu option 5 — asks for two routers and a new cost, updates the matrix symmetrically.
**Input:** `n`, `graph` (modified in place); keyboard: two letters and an int.
**Why:** to see how routing tables change when a link cost changes (e.g. re-run options 3/4/6 afterwards).
**Where called:** `main.c` case 5.

```c
    char c1, c2;
    int r1, r2, newCost;
    c1 = readRouter(SRC_PROMPT, n);
    c2 = readRouter(DEST_PROMPT, n);
    r1 = c1 - 'A';
    r2 = c2 - 'A';
```
Read two letters and convert to indices (`'C' - 'A'` = 2).

```c
    if (r1 < 0 || r1 >= n || r2 < 0 || r2 >= n) {
        printf("\nInvalid router letters!\n");
        return;
    }
    if (r1 == r2) {
        printf("\nCost from a router to itself is always 0.\n");
        return;
    }
```
Range check (TC5, TC6) and self-link check (TC4).

```c
    if (graph[r1][r2] == INF)
        printf("\nPrevious cost between %c and %c: INF\n", c1, c2);
    else
        printf("\nPrevious cost between %c and %c: %d\n", c1, c2, graph[r1][r2]);
```
Show the old value (with the `INF` word).

```c
    printf("Enter new cost (Enter %d for INF/no connection): ", INF);
    scanf("%d", &newCost);
    graph[r1][r2] = graph[r2][r1] = newCost;          /* links are two-way */
    printf("\nCost matrix updated successfully between %c and %c!\n", c1, c2);
```
Read the new cost and assign it to **both** `[r1][r2]` and `[r2][r1]` (chained assignment) because links
are undirected. Note: the initial matrix typed in option 1 is *not* forced to be symmetric — only updates are.

---

## `#include` lines

- `<stdio.h>` — `printf`, `scanf`. `"router.h"` — `MAX`, `INF`, `SRC_PROMPT`, `DEST_PROMPT`, prototypes.

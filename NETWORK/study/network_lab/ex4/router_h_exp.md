# router.h — Header for experiment 4 (Routing simulator)

## What the file is about

Shared constants, prompt strings and prototypes for `main.c`, `matrix.c`, `distance_vector.c` and
`link_state.c`. Everything that more than one file needs lives here.

---

## Test cases

| # | Type | Situation | Result | Why |
|---|------|-----------|--------|-----|
| 1 | Positive | `gcc *.c -o ex4` | builds | prototypes match the definitions. |
| 2 | Positive | user types `999` for a link | printed as `INF`, treated as "no link" by DV and Dijkstra | every comparison uses the `INF` macro, and `printMatrix` prints the word. |
| 3 | Positive | `readRouter(SRC_PROMPT, 4)` | prints `Enter source router (A-D): ` | the `%c` in the macro is filled with `'A' + 3`. |
| 4 | Negative | change `MAX` to 3 and enter 4 routers | `Invalid number of routers!` | `readMatrix` checks against `MAX`. |
| 5 | Negative | change `INF` to `99` and enter a link cost of 99 | that link is treated as missing | `INF` is a plain sentinel value; any real cost equal to it is indistinguishable from "no link". Keep real costs below 999. |
| 6 | Edge | a path whose true cost would exceed 999 (e.g. 500 + 600) | the sum 1100 is compared against `INF` = 999 and can be rejected/misprinted | costs are assumed to be small; 999 as infinity only works while real path costs stay well below it. |
| 7 | Positive | header included twice | fine | guard `ROUTER_H`. |

---

## Line by line

```c
#ifndef ROUTER_H
#define ROUTER_H
```
Include guard.

```c
#define MAX 10
```
Maximum number of routers; sizes every `[MAX][MAX]` matrix and `[MAX]` array. Letters `A`–`J`.

```c
#define INF 999
```
"Infinity" sentinel for "no direct link" / "unreachable". Chosen as a number the user can type easily.

```c
#define SRC_PROMPT  "\nEnter source router (A-%c): "
#define DEST_PROMPT "Enter destination router (A-%c): "
```
Prompt format strings with a `%c` placeholder for the highest router letter. Defined once so the three
functions that ask for routers print identical prompts (`readRouter` supplies the letter).

```c
/* matrix.c */
char readRouter(const char *prompt, int n);
void printMatrix(int n, int m[MAX][MAX]);
void displayCostMatrix(int n, int graph[MAX][MAX]);
void updateLinkCost(int n, int graph[MAX][MAX]);
```
Helpers in `matrix.c`. A 2-D array parameter must carry its second dimension (`[MAX]`) so the compiler can
compute `m[i][j]` addresses; the first dimension may be left empty or given — here both are `MAX`.

```c
/* distance_vector.c */
void runDistanceVector(int n, int cost[MAX][MAX]);
```
Menu option 3.

```c
/* link_state.c */
void runLinkState(int n, int graph[MAX][MAX]);
void findShortestRoute(int n, int graph[MAX][MAX]);
```
Menu options 4 and 6. (`dijkstra`, `getMinimumNode`, `printShortestPath` are `static` inside `link_state.c`
and therefore not declared here.)

```c
#endif
```

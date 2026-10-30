# link_state.c (ex4) — Link State Routing (Dijkstra's shortest path)

## What the file is about

In **Link State** routing every router knows the *whole* topology (the full cost matrix) and computes its
own shortest-path tree with **Dijkstra's algorithm**:

1. Start with `dist[source] = 0`, everything else INF, nothing visited.
2. Repeat: pick the **unvisited** router with the smallest `dist`, mark it visited, and **relax** all its
   neighbours (`dist[current] + link < dist[i]` → update `dist[i]` and remember `parent[i] = current`).
3. After n − 1 picks (or when nothing reachable remains) all distances are final.

`parent[]` lets the path be reconstructed backwards from any destination to the source.

This file provides two menu options: option 4 (full table from one source) and option 6 (one source →
one destination pair).

---

## Test cases (whole program)

Build `gcc *.c -o ex4`, option `1` with the run.md matrix, then:

### TC1 — Positive: option `4`, source `A`

```
Source Router: A
Destination	Cost	Shortest Path
A		0	A
B		2	A -> B
C		5	A -> D -> C
D		1	A -> D
```

Dijkstra trace from A (dist shown as [A,B,C,D]):

| step | pick (min unvisited) | relax | dist after | parent after |
|------|----------------------|-------|------------|--------------|
| init | – | – | [0, INF, INF, INF] | [-1,-1,-1,-1] |
| 1 | A (0) | B: 0+2=2; D: 0+1=1; C: no link | [0, 2, INF, 1] | [-1, A, -1, A] |
| 2 | D (1) | C: 1+4=5 < INF; B: no link | [0, 2, 5, 1] | [-1, A, D, A] |
| 3 | B (2) | C: 2+3=5, **not** < 5 → keep | [0, 2, 5, 1] | unchanged |

Loop ends after n − 1 = 3 picks. Path to C: parent[C] = D, parent[D] = A → `A -> D -> C`. Note DV chose
`via B` for the same cost 5 — both are correct; the tie is broken by processing order.

### TC2 — Positive: option `6`, `A` → `D`

```
     SHORTEST PATH: A -> D
Shortest Cost: 1
Path: A -> D
```

### TC3 — Positive after a cost change: option `5` A–D → 10, then option `6` `A` → `D`

```
Shortest Cost: 9
Path: A -> B -> C -> D
```
How: direct link now 10; A→B→C→D = 2 + 3 + 4 = 9 is cheaper. Dijkstra picks B (2), then C (5), then D (9 < 10).

### TC4 — Edge: disconnected network (A–B, C–D), option `4` source `A`

```
A		0	A
B		1	A -> B
C		INF	No Path
D		INF	No Path
```
How: after picking A and B, `getMinimumNode` finds no unvisited node with `dist < INF` → returns −1 → `break`.
C and D stay INF.

### TC5 — Edge: option `6`, `A` → `C` in the disconnected network

`No path exists between A and C.`

### TC6 — Negative: option `4`, source `Z`

`Invalid source router!` How: `'Z' - 'A'` = 25 ≥ n.

### TC7 — Negative: option `6`, `A` → `z` (lowercase)

`Invalid router letters!` (`'z' - 'A'` = 57).

### TC8 — Edge: option `6`, `A` → `A`

Cost 0, path `A`. How: `dist[source] = 0`, `parent[A] = -1` so `printShortestPath` prints just `A`.

### TC9 — Edge: 1 router

Table has one row `A  0  A`. The `for (count < n - 1)` loop runs 0 times.

### TC10 — Negative (theory): negative link cost

Dijkstra assumes non-negative costs; with a negative link it can finalise a node too early and report a
non-optimal path. Not guarded.

---

## Function 1: `getMinimumNode`

```c
static int getMinimumNode(const int dist[MAX], const int visited[MAX], int n)
```
**What:** finds the unvisited router with the smallest tentative distance.
**Input:** `dist`, `visited` arrays, `n`. **Output:** its index, or −1 if every unvisited router is INF (unreachable).
**Why:** it's the "greedy pick" step of Dijkstra. **Where called:** `dijkstra()`.

```c
    int min = INF, pos = -1;
    for (int i = 0; i < n; i++) {
        if (!visited[i] && dist[i] < min) {
            min = dist[i];
            pos = i;
        }
    }
    return pos;
```
Linear scan. Starting `min` at INF and using strict `<` means a router at distance INF is never chosen —
that's how `pos` stays −1 for disconnected remainders (TC4). Ties → lowest index wins.

---

## Function 2: `printShortestPath`

```c
static void printShortestPath(const int parent[MAX], int node)
```
**What:** prints the path from the source to `node` as `A -> D -> C`.
**Input:** `parent` array, destination index. **Output:** console (no newline).
**Why:** `parent[]` links point *backwards* (child → parent); to print forwards you go up first, then print
on the way back — a natural fit for **recursion**.
**Where called:** `runLinkState()` (each row), `findShortestRoute()`.

```c
    if (parent[node] != -1) {
        printShortestPath(parent, parent[node]);
        printf(" -> ");
    }
    printf("%c", 'A' + node);
```
If this node has a parent, first print the path to the parent (recursive call), then an arrow, then this
node's letter. The source has `parent == -1`, so the recursion stops there and it is printed first.
For C in TC1: `print(C)` → `print(D)` → `print(A)` prints `A`; back in `print(D)`: ` -> D`; back in
`print(C)`: ` -> C`.

---

## Function 3: `dijkstra`

```c
static void dijkstra(int n, int graph[MAX][MAX], int source, int dist[MAX], int parent[MAX])
```
**What:** computes shortest distances and parents from `source` to all routers.
**Input:** `n`, `graph`, `source`. **Output:** fills `dist[]` and `parent[]` (caller's arrays).
**Where called:** `runLinkState()` and `findShortestRoute()`.

```c
    int visited[MAX];
    for (int i = 0; i < n; i++) {
        dist[i] = INF;
        visited[i] = 0;
        parent[i] = -1;
    }
    dist[source] = 0;
```
Initialise: all unreachable, none visited, no parents; the source is at distance 0 from itself.

```c
    for (int count = 0; count < n - 1; count++) {
        int current = getMinimumNode(dist, visited, n);
        if (current == -1)
            break;
        visited[current] = 1;
```
At most n − 1 picks are needed (the last unvisited node's distance is already final). Pick the closest
unvisited node; if none is reachable, stop early. Mark it visited — its distance is now final and never changes.

```c
        for (int i = 0; i < n; i++) {
            if (!visited[i] && graph[current][i] != INF && dist[current] + graph[current][i] < dist[i]) {
                dist[i] = dist[current] + graph[current][i];
                parent[i] = current;
            }
        }
    }
```
**Relaxation:** for every unvisited neighbour `i` of `current` (there is a link and it's not INF), if going
through `current` is strictly shorter, update the distance and record `current` as the parent. Strict `<`
keeps the first-found path on ties (TC1: C keeps parent D because B's offer of 5 is not < 5).

---

## Function 4: `runLinkState`

```c
void runLinkState(int n, int graph[MAX][MAX])
```
**What:** menu option 4 — asks for a source, runs Dijkstra, prints cost and path to every router.
**Where called:** `main.c` case 4. Declared in `router.h`.

```c
    int dist[MAX], parent[MAX];
    int source = readRouter(SRC_PROMPT, n) - 'A';
    if (source < 0 || source >= n) {
        printf("\nInvalid source router!\n");
        return;
    }
    dijkstra(n, graph, source, dist, parent);
```
Read the letter (via `matrix.c`), convert to index, validate, compute.

```c
    printf("\n========================================\n");
    printf("          LINK STATE ROUTING\n");
    printf("========================================\n");
    printf("\nSource Router: %c\n", 'A' + source);
    printf("\nDestination\tCost\tShortest Path\n");
    printf("----------------------------------------\n");
    for (int i = 0; i < n; i++) {
        printf("%c\t\t", 'A' + i);
        if (dist[i] == INF)
            printf("INF\tNo Path\n");
        else {
            printf("%d\t", dist[i]);
            printShortestPath(parent, i);
            printf("\n");
        }
    }
```
One row per destination: INF → "No Path"; otherwise the cost and the reconstructed path.

---

## Function 5: `findShortestRoute`

```c
void findShortestRoute(int n, int graph[MAX][MAX])
```
**What:** menu option 6 — source and destination, prints only that one path.
**Where called:** `main.c` case 6.

```c
    int dist[MAX], parent[MAX];
    int source = readRouter(SRC_PROMPT, n) - 'A';
    int dest = readRouter(DEST_PROMPT, n) - 'A';
    if (source < 0 || source >= n || dest < 0 || dest >= n) {
        printf("\nInvalid router letters!\n");
        return;
    }
    dijkstra(n, graph, source, dist, parent);
```
Read both, validate both, run Dijkstra once from the source (it computes all destinations anyway; we just
report one).

```c
    printf("\n========================================\n");
    printf("     SHORTEST PATH: %c -> %c\n", 'A' + source, 'A' + dest);
    printf("========================================\n");
    if (dist[dest] == INF)
        printf("No path exists between %c and %c.\n", 'A' + source, 'A' + dest);
    else {
        printf("Shortest Cost: %d\n", dist[dest]);
        printf("Path: ");
        printShortestPath(parent, dest);
        printf("\n");
    }
```
Report cost and path, or "no path".

---

## `#include` lines

- `<stdio.h>` — `printf`. `"router.h"` — `MAX`, `INF`, prompts, `readRouter` prototype.

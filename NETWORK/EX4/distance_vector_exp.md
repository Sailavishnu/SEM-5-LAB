# distance_vector.c (ex4) — Distance Vector Routing (Bellman-Ford style relaxation)

## What the file is about

In **Distance Vector (DV)** routing every router keeps a table: for each destination, the *cost* and the
*next hop*. Initially it only knows its direct neighbours. Routers then repeatedly exchange tables and apply
the **Bellman-Ford** rule:

> cost(i → j) = min over every neighbour k of [ cost(i → k) + cost(k → j) ]

If going through *k* is cheaper than what *i* currently believes, *i* updates its cost and sets its next hop
to whatever it uses to reach *k*. This repeats until a whole round passes with **no change** (convergence).

This program simulates all routers at once: one `do…while` round applies the rule to every (i, j, k) triple,
counts the rounds, then prints each router's table and the final all-pairs cost matrix.

---

## Test cases (whole program)

Build `gcc *.c -o ex4`, option `1`, then option `3`.

### TC1 — Positive: run.md sample matrix

```
0 2 999 1
2 0 3 999
999 3 0 4
1 999 4 0
```
Output (abridged):
```
Iterations = 2

Routing Table - A
Destination	Next Hop	Cost
A		-		0
B		B		2
C		B		5
D		D		1

Routing Table - B
A		A		2
B		-		0
C		C		3
D		A		3

Routing Table - C
A		B		5
B		B		3
C		-		0
D		D		4

Routing Table - D
A		A		1
B		A		3
C		C		4
D		-		0

--- Final Shortest Path Cost Matrix ---
	A	B	C	D
A	0	2	5	1
B	2	0	3	3
C	5	3	0	4
D	1	3	4	0
```

How (round 1, router A, destination C): `dist[A][C]` starts INF. Try k = B: 2 + 3 = 5 < INF → update to 5,
`next[A][C] = next[A][B] = B`. Try k = D: 1 + 4 = 5, **not** `<` 5 → no change. So A→C via B (a tie, first
found wins; Link State picks D for the same cost — see `link_state_exp.md`). B→D: via A, 2 + 1 = 3 (better
than B→C→D = 7). Round 2 finds nothing to improve → `updated = 0` → loop ends → Iterations = 2.

### TC2 — Positive: a straight line A–B–C–D, all costs 1

```
0 1 999 999
1 0 1 999
999 1 0 1
999 999 1 0
```
```
Iterations = 2
	A	B	C	D
A	0	1	2	3
B	1	0	1	2
C	2	1	0	1
D	3	2	1	0
```
How: in one pass, because the loops process i = A first and k runs over all routers *after* earlier updates
in the same round, A learns C (via B, cost 2) and then D (via C, cost 3) within round 1. Round 2 confirms.
(Textbook DV would take 3 exchanges; this in-place simulation converges faster because updates are visible
immediately — a Gauss-Seidel style sweep.)

### TC3 — Edge: disconnected network (A–B and C–D only)

```
0 1 999 999
1 0 999 999
999 999 0 5
999 999 5 0
```
```
Routing Table - A
A		-		0
B		B		1
C		-		INF
D		-		INF
```
How: the `dist[i][k] != INF && dist[k][j] != INF` guard prevents `INF + something` from being compared, so
unreachable pairs stay INF and are printed as `-  INF`.

### TC4 — Edge: 1 router

`Iterations = 1`, one table with a single `A  -  0` row. Loops run but nothing to relax.

### TC5 — Edge: 5-router line

Still `Iterations = 2` for the same reason as TC2.

### TC6 — Negative (theory): a negative-cost link (option 5, cost −5)

Bellman-Ford with negative cycles never converges in theory; here the `<` rule would keep decreasing some
`dist` every round → the loop would run until values underflow. The program does not guard against it;
don't use negative costs.

### TC7 — Edge: asymmetric matrix typed in option 1 (e.g. A→B = 2 but B→A = 7)

DV uses the matrix as-is, so table A says B costs 2 and table B says A costs 7. Only option 5 forces symmetry.

---

## Function: `runDistanceVector`

```c
void runDistanceVector(int n, int cost[MAX][MAX])
```
**What:** runs DV to convergence and prints every router's table + the final cost matrix.
**Input:** `n` routers, `cost` matrix (not modified). **Output:** console.
**Why:** menu option 3. **Where called:** `main.c` case 3. Declared in `router.h`.

### Initialisation

```c
    int dist[MAX][MAX], next[MAX][MAX];
    int updated, rounds = 0;
```
`dist[i][j]` = best known cost from i to j. `next[i][j]` = first hop on that path (router index, or −1).
`updated` = did anything change in this round? `rounds` = iteration counter.

```c
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            dist[i][j] = cost[i][j];
            next[i][j] = (cost[i][j] != INF && i != j) ? j : -1;   /* first hop towards j */
        }
    }
```
Copy the direct costs. If there is a direct link (and it's not the router itself), the next hop is the
destination itself; otherwise −1 (unknown). This is what each router knows before exchanging anything.

### Relaxation loop

```c
    do {
        updated = 0;
        rounds++;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                for (int k = 0; k < n; k++) {
```
Repeat rounds until a round changes nothing. Inside: for every source i, destination j and intermediate k.

```c
                    if (dist[i][k] != INF && dist[k][j] != INF && dist[i][k] + dist[k][j] < dist[i][j]) {
```
Three conditions:
1. i can reach k. `INF` is just the number 999, so `INF + 3 = 1002` is an ordinary integer; the guard makes
   sure such "fake" sums are never treated as real costs (they would be rejected by `<` anyway because
   they exceed 999, but the guard states the intent and stays correct if `INF` or the cost range changes).
2. k can reach j (same reason).
3. Going via k is **strictly** cheaper than the current belief. Strict `<` means ties keep the first path
   found — that's why A→C goes via B (k = 1) and not via D (k = 3) in TC1.

```c
                        dist[i][j] = dist[i][k] + dist[k][j];
                        next[i][j] = next[i][k];
                        updated = 1;
```
Adopt the cheaper cost. The next hop towards j becomes **the next hop towards k** — because to go via k you
first go wherever you go to reach k. Flag that something changed.

```c
                    }
                }
            }
        }
    } while (updated);
```
When a full round passes with `updated == 0`, the tables are stable → convergence.

### Printing the tables

```c
    printf("\n========================================\n");
    printf("        DISTANCE VECTOR ROUTING\n");
    printf("========================================\n");
    printf("\nIterations = %d\n", rounds);
```
Header and how many rounds it took (the last round is always the "no change" confirmation round, so the
minimum is 1).

```c
    for (int i = 0; i < n; i++) {
        printf("\nRouting Table - %c\n", 'A' + i);
        printf("----------------------------------------\n");
        printf("Destination\tNext Hop\tCost\n");
        for (int j = 0; j < n; j++) {
            printf("%c\t\t", 'A' + j);
            if (i == j)
                printf("-\t\t0\n");
            else if (dist[i][j] == INF)
                printf("-\t\tINF\n");
            else
                printf("%c\t\t%d\n", 'A' + next[i][j], dist[i][j]);
        }
    }
```
One table per router. Three row types: itself (`-`, 0), unreachable (`-`, INF), reachable (next-hop letter =
`'A' + index`, cost).

```c
    printf("\n========================================\n");
    printf("      FINAL UPDATED ROUTING TABLE\n");
    printf("          (Distance Vector)\n");
    printf("========================================\n");
    printf("\n--- Final Shortest Path Cost Matrix ---\n");
    printMatrix(n, dist);
```
Finally the whole `dist` matrix through `printMatrix` (from `matrix.c`) — the all-pairs shortest costs.

---

## `#include` lines

- `<stdio.h>` — `printf`. `"router.h"` — `MAX`, `INF`, `printMatrix` prototype.

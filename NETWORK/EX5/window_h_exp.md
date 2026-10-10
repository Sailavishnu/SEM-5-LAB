# window.h — Header for experiment 5 (Sliding window protocols)

## What the file is about

Shared header for `main.c`, `stop_and_wait.c`, `go_back_n.c`, `selective_repeat.c`. Provides the frame
limit `MAX`, a `MIN(a, b)` macro used to clamp window edges, and the three protocol prototypes.

---

## Test cases

| # | Type | Situation | Result | Why |
|---|------|-----------|--------|-----|
| 1 | Positive | `MIN(6, 7)` | 6 | `(6) < (7) ? (6) : (7)`. |
| 2 | Positive | `MIN(Sb + N - 1, frames - 1)` with `Sb = 7, N = 4, frames = 8` | 7 | `10 < 7` false → `7`. This is how the last window prints `[7 - 7]`. |
| 3 | Edge | `MIN(3, 3)` | 3 | equal → second operand (same value). |
| 4 | Edge | `MIN(-1, 0)` | −1 | works with negatives. |
| 5 | Negative | `MIN(i++, j)` | `i` may be incremented **twice** | the macro pastes `i++` in two places: `((i++) < (j) ? (i++) : (j))`. Classic macro pitfall — the project never does this. |
| 6 | Negative | a version without the parentheses, `#define MIN(a,b) a < b ? a : b`, used as `2 * MIN(1, 2)` | would evaluate as `2 * 1 < 2 ? 1 : 2` = `0 ? 1 : 2` = 2 (wrong) | the outer parentheses in the real macro force the whole comparison to be evaluated first. |
| 7 | Positive | `gcc *.c -o ex5` | builds | prototypes match the definitions. |
| 8 | Negative | frames `51` in `main.c` | `Invalid input configurations!` | `n > MAX`. |

---

## Line by line

```c
#ifndef WINDOW_H
#define WINDOW_H
```
Include guard.

```c
#define MAX 50
```
Maximum number of frames per simulation (checked in `main.c`). Keeps output tables short and bounds the
`calloc(frames, …)` in Selective Repeat.

```c
#define MIN(a, b) ((a) < (b) ? (a) : (b))
```
Function-like macro: expands *textually* to a ternary expression returning the smaller of two values.
Every use of `a` and `b` is wrapped in parentheses so that arguments containing operators (`Sb + N - 1`)
are evaluated correctly, and the whole thing is parenthesised so it can be embedded in larger expressions
(test 6). Used in `go_back_n.c` and `selective_repeat.c` to clamp the window's upper edge to the last frame.

```c
void stopAndWait(int frames, int lostFrame);
```
Menu option 1. No window size parameter (window = 1 by definition).

```c
void goBackN(int frames, int N, int lostFrame);
```
Menu option 2. `N` = window size.

```c
void selectiveRepeat(int frames, int N, int lostFrame);
```
Menu option 3.

```c
#endif
```

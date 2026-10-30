# stop_and_wait.c (ex5) — Stop-and-Wait ARQ simulation

## What the file is about

**Stop-and-Wait** is the simplest reliable-delivery protocol: the sender transmits **one** frame, then
*stops* and *waits* for its acknowledgement (ACK). Only after the ACK arrives does it send the next frame.
If a frame is lost, no ACK comes; a **timer** expires and the sender retransmits the **same** frame.
The window size is effectively 1.

This program simulates `frames` frames numbered 0 … frames−1, where at most one frame (`lostFrame`) is lost
on its first transmission. It prints a step table and the total number of transmissions.

---

## Test cases (whole program)

Build `gcc *.c -o ex5`, choose `1`.

### TC1 — Positive with loss: frames `4`, lose `2`

```
| Step | Frame | Status  | Window     |
| 1    | 0     | ACK     | [0]        |
      -> Frame 0 successfully delivered.
      -> ACK 1 received. Moving to next frame.
| 2    | 1     | ACK     | [1]        |
      -> ACK 2 received. Moving to next frame.
| 3    | 2     | LOST    | [2]        |
      -> Frame 2 lost! Timer started... ⏰
      -> *** TIMEOUT ***. Resending same frame.
| 4    | 2     | ACK     | [2]        |
      -> ACK 3 received. Moving to next frame.
| 5    | 3     | ACK     | [3]        |
Frames delivered : 4
Total transmits  : 5
```
How: `Sn` goes 0, 1, 2 (lost → `Sn` not incremented, `lostFrame` cleared), 2 again (ACK), 3. Five sends for four frames.

### TC2 — Positive, no loss: frames `3`, lose `-1`

```
| 1 | 0 | ACK | [0] |
| 2 | 1 | ACK | [1] |
| 3 | 2 | ACK | [2] |
Frames delivered : 3
Total transmits  : 3
```
How: `Sn == -1` is never true, so every frame is ACKed first time.

### TC3 — Edge: lose the first frame (`0`)

Step 1: frame 0 LOST; step 2: frame 0 ACK; then 1, 2 … → total = frames + 1.

### TC4 — Edge: lose the last frame (`frames − 1`)

Last frame is sent twice; total = frames + 1.

### TC5 — Edge: 1 frame, lose `0`

Two steps (LOST, ACK); delivered 1, transmits 2.

### TC6 — Negative (caught in `main.c`): lose `4` with only 4 frames

`main.c` rejects `lost >= n` before calling this function (`Invalid lost frame number!`). If it were called
directly with such a value, the function would simply never lose anything (no frame number matches).

### TC7 — Edge: lose `-7` (any negative)

Same as no loss.

### TC8 — Edge: the ⏰ emoji on Windows CMD

May print as `?` unless the console is in UTF-8 (`chcp 65001`). Purely cosmetic (`run.md` §4).

---

## Function: `stopAndWait`

```c
void stopAndWait(int frames, int lostFrame)
```
**What:** simulates sending `frames` frames with at most one loss.
**Input:** `frames` — how many; `lostFrame` — index of the frame lost on first attempt (−1 or out of range = none).
**Output:** console table and totals.
**Why:** menu option 1 of the sliding-window experiment. **Where called:** `main.c` case 1. Declared in `window.h`.

```c
    int Sn = 0, step = 1, total = 0;
```
`Sn` = sequence number of the frame currently being sent (also the window base, since window = 1).
`step` = row number in the table. `total` = number of transmissions (including retransmissions).

```c
    printf("\n================ STOP-AND-WAIT ================\n");
    printf("+------+-------+---------+------------+\n");
    printf("| Step | Frame | Status  | Window     |\n");
    printf("+------+-------+---------+------------+\n");
```
Table header.

```c
    while (Sn < frames) {
```
Keep going until every frame (0 … frames−1) has been ACKed.

```c
        int lost = (Sn == lostFrame);
        total++;
```
Is this transmission the one that gets lost? Count the transmission either way.

```c
        printf("\n| %-4d | %-5d | %-7s | [%d]        |\n", step++, Sn, lost ? "LOST" : "ACK", Sn);
```
Print the row. `%-4d` = left-aligned in 4 columns (the `-` flag), so the table stays aligned. `step++` uses
the value then increments. The window column shows `[Sn]` — a single frame.

```c
        if (lost) {
            printf("      -> Frame %d lost! Timer started... ⏰\n", Sn);
            printf("      -> *** TIMEOUT ***. Resending same frame.\n");
            lostFrame = -1;                    /* the resent frame gets through */
        }
```
Loss: narrate the timer/timeout. `Sn` is **not** incremented, so the next loop iteration sends the same
frame again. Setting `lostFrame = -1` guarantees the retransmission succeeds (only one loss is simulated).

```c
        else {
            printf("      -> Frame %d successfully delivered.\n", Sn);
            printf("      -> ACK %d received. Moving to next frame.\n", Sn + 1);
            Sn++;
        }
    }
```
Success: ACK numbering follows the "next expected frame" convention (ACK n+1 acknowledges frame n).
Advance to the next frame.

```c
    printf("\n+------+-------+---------+------------+\n");
    printf("Frames delivered : %d\n", frames);
    printf("Total transmits  : %d\n", total);
    printf("=============================================\n\n");
```
Footer: all frames delivered; `total` = frames + (1 if a loss occurred).

---

## `#include` lines

- `<stdio.h>` — `printf`. `"window.h"` — the prototype (and `MAX`, `MIN`, unused here).


---

```





flow to memory




```


Exactly. **First flow, no code memorization yet.** Based on your exact Go-Back-N program. 

### 🧠 Go-Back-N flow

Think:

**WINDOW → SEND → LOSS? → ACK / GO BACK → NEW WINDOW**

Suppose:

```text
frames = 8
N = 4
lostFrame = 3
```

#### 1. Start

```text
Sb = 0
```

`Sb` = **starting/base frame of the current window**.

So window:

```text
[0 - 3]
```

---

#### 2. Send frames inside the window

```text
0 → ACK
1 → ACK
2 → ACK
3 → LOST
```

As each frame is ACKed:

```text
Sb++
```

So:

```text
Sb: 0 → 1 → 2 → 3
```

When frame `3` is lost:

**Sb does NOT increase.**

So `Sb = 3`.

---

#### 3. LOSS → GO BACK 🔥

Because frame `3` is lost:

```text
Timeout!
Go-Back-N triggered.
```

The next window starts **from `Sb`**:

```text
[3 - 6]
```

So frame `3` is sent again.

Then:

```text
3 → ACK
4 → ACK
5 → ACK
6 → ACK
```

Now:

```text
Sb = 7
```

---

#### 4. New window

Remaining frame:

```text
[7 - 7]
```

Send:

```text
7 → ACK
```

Then:

```text
Sb = 8
```

Since:

```text
Sb < frames
8 < 8  → false
```

**STOP.**

---

### 🔥 Remember Go-Back-N with just this

```text
Sb = beginning of window

        WINDOW
          ↓
      Send frames
          ↓
       LOSS?
       /    \
     NO      YES
     ↓        ↓
  Sb++     STOP window
              ↓
        go back to Sb
              ↓
        resend from Sb
              ↓
        create new window
```

### Most important difference from Stop-and-Wait

**Stop-and-Wait:**

```text
one frame → ACK → next
```

**Go-Back-N:**

```text
N frames → ACKs → if loss → GO BACK to lost frame
```

And the **main variable to remember is `Sb`**:

> **`Sb` = where the current window starts / where Go-Back-N goes back to.**

Once this flow is solid, the exact code becomes much easier.








# go_back_n.c (ex5) — Go-Back-N ARQ simulation

## What the file is about

**Go-Back-N** is a sliding-window protocol. The sender may have up to **N** frames "in flight" (sent but not
yet acknowledged) — the *window* `[Sb … Sb+N−1]`, where `Sb` is the window **base** (oldest unACKed frame).
Each ACK slides the base forward by one. If a frame is lost, the receiver discards it *and every later frame*
(it only accepts frames in order), so after the timeout the sender **goes back** to the lost frame and resends
everything from there.

This simulation sends frames one row at a time. When the lost frame is hit, the current window stops
(the frames after it in that window are not shown as sent), a timeout message is printed, and the outer
loop starts a new window from the lost frame.

---

## Test cases (whole program)

Build `gcc *.c -o ex5`, choose `2`.

### TC1 — Positive (run.md sample): frames `8`, lose `3`, window `4`

```
| 1    | 0     | ACK     | [0 - 3] |
| 2    | 1     | ACK     | [0 - 3] |
| 3    | 2     | ACK     | [0 - 3] |
| 4    | 3     | LOST    | [0 - 3] |
      -> Frame 3 lost! Receiver rejects subsequent pipeline frames.
      -> Timeout! Go-Back-N triggered. Resending window from frame 3...

| 5    | 3     | ACK     | [3 - 6] |
| 6    | 4     | ACK     | [3 - 6] |
| 7    | 5     | ACK     | [3 - 6] |
| 8    | 6     | ACK     | [3 - 6] |
      -> Window successfully moved to [7 - 7]

| 9    | 7     | ACK     | [7 - 7] |
Frames delivered : 8
Total transmits  : 9
```
How: window 1 = [0–3]; frames 0,1,2 ACK (`Sb` → 3); frame 3 lost → inner loop stops (`!lost` fails), `Sb`
stays 3. Outer loop: new window [3–6] from `Sb = 3`; all ACK → `Sb = 7`; window [7–7] (clamped by
`MIN(Sm, frames-1)`); frame 7 ACK → `Sb = 8` → done. 9 transmissions for 8 frames.

### TC2 — Positive, no loss: frames `5`, lose `-1`, window `2`

```
| 1 | 0 | ACK | [0 - 1] |
| 2 | 1 | ACK | [0 - 1] |
      -> Window successfully moved to [2 - 3]
| 3 | 2 | ACK | [2 - 3] |
| 4 | 3 | ACK | [2 - 3] |
      -> Window successfully moved to [4 - 4]
| 5 | 4 | ACK | [4 - 4] |
Total transmits  : 5
```
How: windows advance in blocks of N; the last window is shortened to the remaining frame.

### TC3 — Edge: lose the very first frame: frames `6`, lose `0`, window `3`

```
| 1 | 0 | LOST | [0 - 2] |
      -> Timeout! Go-Back-N triggered. Resending window from frame 0...
| 2 | 0 | ACK  | [0 - 2] |
| 3 | 1 | ACK  | [0 - 2] |
| 4 | 2 | ACK  | [0 - 2] |
      -> Window successfully moved to [3 - 5]
| 5 | 3 | ACK  | [3 - 5] |
| 6 | 4 | ACK  | [3 - 5] |
| 7 | 5 | ACK  | [3 - 5] |
Total transmits  : 7
```
How: window base never moved, so the retry window is the same `[0 - 2]`.

### TC4 — Edge: window = 1 (behaves like Stop-and-Wait)

frames 4, lose 2, N 1 → windows [0-0], [1-1], [2-2] LOST, [2-2] ACK, [3-3] → 5 transmits, same as Stop-and-Wait.

### TC5 — Edge: window = frames (one big window), lose the last frame

frames 4, lose 3, N 4 → 0,1,2 ACK, 3 LOST → resend window from 3 = [3-3] → 5 transmits.

### TC6 — Negative (rejected by `main.c`): window `0` or window `> frames`

`Invalid input configurations!` before this function is called.

### TC7 — Behavioural note (simplification)

Real Go-Back-N would *also* transmit frames 4, 5, 6 after frame 3 in TC1 (they are in the window and get
discarded by the receiver), so the real count would be 12, not 9. This simulation stops the window at the
lost frame to keep the table short; the message "Receiver rejects subsequent pipeline frames" describes what
would have happened to them.

---

## Function: `goBackN`

```c
void goBackN(int frames, int N, int lostFrame)
```
**What:** simulates Go-Back-N with window `N` and one lost frame.
**Input:** `frames`, `N` (window size), `lostFrame` (−1 = none).
**Output:** console table and totals. **Where called:** `main.c` case 2. Declared in `window.h`.

```c
    int Sb = 0, step = 1, total = 0;
```
`Sb` = send base (first unACKed frame). `step`, `total` as in Stop-and-Wait.

```c
    printf("\n================ GO-BACK-N ================\n");
    printf("+------+-------+---------+------------+\n");
    printf("| Step | Frame | Status  | Window     |\n");
    printf("+------+-------+---------+------------+\n");
```
Header.

```c
    while (Sb < frames) {
```
One iteration = one window's worth of sending. Ends when the base passes the last frame.

```c
        int Sm = Sb + N - 1, first = Sb, last = MIN(Sm, frames - 1), lost = 0;
        printf("\n");
```
`Sm` = theoretical top of the window. `first`/`last` are frozen copies used only for display (the `[a - b]`
column), with `last` clamped to the final frame via the `MIN` macro from `window.h`, so the last window
shows `[7 - 7]` and not `[7 - 10]`. `lost` = did this window hit the lost frame?

```c
        for (int Sn = Sb; Sn <= Sm && Sn < frames && !lost; Sn++) {
```
Send frames `Sb … Sm` in order, but stop early (a) at the end of the data and (b) as soon as a frame is lost.

```c
            total++;
            lost = (Sn == lostFrame);
            printf("| %-4d | %-5d | %-7s | [%d - %d] |\n", step++, Sn, lost ? "LOST" : "ACK", first, last);
```
Count the transmission, decide the status, print the row with the window range.

```c
            if (lost) {
                printf("      -> Frame %d lost! Receiver rejects subsequent pipeline frames.\n", Sn);
                lostFrame = -1;
            } else
                Sb++;                          /* ACKed: window base moves forward */
        }
```
Lost: explain, and clear `lostFrame` so the retransmission succeeds. Not lost: the ACK slides the base by one.
Because `Sb` is *not* incremented for the lost frame, `Sb == Sn` at that moment — the base is exactly the
frame to go back to.

```c
        if (lost)
            printf("      -> Timeout! Go-Back-N triggered. Resending window from frame %d...\n", Sb);
        else if (Sb < frames)
            printf("      -> Window successfully moved to [%d - %d]\n", Sb, MIN(Sb + N - 1, frames - 1));
    }
```
After the inner loop: either announce the timeout and the resend point (`Sb`), or — if there is still
something to send — announce the new window position. On the last window neither message is printed.

```c
    printf("\n+------+-------+---------+------------+\n");
    printf("Frames delivered : %d\n", frames);
    printf("Total transmits  : %d\n", total);
    printf("=============================================\n\n");
```
Footer.

---

## `#include` lines

- `<stdio.h>` — `printf`. `"window.h"` — `MIN` macro and the prototype.


---

```




Flow to memory




```

---


Yep, now I have the **exact Stop-and-Wait code**. 

To remember the variables **without changing even one letter**:

* `Sn` → **current frame being sent**
* `step` → **table row number**
* `total` → **total transmissions**
* `lost` → **is current `Sn` the lost frame?**
* `lostFrame` → **which frame should be lost**

The core flow is literally:

```c
int Sn = 0, step = 1, total = 0;
```

Remember:

**`Sn` = where I am**
**`step` = which attempt**
**`total` = how many sent**

Then:

```c
while (Sn < frames)
```

👉 **Is there still a frame to send?**

```c
int lost = (Sn == lostFrame);
total++;
```

👉 Check loss → count transmission.

Then the **MOST IMPORTANT PART**:

```c
if (lost) {
    lostFrame = -1;
}
else {
    Sn++;
}
```

🔥 Memorize this as:

> **LOST → `Sn` stays**
> **ACK → `Sn++`**

That's the entire variable logic of Stop-and-Wait.



```

Initialize Sn, step, total
        ↓
while (Sn < frames)
        ↓
check lost
        ↓
total++
        ↓
display intermediate result
        ↓
   lost?
   /    \
 YES     NO
 ↓       ↓
timeout  ACK
 ↓       ↓
same Sn  Sn++
        ↓
     repeat

```
# selective_repeat.c (ex5) — Selective Repeat ARQ simulation

## What the file is about

**Selective Repeat** is the smarter sliding-window protocol. Like Go-Back-N, the sender may have up to **N**
packets outstanding, but the receiver **buffers** out-of-order packets instead of discarding them. So when
one packet is lost, **only that packet** is retransmitted; the ones after it were already accepted.

The window still slides only when its **base** is acknowledged: if packet 3 (the base) is lost while 4, 5, 6
are fine, the window stays at 3 until 3 is resent and ACKed, then jumps straight past everything already
ACKed.

The program tracks which packets are ACKed in a dynamically allocated array `acked[]` and skips them on the
next pass through the window.

---

## Test cases (whole program)

Build `gcc *.c -o ex5`, choose `3`.

### TC1 — Positive: frames `8`, lose `3`, window `4`

```
| 1    | 0      | ACK     | [0 - 3] |
| 2    | 1      | ACK     | [0 - 3] |
| 3    | 2      | ACK     | [0 - 3] |
| 4    | 3      | LOST    | [0 - 3] |
      -> Packet 3 lost. Will be selectively retransmitted later.
      -> Window base ACKed. Moving window to [3 - 6]

| 5    | 3      | ACK     | [3 - 6] |
| 6    | 4      | ACK     | [3 - 6] |
| 7    | 5      | ACK     | [3 - 6] |
| 8    | 6      | ACK     | [3 - 6] |
      -> Window base ACKed. Moving window to [7 - 7]

| 9    | 7      | ACK     | [7 - 7] |
Frames delivered : 8
Total transmits  : 9
```
How: first pass sends 0–3; 0,1,2 are marked `acked`, 3 is not. The slide loop moves `Sb` from 0 to 3 (stops
at the first un-ACKed). Second pass [3–6]: 3 is resent (ACK), 4,5,6 sent. Slide to 7. Third pass: 7. Here the
lost packet was the last one in its window, so the trace looks like Go-Back-N; TC2 shows the difference.

### TC2 — Positive, loss at the **start** of a window: frames `6`, lose `0`, window `3`

```
| 1    | 0      | LOST    | [0 - 2] |
      -> Packet 0 lost. Will be selectively retransmitted later.
| 2    | 1      | ACK     | [0 - 2] |
      -> Packet 1 delivered and buffered by receiver.
| 3    | 2      | ACK     | [0 - 2] |
      -> Packet 2 delivered and buffered by receiver.

| 4    | 0      | ACK     | [0 - 2] |
      -> Packet 0 delivered and buffered by receiver.
      -> Window base ACKed. Moving window to [3 - 5]

| 5    | 3      | ACK     | [3 - 5] |
| 6    | 4      | ACK     | [3 - 5] |
| 7    | 5      | ACK     | [3 - 5] |
Total transmits  : 7
```
How: after packet 0 is lost the sender **keeps sending** 1 and 2 (unlike Go-Back-N, which stopped). The
window cannot slide (`acked[0] == 0`), so no "moving" message. Second pass over [0–2]: 0 is resent; 1 and 2
are skipped by `if (acked[Sn]) continue;` (not counted, not printed). Now `Sb` slides over 0,1,2 to 3.
Compare with Go-Back-N's TC3: same 7 transmissions here only because the loss was at the base; with a loss
in the middle Selective Repeat needs fewer.

### TC3 — Positive, no loss: frames `5`, lose `-1`, window `5`

Single window [0–4], five ACKs, 5 transmits, no "moving" message (after sliding, `Sb == frames`).

### TC4 — Edge: loss in the middle of a window: frames `5`, lose `1`, window `3`

Pass 1 [0–2]: 0 ACK, 1 LOST, 2 ACK. Slide: `Sb` → 1 (0 acked, 1 not) → "Moving window to [1 - 3]".
Pass 2 [1–3]: 1 resent ACK, 2 skipped, 3 ACK. Slide → 4. Pass 3: 4 ACK. Total 6. (The Go-Back-N program
also shows 6 for these inputs only because it stops sending at the loss; real Go-Back-N would transmit
packet 2 before the timeout and then resend 1, 2, 3 → 7.)

### TC5 — Edge: window = 1

Behaves like Stop-and-Wait: each pass is one packet.

### TC6 — Negative (rejected in `main.c`): window `> frames`, frames `0`, lose `≥ frames`

`Invalid input configurations!` / `Invalid lost frame number!` before this function runs.

### TC7 — Memory: `calloc` failure

If `calloc` returned NULL (out of memory — practically impossible for ≤ 50 ints) the code would dereference
NULL. There is no check; the `MAX = 50` limit in `main.c` keeps the allocation tiny.

---

## Function: `selectiveRepeat`

```c
void selectiveRepeat(int frames, int N, int lostFrame)
```
**What:** simulates Selective Repeat with window `N` and one lost packet.
**Input:** `frames`, `N`, `lostFrame`. **Output:** console. **Where called:** `main.c` case 3. Declared in `window.h`.

```c
    int Sb = 0, step = 1, total = 0;
    int *acked = calloc(frames, sizeof(int));
```
`Sb` = window base. `acked` = array of `frames` ints, all **zero-initialised** by `calloc` (unlike `malloc`),
one flag per packet: 1 = delivered and acknowledged. Heap-allocated because the size is only known at run time.

```c
    printf("\n============ SELECTIVE REPEAT ============\n");
    printf("+------+--------+---------+------------+\n");
    printf("| Step | Packet | Status  | Window     |\n");
    printf("+------+--------+---------+------------+\n");
```
Header (the column is called "Packet" — Selective Repeat literature often says packet rather than frame).

```c
    while (Sb < frames) {
        int Sm = Sb + N - 1, last = MIN(Sm, frames - 1), oldSb = Sb;
        printf("\n");
```
One pass over the current window per iteration. `Sm` = top of window, `last` = clamped top for display,
`oldSb` = remember where the base was so we can tell later whether it moved.

```c
        for (int Sn = Sb; Sn <= Sm && Sn < frames; Sn++) {
            int lost = (Sn == lostFrame);
            if (acked[Sn])
                continue;
```
Walk the window. Packets already ACKed in an earlier pass are **skipped** (no transmission, no row) — this is
the "selective" part. Note that unlike Go-Back-N the loop does **not** stop on a loss.

```c
            total++;
            printf("| %-4d | %-6d | %-7s | [%d - %d] |\n", step++, Sn, lost ? "LOST" : "ACK", Sb, last);
```
Count and print. The window shown is the *current* `[Sb - last]`.

```c
            if (lost) {
                printf("      -> Packet %d lost. Will be selectively retransmitted later.\n", Sn);
                lostFrame = -1;
            } else {
                acked[Sn] = 1;
                printf("      -> Packet %d delivered and buffered by receiver.\n", Sn);
            }
        }
```
Lost → leave `acked[Sn]` at 0 so the next pass resends it; clear `lostFrame` so that resend succeeds.
Delivered → mark it. "Buffered by receiver" reminds you that out-of-order packets are kept.

```c
        while (Sb < frames && acked[Sb])       /* slide over everything that is already ACKed */
            Sb++;
```
Slide the base forward past every consecutive ACKed packet. It stops at the first un-ACKed one (the lost
packet) or at the end. This single loop is what lets the window jump several positions at once after a
retransmission (TC2: 0 → 3 in one go).

```c
        if (Sb > oldSb && Sb < frames)
            printf("      -> Window base ACKed. Moving window to [%d - %d]\n", Sb, MIN(Sb + N - 1, frames - 1));
    }
```
Only announce a move if the base actually advanced and there is still something left to send.

```c
    printf("\n+------+--------+---------+------------+\n");
    printf("Frames delivered : %d\n", frames);
    printf("Total transmits  : %d\n", total);
    printf("===========================================\n\n");
    free(acked);
```
Footer, then release the heap array (every `calloc`/`malloc` needs a matching `free`).

---

## `#include` lines

- `<stdio.h>` — `printf`. `<stdlib.h>` — `calloc`, `free`. `"window.h"` — `MIN` and the prototype.



---

```






flow to memory





```

Yep 😈 **Selective Repeat next.**

Same method:

1. **Flow first**
2. Then **exact code + variables**
3. Then **intermediate output**

For Selective Repeat, the big new idea is:

> **Send the whole window, even if one frame is LOST. Already ACKed frames are skipped during retransmission.**

Your exact code uses `Sb`, `Sm`, `last`, `oldSb`, `Sn`, `lost`, `acked`, `step`, `total`. 

### Flow

```text
Sb = start of window
        ↓
Create window [Sb - last]
        ↓
Send every Sn in the window
        ↓
      LOST?
     /      \
   NO        YES
   ↓          ↓
acked[Sn]=1  don't ACK it
              ↓
        continue sending
        next frames
        ↓
After window:
slide Sb while acked[Sb] == 1
        ↓
If lost frame remains unACKed
        ↓
next pass sends ONLY that frame
(ACKed frames are skipped)
        ↓
Sb moves forward
        ↓
repeat until Sb == frames
```

### 🔥 Main difference

**Go-Back-N:**

> Loss → **stop sending** → go back.

**Selective Repeat:**

> Loss → **keep sending** → later **resend only the lost frame**.

And the key variable is:

```c
acked
```

Think:

> **`acked[Sn] = 1` → already done, SKIP it.**

That's the whole new concept.



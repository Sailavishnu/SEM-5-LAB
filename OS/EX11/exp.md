# Explanation of `disc.c`

The program has three parts:

1. **`display`** prints the path and total head movement. Every algorithm uses it.
2. **`sort`** and the six algorithm functions build the order in which the head visits the requests.
3. **`main`** reads the input, checks it, works out the direction, and runs the menu.

I'll use one example throughout:

- Requests: `98 183 37 122 14 124 65 67`
- Head: `53`, previous head: `50`, cylinders: `200`

---

## 1. `display`: the common function

```c
void display(int seq[], int n, int head)
{
    int i, total = 0;
    printf("Head movement: %d", head);
    for (i = 0; i < n; i++)
    {
        total += abs(head - seq[i]);
        head = seq[i];
        printf(" -> %d", head);
    }
    printf("\nTotal head movement = %d\n", total);
}
```

**What it receives:** `seq` is the order in which the head will visit the stops. `n` is how many stops there are. `head` is the starting position.

Each algorithm only has to produce `seq`. `display` then does the same job for all of them: it measures the distance between consecutive positions and adds it up.

**Line by line**

| Line | Meaning |
|---|---|
| `printf("Head movement: %d", head);` | Print the starting position. This is the first number in the path. |
| `for (i = 0; i < n; i++)` | Visit each stop in `seq`, from the first to the last. `i < n` means it stops after the last stop. |
| `total += abs(head - seq[i]);` | The distance from the current position to the next stop. `abs` gives the distance without a sign. |
| `head = seq[i];` | The head is now at that stop. This becomes the starting point for the next distance. |
| `printf(" -> %d", head);` | Print the stop, so the path is visible. |
| `printf("\nTotal ...", total);` | After the last stop, print the sum. |

**Worked example (FCFS, head 53):**

| Step | From | To | Distance | Running total |
|---|---|---|---|---|
| 1 | 53 | 98 | 45 | 45 |
| 2 | 98 | 183 | 85 | 130 |
| 3 | 183 | 37 | 146 | 276 |
| … | … | … | … | … |

The final total is 640.

**Why `head` can be changed:** `head` is a copy made when the function is called. Changing it inside `display` does not affect the caller. That's why the algorithms can pass their starting position in directly.

---

## 2. `sort`: bubble sort

```c
void sort(int a[], int n)
{
    for (i = 0; i < n - 1; i++)
        for (j = 0; j < n - i - 1; j++)
            if (a[j] > a[j + 1]) { swap }
}
```

**Purpose:** put the requests in ascending order. SCAN, CSCAN, LOOK, and CLOOK need this.

**Outer loop `i < n - 1`:** each pass places one more value at the end, so `n - 1` passes are enough.

**Inner loop `j < n - i - 1`:** after `i` passes, the last `i` values are already in place, so the inner loop can stop earlier. The `- 1` stops it from reading past the end.

**The `if`:** if the left value is bigger than the right value, swap them. After all passes, the array is in ascending order.

---

## 3. FCFS

```c
void FCFS(int req[], int n, int head)
{
    printf("\nFCFS\n");
    display(req, n, head);
}
```

FCFS serves requests in the order they arrived. No sorting or looping is needed. It passes the original array straight to `display`.

---

## 4. SSTF: shortest seek time first

```c
int done[MAX] = {0};
int seq[MAX], i, j, pos, min, count = 0;
int start = head;

for (i = 0; i < n; i++)
{
    min = 100000;
    pos = -1;
    for (j = 0; j < n; j++)
    {
        if (!done[j] && abs(head - req[j]) < min)
        {
            min = abs(head - req[j]);
            pos = j;
        }
    }
    done[pos] = 1;
    seq[count++] = req[pos];
    head = req[pos];
}
display(seq, n, start);
```

**Variables**

- `done[j]`: 1 means request `j` has already been served.
- `start`: the original head. It's saved because `head` changes during the loop, and `display` needs the starting position.
- `min`: the smallest distance found so far. It starts at 100000, which is larger than any real distance, so the first unserved request always becomes the first candidate.

**Outer loop `i < n`:** serve all `n` requests, one per pass.

**Inner loop `j < n`:** look at every request to find the closest one that is still waiting.

**The `if` condition, part by part:**

- `!done[j]`: request `j` is not served yet.
- `abs(head - req[j]) < min`: request `j` is closer than the best one found so far.

The comparison is `<`, not `<=`. So if two requests are equally close, the earlier one in the input keeps the lead.

**After the inner loop:** `pos` is the closest request. Mark it as done, add it to `seq`, and move the head to it.

---

## 5. SCAN: the elevator

```c
void SCAN(int req[], int n, int head, int size, int dir)
{
    int a[MAX], seq[MAX + 1];
    int i, count = 0;
    for (i = 0; i < n; i++) a[i] = req[i];
    sort(a, n);

    if (dir == 1)
    {
        for (i = 0; i < n; i++)
            if (a[i] >= head) seq[count++] = a[i];
        seq[count++] = size - 1;
        for (i = n - 1; i >= 0; i--)
            if (a[i] < head) seq[count++] = a[i];
    }
    else
    {
        for (i = n - 1; i >= 0; i--)
            if (a[i] <= head) seq[count++] = a[i];
        seq[count++] = 0;
        for (i = 0; i < n; i++)
            if (a[i] > head) seq[count++] = a[i];
    }
    display(seq, count, head);
}
```

**Why copy into `a[]`:** `sort` changes the array. Copying keeps the original request order safe for FCFS and SSTF.

**Right-moving (`dir == 1`), three parts:**

1. `for (i = 0; i < n; i++)` with `if (a[i] >= head)`: serve requests at or above the head, in ascending order. The loop goes from the smallest to the largest, which is the order the head meets them going up.
2. `seq[count++] = size - 1;`: go all the way to the top of the disk. SCAN always does this, even with no request there.
3. `for (i = n - 1; i >= 0; i--)` with `if (a[i] < head)`: on the way back down, serve requests below the head, in descending order.

**Left-moving (`else`), three parts:** the mirror image.

1. Requests at or below the head, in descending order.
2. Go to cylinder 0.
3. Requests above the head, in ascending order.

**Why the conditions split the requests this way:** `>=` and `<` (or `<=` and `>`) divide the requests into two groups that cover all of them exactly once. A request equal to the head is placed in the first group, so it's served immediately.

**Worked example (dir = Right, head 53):** the sorted list is `14 37 65 67 98 122 124 183`.

- Group 1, `>= 53`: `65 67 98 122 124 183`
- Edge: `199`
- Group 2, `< 53`, descending: `37 14`

The path is `53 → 65 → 67 → 98 → 122 → 124 → 183 → 199 → 37 → 14`, and the total is 331.

---

## 6. CSCAN: circular SCAN

The structure is the same as SCAN, with one change: after reaching the edge, the head jumps back to the other end and serves in the same direction.

**Right-moving:**

1. Requests `>= head`, ascending.
2. Go to `size - 1`.
3. Jump to `0` (added to `seq`, so the jump distance is counted).
4. Requests `< head`, **ascending** this time, because the head is moving up from 0.

**Left-moving:**

1. Requests `<= head`, descending.
2. Go to `0`.
3. Jump to `size - 1`.
4. Requests `> head`, **descending**, because the head is moving down from the top.

The second group's loop direction is what makes CSCAN circular. Both groups are served in the same direction.

---

## 7. LOOK and CLOOK

**LOOK** is SCAN without the edge visits. The loop stops at the last request in each direction.

- Right: `>= head` ascending, then `< head` descending.
- Left: `<= head` descending, then `> head` ascending.

**CLOOK** is CSCAN without the edge visits. The jump goes from the last request to the first request.

- Right: `>= head` ascending, then `< head` ascending.
- Left: `<= head` descending, then `> head` descending.

**Summary of the conditions:**

| Algorithm | Right: first group | Right: second group | Left: first group | Left: second group |
|---|---|---|---|---|
| SCAN | `>= head`, asc, then edge `size-1` | `< head`, desc | `<= head`, desc, then edge `0` | `> head`, asc |
| CSCAN | `>= head`, asc, then `size-1`, `0` | `< head`, asc | `<= head`, desc, then `0`, `size-1` | `> head`, desc |
| LOOK | `>= head`, asc | `< head`, desc | `<= head`, desc | `> head`, asc |
| CLOOK | `>= head`, asc | `< head`, asc | `<= head`, desc | `> head`, desc |

---

## 8. `main`

### Reading input

```c
printf("Enter number of requests: ");
scanf("%d", &n);
if (n < 1 || n > MAX) { printf(...); return 1; }

for (i = 0; i < n; i++)
    scanf("%d", &req[i]);

scanf("%d", &head);       /* initial head */
scanf("%d", &prev_head);  /* previous head */
scanf("%d", &size);       /* total cylinders */
```

- `scanf("%d", &n)` reads the number of requests. `&n` gives `scanf` the address to store the value in.
- The `if` rejects a count below 1 or above `MAX` (100), and the program exits with `return 1`.
- The loop reads the request queue, one number at a time.
- The last three reads give the head, the previous head, and the disk size.

### Validation

```c
if (size < 1 || head < 0 || head >= size || prev_head < 0 || prev_head >= size)
    error
for (i = 0; i < n; i++)
    if (req[i] < 0 || req[i] >= size) error
```

Every position must be between `0` and `size - 1`. The `||` means the program stops if any one check fails.

### Calculating the direction

```c
if (head >= prev_head) dir = 1;   /* Right */
else                   dir = 0;   /* Left */
```

The head moved from `prev_head` to `head`, so the direction is the sign of that movement.

- Example: head `53`, previous `50`. `53 >= 50`, so `dir = 1` (Right).
- If head is `60` and previous is `53`, `dir` is `1` (Right).
- If head is `40` and previous is `60`, `dir` is `0` (Left).
- If the two are equal, the code chooses Right. The head hasn't moved, so this is a default choice.

`dir` is stored as `1` or `0`, and the SCAN, CSCAN, LOOK, and CLOOK functions use it to choose which branch to run.

### Menu loop

```c
do
{
    print menu
    scanf("%d", &choice);
    switch (choice) { ... }
} while (choice != 7);
```

- `do ... while` runs the menu at least once, then repeats while `choice` is not 7.
- The `switch` calls the function for the chosen algorithm. Each call receives the request array, the count, the head, and, for the disk-edge algorithms, `size` and `dir`.
- `default` handles any number outside 1 to 7.

---

## What to remember for the viva

1. `display` only needs the visiting order. Every algorithm produces that order, and `display` adds up the distances.
2. The if conditions split the requests into two groups, `>=`/`<` or `<=`/`>`. Together the groups contain every request once.
3. The loop direction is the key difference between the algorithms: SCAN reverses, CSCAN keeps the same direction, and the "C" versions jump back.
4. Direction comes from `head >= prev_head`.
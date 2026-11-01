# Banker's Algorithm in C: Experiment Notes

This file explains the whole program: its flow, every function, every variable, and all test cases with the real terminal output.

---

## 1. Big Picture

The Banker's algorithm answers one question before resources are given out:

> If I give this process what it asks for, can all processes still finish in some order?

If yes, the state is **safe** and the grant stays. If no, the state is **unsafe** and the grant is undone.

The program does this in four steps:

1. Read the data (Allocation, Max, Available).
2. Compute Need = Max − Allocation.
3. Find a safe sequence for the starting state.
4. Loop over requests. For each request: check it, pretend to grant it, check safety, and keep or undo it.

---

## 2. Global Constant

```c
#define MAX 10
```

- `MAX` is the largest number of processes and resource types the arrays can hold.
- Every array is sized `MAX`, so the program works for up to 10 × 10.
- It is a preprocessor constant. It is replaced by `10` before compilation and takes no memory.
- It is the only global name in the program. No variables are global.

---

## 3. Function Overview

| Function | Job | Called from |
|---|---|---|
| `calculateNeed` | Computes Need = Max − Allocation | `main` (once) |
| `checkSafety` | Finds a safe sequence, returns 1 or 0 | `main`, `requestResources` |
| `printSequence` | Prints a sequence like `P0 P2 P1` | `main`, `requestResources` |
| `requestResources` | Validates one request, tentatively grants, checks safety, keeps or undoes | `main` (in loop) |
| `main` | Reads input, runs initial check, runs the request loop | Program start |

---

## 4. Function by Function

### 4.1 `calculateNeed`

```c
void calculateNeed(int p, int r, int maxm[][MAX], int alloc[][MAX], int need[][MAX])
{
    for (int i = 0; i < p; i++)
        for (int j = 0; j < r; j++)
            need[i][j] = maxm[i][j] - alloc[i][j];
}
```

**Purpose:** fill the Need matrix. Need is the most a process may still ask for.

**Parameters**

| Name | Type | Meaning |
|---|---|---|
| `p` | `int` | number of processes (rows) |
| `r` | `int` | number of resource types (columns) |
| `maxm` | `int[][MAX]` | Max matrix, read only |
| `alloc` | `int[][MAX]` | Allocation matrix, read only |
| `need` | `int[][MAX]` | Need matrix, written here |

The array parameters are written `[][MAX]` because C needs the column size to index a 2D array. Arrays are passed by reference, so writing into `need` changes the caller's array.

**Line by line**

| Line | What it does |
|---|---|
| `for (int i = 0; i < p; i++)` | Loop over each process `i` (row). `int i` is declared here, so it exists only inside this loop. |
| `for (int j = 0; j < r; j++)` | For each process, loop over each resource `j` (column). |
| `need[i][j] = maxm[i][j] - alloc[i][j];` | The formula. Subtract what the process holds from what it may ever need. Stored in `need`. |

Both loops have no braces because each body is a single statement.

---

### 4.2 `checkSafety`

```c
int checkSafety(int p, int r, int alloc[][MAX], int need[][MAX], int avail[], int seq[], int *count)
{
    int work[MAX], finish[MAX] = {0}, found = 1;

    *count = 0;
    for (int j = 0; j < r; j++)
        work[j] = avail[j];

    while (found)
    {
        found = 0;
        for (int i = 0; i < p; i++)
        {
            int ok = !finish[i];
            for (int j = 0; j < r; j++)
                if (need[i][j] > work[j])
                    ok = 0;

            if (ok)
            {
                for (int j = 0; j < r; j++)
                    work[j] += alloc[i][j];
                seq[(*count)++] = i;
                finish[i] = found = 1;
            }
        }
    }

    return *count == p;
}
```

**Purpose:** decide whether the state is safe, and record the safe sequence if it is.

**Returns:** `1` if every process can finish (safe), `0` otherwise (unsafe).

**Parameters**

| Name | Type | Meaning |
|---|---|---|
| `p` | `int` | number of processes |
| `r` | `int` | number of resource types |
| `alloc` | `int[][MAX]` | current Allocation, read only |
| `need` | `int[][MAX]` | current Need, read only |
| `avail` | `int[]` | current Available, read only |
| `seq` | `int[]` | output: process numbers in the order they finish |
| `count` | `int *` | output: how many processes finished (pointer, so the caller sees the change) |

**Local variables**

| Name | Meaning | Why it exists |
|---|---|---|
| `work[MAX]` | a copy of Available that grows as processes finish | the Banker's "Work" vector. The real `avail` must not change during the test. |
| `finish[MAX]` | `finish[i] = 1` when process `i` has finished | stops a process from being counted twice. `= {0}` sets all entries to 0 at start. |
| `found` | `1` if some process finished during the last full scan | tells the `while` loop whether to keep scanning |

**Line by line**

| Line | What it does |
|---|---|
| `int work[MAX], finish[MAX] = {0}, found = 1;` | Declares the three locals. `found = 1` makes the `while` loop run at least once. |
| `*count = 0;` | Resets the finished-counter. `*count` writes to the caller's variable through the pointer. |
| `for (int j = 0; j < r; j++)` | Start of the copy loop. |
| `work[j] = avail[j];` | Copy each Available value into Work. Work starts as Available. |
| `while (found)` | Repeat full scans while the last scan made progress. |
| `found = 0;` | Assume no process fits this scan. If one does, it is set to 1. |
| `for (int i = 0; i < p; i++)` | Scan every process in order `0, 1, ..., p-1`. |
| `int ok = !finish[i];` | Start with `ok = 1` only if process `i` has not finished yet. `!` flips 0 to 1 and 1 to 0. |
| `for (int j = 0; j < r; j++)` | Compare each resource type. |
| `if (need[i][j] > work[j])` | If process `i` needs more of resource `j` than Work has, it cannot run now. |
| `ok = 0;` | Mark it as not runnable. Keep checking the other resources anyway (harmless). |
| `if (ok)` | Process `i` can run: all its needs fit in Work. |
| `work[j] += alloc[i][j];` | When `i` finishes, it gives back everything it holds. |
| `seq[(*count)++] = i;` | Store `i` at the next free spot in `seq`, then increase `*count` by 1. The `(*count)++` form first uses the old value as the index, then increments it. |
| `finish[i] = found = 1;` | Mark `i` finished, and set `found = 1` so another scan happens. This is a chained assignment: `found = 1` first, then `finish[i] = 1`. |
| `return *count == p;` | Return 1 if all `p` processes finished, otherwise 0. |

**Why it loops:** one scan can unlock another process. For example, after P2 finishes it may give P1 enough resources, so P1 needs a second scan. The `while` loop repeats until a full scan finds nothing new.

---

### 4.3 `printSequence`

```c
void printSequence(int count, int seq[])
{
    for (int i = 0; i < count; i++)
        printf("P%d ", seq[i]);
    printf("\n");
}
```

**Purpose:** print a sequence such as `P0 P2 P1`.

| Parameter | Meaning |
|---|---|
| `count` | how many entries to print |
| `seq` | the process numbers to print |

| Line | What it does |
|---|---|
| `for (int i = 0; i < count; i++)` | Visit each stored entry. |
| `printf("P%d ", seq[i]);` | Print `P` followed by the number, then a space. `%d` prints the number. |
| `printf("\n");` | End the line. |

---

### 4.4 `requestResources`

```c
void requestResources(int p, int r, int pid, int req[], int alloc[][MAX], int need[][MAX], int avail[])
{
    int seq[MAX], count;

    if (pid < 0 || pid >= p)
    {
        printf("Invalid process number. Use 0 to %d.\n", p - 1);
        return;
    }

    for (int j = 0; j < r; j++)
        if (req[j] > need[pid][j])
        {
            printf("Denied: request exceeds P%d's Need.\n", pid);
            return;
        }

    for (int j = 0; j < r; j++)
        if (req[j] > avail[j])
        {
            printf("Denied: not enough Available. P%d must wait.\n", pid);
            return;
        }

    for (int j = 0; j < r; j++)
    {
        avail[j] -= req[j];
        alloc[pid][j] += req[j];
        need[pid][j] -= req[j];
    }

    if (checkSafety(p, r, alloc, need, avail, seq, &count))
    {
        printf("Request GRANTED. New safe sequence: ");
        printSequence(count, seq);
    }
    else
    {
        for (int j = 0; j < r; j++)
        {
            avail[j] += req[j];
            alloc[pid][j] -= req[j];
            need[pid][j] += req[j];
        }
        printf("Request NOT granted: resulting state is unsafe. P%d must wait.\n", pid);
    }
}
```

**Purpose:** process one request using the four Banker's checks, in order.

**Parameters**

| Name | Meaning |
|---|---|
| `p`, `r` | sizes of the system |
| `pid` | the process making the request |
| `req` | the requested amount for each resource type |
| `alloc`, `need`, `avail` | the live state. Changed here when the grant is kept, restored when undone. |

**Local variables**

| Name | Meaning | Why it exists |
|---|---|---|
| `seq[MAX]` | the safe sequence returned by `checkSafety` | `checkSafety` needs somewhere to write it |
| `count` | how many processes finished | `checkSafety` writes it through `&count` |

These are local so they are not shared with `main`. Their values are only needed inside this function.

**The four checks, in order**

| Check | Lines | Meaning | Fail message |
|---|---|---|---|
| 0. Valid pid | `if (pid < 0 \|\| pid >= p)` | Process must exist | "Invalid process number" |
| 1. Request ≤ Need | first `for` loop | Process may not ask for more than it declared | "Denied: request exceeds Need" |
| 2. Request ≤ Available | second `for` loop | Resources must exist right now | "Denied: not enough Available" |
| 3. Tentative grant + safety | third `for`, then `checkSafety` | Would the state still be safe? | "NOT granted ... unsafe" |

**Line by line**

| Line | What it does |
|---|---|
| `int seq[MAX], count;` | Declare the locals for the safety call. |
| `if (pid < 0 \|\| pid >= p)` | Reject a process number outside `0 .. p-1`. `\|\|` means "or". Without this check, the program would read or write outside the array. |
| `printf("Invalid process number...")` | Explain the error. |
| `return;` | Leave the function. Nothing has been changed yet. |
| `for (int j = 0; j < r; j++)` | Check every resource type. |
| `if (req[j] > need[pid][j])` | Is the request bigger than what this process still needs? |
| `printf(...); return;` | Deny the request. The first `return` means nothing is changed. |
| `if (req[j] > avail[j])` | Is the request bigger than what is free right now? |
| `printf(...); return;` | Deny it. The process must wait. Nothing is changed. |
| `avail[j] -= req[j];` | Pretend to grant: take the resources out of Available. |
| `alloc[pid][j] += req[j];` | Pretend to grant: add them to the process's Allocation. |
| `need[pid][j] -= req[j];` | Pretend to grant: reduce the process's remaining Need. |
| `if (checkSafety(...))` | Run the safety check on the pretend state. `&count` is passed so `count` is filled in. |
| `printf("GRANTED..."); printSequence(...)` | The grant stays. Print the new safe sequence. |
| `else { ... }` | The pretend state is unsafe. Undo the three changes. |
| `avail[j] += req[j];` | Put the resources back in Available. |
| `alloc[pid][j] -= req[j];` | Remove them from Allocation. |
| `need[pid][j] += req[j];` | Restore the Need. |
| `printf("NOT granted...")` | Report that the process must wait. |

**Why the rollback is needed:** `checkSafety` reads the real `avail`, `alloc`, and `need`. The grant has to be applied before the check runs. If the result is unsafe, the change must be undone so later requests start from a true state.

**Why the first two checks use `return` instead of `else`:** each failed check exits at once, so the pretend grant never happens for a bad request.

---

### 4.5 `main`

```c
int main(void)
{
    int p, r, pid, count;
    int alloc[MAX][MAX], maxm[MAX][MAX], need[MAX][MAX];
    int avail[MAX], req[MAX], seq[MAX];
    char choice;
    ...
}
```

**Purpose:** read the input, run the initial safety check, then loop over requests.

**Variables**

| Name | Type | Meaning | Where used |
|---|---|---|---|
| `p` | `int` | number of processes | read, then passed everywhere |
| `r` | `int` | number of resource types | read, then passed everywhere |
| `pid` | `int` | process number typed by the user | request loop |
| `count` | `int` | number of processes in the safe sequence | initial check, printing |
| `alloc` | `int[MAX][MAX]` | Allocation matrix | read, passed to all functions |
| `maxm` | `int[MAX][MAX]` | Max matrix | read, used only by `calculateNeed` |
| `need` | `int[MAX][MAX]` | Need matrix | computed, passed to safety and request |
| `avail` | `int[MAX]` | Available vector | read, changed by requests |
| `req` | `int[MAX]` | the request vector for one request | read each loop, passed to `requestResources` |
| `seq` | `int[MAX]` | initial safe sequence | filled by initial check, printed |
| `choice` | `char` | answer to "another request?" | loop condition |

**Line by line (main)**

| Line | What it does |
|---|---|
| `printf("Enter number of processes and resource types: ");` | Prompt. |
| `scanf("%d %d", &p, &r);` | Read `p` and `r`. `&` gives the address so `scanf` can store the value. |
| `for (...) for (...) scanf("%d", &alloc[i][j]);` | Read the Allocation matrix row by row. |
| `for (...) for (...) scanf("%d", &maxm[i][j]);` | Read the Max matrix row by row. |
| `for (int j = 0; j < r; j++) scanf("%d", &avail[j]);` | Read the Available vector. |
| `calculateNeed(p, r, maxm, alloc, need);` | Fill the Need matrix. |
| `if (!checkSafety(p, r, alloc, need, avail, seq, &count))` | Run the initial safety check. `!` means "if it is NOT safe". |
| `printf("...UNSAFE..."); return 0;` | The starting state is unsafe. No request is processed, so the program ends. |
| `printf("Initial state is SAFE...")` | Print the header. |
| `printSequence(count, seq);` | Print the initial safe sequence. |
| `do { ... } while (choice == 'y' \|\| choice == 'Y');` | Request loop. The body runs once first, then repeats while the user answers `y` or `Y`. |
| `scanf("%d", &pid);` | Read the process number. |
| `scanf("%d", &req[j]);` (loop) | Read the request vector. |
| `requestResources(p, r, pid, req, alloc, need, avail);` | Process the request. |
| `scanf(" %c", &choice);` | Read the answer. The space before `%c` skips the leftover newline from the last input. Without it, `choice` would get `\n`. |

**Why `do ... while` and not `while`:** the request must be asked at least once. A `while` loop would check the condition first and might never ask.

---

## 5. Why the Important Variables Exist

| Variable | Why it is needed |
|---|---|
| `alloc` | what each process currently holds. Used in the Work update and in the grant. |
| `maxm` | the declared maximum. Used only to compute Need. Not needed after that. |
| `need` | what each process may still ask for. The main test for a request. |
| `avail` | what is free now. The main test for a request, and the input to the safety check. |
| `work` (inside `checkSafety`) | a scratch copy so the real `avail` is not changed during the safety test. |
| `finish` | prevents a process from being run twice. |
| `seq` | records the order, so it can be printed. |
| `count` | tells us if all processes finished (`count == p`). |
| `req` | the request being tested. |

---

## 6. Terminal Test Cases

### Base data (3 processes × 3 resources)

| Process | Max | Alloc | Need |
|---|---|---|---|
| P0 | 3 2 2 | 0 1 0 | 3 1 2 |
| P1 | 6 1 3 | 2 0 0 | 4 1 3 |
| P2 | 3 1 4 | 3 0 2 | 0 1 2 |

Available = `3 3 2`

Every run below starts with this input (except Test 2):

```
3 3
0 1 0
2 0 0
3 0 2
3 2 2
6 1 3
3 1 4
3 3 2
```

Then the request lines are typed. The program always asks for a process number first, so to quit after a check you enter a zero request and then `n`.

---

### Test 1: Safe start, zero request, quit

**Input after base:** `0 0 0 0`, then `n`

```
Enter number of processes and resource types: 3 3
Enter Allocation Matrix:
Enter Max Matrix:
Enter Available Resources:

Initial state is SAFE.
Safe sequence: P0 P2 P1

Enter process number (0 to 2): 0
Enter request for each of 3 resources: 0 0 0
Request GRANTED. New safe sequence: P0 P2 P1

Another request? (y/n): n
```

**Explanation:** The initial check finds P0 → P2 → P1. A zero request passes every check, and the state is still safe.

---

### Test 2: Unsafe start

**Input:** Alloc, Max as above, but Available = `0 0 0`

```
Enter Available Resources:
Initial state is UNSAFE (deadlock possible). No requests can be processed.
```

**Explanation:** With nothing free, no process's Need fits, so the initial check fails. The program exits before the request loop.

---

### Test 3: Request exceeds Need

**Input after base:** `1`, then `5 0 0`, then `n`

```
Initial state is SAFE.
Safe sequence: P0 P2 P1

Enter process number (0 to 2): 1
Enter request for each of 3 resources: Denied: request exceeds P1's Need.

Another request? (y/n): n
```

**Explanation:** P1's Need in resource 0 is 4. The request of 5 is larger, so it fails check 1. Nothing is changed.

---

### Test 4: Within Need, but more than Available

**Input after base:** `1`, then `4 0 0`, then `n`

```
Enter process number (0 to 2): 1
Enter request for each of 3 resources: Denied: not enough Available. P1 must wait.
```

**Explanation:** 4 is within P1's Need (4), so check 1 passes. Available in resource 0 is 3, so check 2 fails. P1 must wait.

---

### Test 5: Granted (safe)

**Input after base:** `2`, then `0 1 2`, then `n`

```
Enter process number (0 to 2): 2
Enter request for each of 3 resources: Request GRANTED. New safe sequence: P2 P0 P1
```

**Explanation:** Checks 1 and 2 pass. The pretend grant gives:
- Available: 3 3 2 → 3 2 0
- P2 Allocation: 3 0 2 → 3 1 4
- P2 Need: 0 1 2 → 0 0 0

The safety check on this state finds P2 → P0 → P1, so the grant is kept. The sequence changed because the state changed.

---

### Test 6: Tentatively granted, then rolled back

**Input after base:** `1`, then `2 0 2`, then `n`

```
Enter process number (0 to 2): 1
Enter request for each of 3 resources: Request NOT granted: resulting state is unsafe. P1 must wait.
```

**Explanation:** Checks 1 and 2 pass (2 ≤ Need 4, 2 ≤ Available 3). The pretend grant gives:
- Available: 3 3 2 → 1 3 0
- P1 Allocation: 2 0 0 → 4 0 2
- P1 Need: 4 1 3 → 2 1 1

The safety check finds no process whose Need fits in Work (1 3 0). The state is unsafe, so all three changes are undone and the state returns to the base.

This is the most important test. It shows that a request can pass both simple checks and still be refused.

---

### Test 7: Invalid process number

**Input after base:** `7`, then `1 0 0`, then `n`

```
Enter process number (0 to 2): 7
Enter request for each of 3 resources: Invalid process number. Use 0 to 2.
```

**Explanation:** Process 7 does not exist. Check 0 catches it before any array is accessed.

---

## 7. How to Study This

1. Draw the base table and compute Need by hand.
2. Trace the initial safety check, writing Work after each step.
3. Trace Test 5 and Test 6 by hand, writing Available, Allocation, Need, and Work at each step.
4. Explain why Test 6 is rolled back even though both simple checks pass.
5. Be ready to explain the purpose of `work`, `finish`, `found`, `count`, and the rollback.

---

## 8. Known Limitation

The request loop always asks for a process number before it asks whether to continue. To quit, you must enter a request first (for example `0 0 0 0`, then `n`). A cleaner design would ask "another request?" before the first request, or check for a quit value. This is fine for the lab, but mention it if an examiner asks about input handling.
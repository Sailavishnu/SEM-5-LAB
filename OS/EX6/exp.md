# Banker's Algorithm — Full Test Run Explained

This document walks through the entire program run, step by step, in the order it happened:

1. Setup and the **initial safe sequence**
2. **Negative test case** — request rejected instantly (invalid)
3. **Unsafe test case** — request tentatively granted, then rolled back (must wait)
4. **Positive test case** — request granted permanently (safe)

---

## 1. The Setup: What We Entered

**Resources:** 3 processes (P0, P1, P2), 3 resource types (let's call them R0, R1, R2 — think of them as A, B, C).

**Allocation Matrix** (what each process currently holds):

| Process | R0 | R1 | R2 |
|---------|----|----|----|
| P0      | 2  | 2  | 2  |
| P1      | 2  | 0  | 3  |
| P2      | 1  | 2  | 4  |

**Max Matrix** (the maximum each process will ever need):

| Process | R0 | R1 | R2 |
|---------|----|----|----|
| P0      | 3  | 6  | 8  |
| P1      | 4  | 3  | 3  |
| P2      | 3  | 4  | 4  |

**Available** (resources sitting free in the system, not given to anyone): `2 3 1`

### Step 1a: Computing the Need Matrix

The formula is simple:

```
Need[i][j] = Max[i][j] - Allocation[i][j]
```

This tells us how much *more* of each resource a process could still ask for before hitting its own declared maximum.

| Process | Max         | Allocation  | Need = Max − Alloc |
|---------|-------------|-------------|---------------------|
| P0      | 3, 6, 8     | 2, 2, 2     | **1, 4, 6**         |
| P1      | 4, 3, 3     | 2, 0, 3     | **2, 3, 0**         |
| P2      | 3, 4, 4     | 1, 2, 4     | **2, 2, 0**         |

This matches the program's output exactly:
```
P0: 1 4 6
P1: 2 3 0
P2: 2 2 0
```

---

## 2. The Initial Safe Sequence Check

**The core question:** Can *every* process eventually finish and return its resources, one after another, without anyone getting stuck forever?

**The method (Safety Algorithm):**
1. Start with `Work = Available`.
2. Look for any process whose `Need ≤ Work` (every resource column checked). If found, pretend it runs to completion.
3. When it "finishes," add its full Allocation back into `Work` (because a finished process releases everything it was holding).
4. Repeat until either all processes have finished (→ **SAFE**), or no process can proceed (→ **UNSAFE**).

**Trace it:**

| Step | Work (before) | Process checked | Need ≤ Work? | Action | Work (after) |
|------|----------------|------------------|--------------|--------|----------------|
| 1 | 2, 3, 1 | P0 needs 1,4,6 | 4 > 3 → **No** | Skip P0 | 2, 3, 1 |
| 1 | 2, 3, 1 | P1 needs 2,3,0 | 2≤2, 3≤3, 0≤1 → **Yes** | P1 runs, add its Alloc (2,0,3) | **4, 3, 4** |
| 2 | 4, 3, 4 | P2 needs 2,2,0 | 2≤4, 2≤3, 0≤4 → **Yes** | P2 runs, add its Alloc (1,2,4) | **5, 5, 8** |
| 3 | 5, 5, 8 | P0 needs 1,4,6 | 1≤5, 4≤5, 6≤8 → **Yes** | P0 runs, add its Alloc (2,2,2) | **7, 7, 10** |

All three processes finished → **SAFE**, in the order **P1 → P2 → P0**.

This is exactly what the program printed:
```
Initial Work = 2 3 1
P1 runs -> Work = 4 3 4
P2 runs -> Work = 5 5 8
P0 runs -> Work = 7 7 10

System is in SAFE state.
Safe sequence: P1 -> P2 -> P0
```

**Why does this matter?** A safe sequence is a *proof* that the system can never deadlock as long as it keeps satisfying requests in a way that stays safe. It doesn't mean this exact order will happen — it means it's *possible*, which is enough to guarantee no permanent deadlock right now.

---

## 3. Test Case 1 (Negative): Request Rejected Instantly

**Request:** P2 asks for `(5, 0, 2)`

**The very first check the program does — before touching Available at all:**

```
Is Request ≤ Need[P2]?
```

Why this check exists: a process is only ever *allowed* to ask for resources up to what it declared in its Max matrix. If a request exceeds its own declared Need, it's not a resource shortage problem — it's an invalid/buggy request that violates the process's own contract.

| Resource | Requested | Need[P2] | Requested ≤ Need? |
|----------|-----------|----------|---------------------|
| R0       | 5         | 2        | **No — 5 > 2, FAILS** |
| R1       | 0         | 2        | Yes |
| R2       | 2         | 0        | Yes |

The check fails right at **R0**: P2 said (via its Max matrix) it would never need more than 3 total of R0, it already holds 1, so at most it can ever request 2 more. Asking for 5 breaks that promise.

**Result:**
```
Request denied: exceeds P2's declared Need.
  Resource R0 -> Requested = 5, Need = 2 (Requested > Need, invalid)
```

**Key takeaway:** This request never even got to touch `Available` or run a safety check — it was rejected at the door for being self-inconsistent. This is different from the next case, where the request *is* valid but still can't be safely granted.

---

## 4. Test Case 2 (Unsafe → Wait): Request Tentatively Granted, Then Rolled Back

**Request:** P0 asks for `(1, 2, 1)`

### Step 4a: The two validity checks

| Resource | Requested | Need[P0] | Requested ≤ Need? |
|----------|-----------|----------|---------------------|
| R0 | 1 | 1 | Yes |
| R1 | 2 | 4 | Yes |
| R2 | 1 | 6 | Yes |

Passes the Need check.

| Resource | Requested | Available | Requested ≤ Available? |
|----------|-----------|-----------|--------------------------|
| R0 | 1 | 2 | Yes |
| R1 | 2 | 3 | Yes |
| R2 | 1 | 1 | Yes |

Passes the Available check too. So far, this looks like a perfectly reasonable request — it doesn't break any rule. This is why the program doesn't reject it immediately.

### Step 4b: Tentatively grant it

The program pretends to hand over the resources and updates all three tables:

| | Before | Change | After |
|---|--------|--------|-------|
| Available | 2, 3, 1 | − (1,2,1) | **1, 1, 0** |
| Alloc[P0] | 2, 2, 2 | + (1,2,1) | 3, 4, 3 |
| Need[P0]  | 1, 4, 6 | − (1,2,1) | 0, 2, 5 |

```
Tentatively granted. Available is now: 1 1 0
```

### Step 4c: Re-run the full safety check on this *new* state

**Work starts at `(1, 1, 0)`.** Check every process:

| Process | Need | Compare to Work (1,1,0) | Can it run? |
|---------|------|---------------------------|-------------|
| P0 | 0, 2, 5 | needs 2 of R1, only 1 available | **No** |
| P1 | 2, 3, 0 | needs 2 of R0, only 1 available | **No** |
| P2 | 2, 2, 0 | needs 2 of R0, only 1 available | **No** |

**Every single process is stuck.** None of them can get enough resources to finish, which means none of them can finish and release resources back, which means `Work` can never grow, which means **nothing will ever unblock**. This is a genuine risk of permanent deadlock — hence **UNSAFE**.

### Step 4d: Roll back

Since granting this request leads to an unsafe state, the program **undoes** the tentative grant completely — as if the request never happened:

| | Tentative | Change | Rolled back to |
|---|-----------|--------|------------------|
| Available | 1, 1, 0 | + (1,2,1) | **2, 3, 1** (original) |
| Alloc[P0] | 3, 4, 3 | − (1,2,1) | 2, 2, 2 (original) |
| Need[P0]  | 0, 2, 5 | + (1,2,1) | 1, 4, 6 (original) |

```
Request CANNOT be granted: resulting state is unsafe. P0 must wait.
Rolled back. Available is now: 2 3 1
```

**Key takeaway:** This is the heart of the Banker's Algorithm — it doesn't just check "is this a valid request," it actually *simulates* granting it and checks whether the *resulting* system state is still safe. If not, it backs out completely and makes P0 wait, even though nothing was technically wrong with the request on its own.

---

## 5. Test Case 3 (Positive): Request Granted Permanently

**Request:** P1 asks for `(1, 0, 0)`

### Step 5a: Validity checks

| Resource | Requested | Need[P1] | OK? |
|----------|-----------|----------|-----|
| R0 | 1 | 2 | Yes |
| R1 | 0 | 3 | Yes |
| R2 | 0 | 0 | Yes |

| Resource | Requested | Available | OK? |
|----------|-----------|-----------|-----|
| R0 | 1 | 2 | Yes |
| R1 | 0 | 3 | Yes |
| R2 | 0 | 1 | Yes |

Both pass.

### Step 5b: Tentatively grant it

| | Before | Change | After |
|---|--------|--------|-------|
| Available | 2, 3, 1 | − (1,0,0) | **1, 3, 1** |
| Alloc[P1] | 2, 0, 3 | + (1,0,0) | 3, 0, 3 |
| Need[P1]  | 2, 3, 0 | − (1,0,0) | 1, 3, 0 |

```
Tentatively granted. Available is now: 1 3 1
```

### Step 5c: Safety check on the new state

**Work starts at `(1, 3, 1)`.**

| Step | Work (before) | Process checked | Need ≤ Work? | Action | Work (after) |
|------|----------------|------------------|--------------|--------|----------------|
| 1 | 1, 3, 1 | P0 needs 1,4,6 | 4 > 3 → No | Skip | 1, 3, 1 |
| 1 | 1, 3, 1 | P1 needs 1,3,0 | 1≤1, 3≤3, 0≤1 → **Yes** | P1 runs, add Alloc (3,0,3) | **4, 3, 4** |
| 2 | 4, 3, 4 | P2 needs 2,2,0 | Yes | P2 runs, add Alloc (1,2,4) | **5, 5, 8** |
| 3 | 5, 5, 8 | P0 needs 1,4,6 | Yes | P0 runs, add Alloc (2,2,2) | **7, 7, 10** |

All processes finish → **SAFE**.

```
Request CAN be granted immediately. New state is safe.
Safe sequence: P1 -> P2 -> P0
```

**Why did this succeed where P0's request failed?** Look closely: P0's request ate into resource **R1**, dragging it down to 1 — and it turned out *every* process needed more R1 than that to proceed, so everyone got stuck. P1's request left R1 completely untouched at **3**, which happened to be exactly enough for P1 *itself* to immediately qualify to finish. Once P1 finishes, it hands back all of its resources `(3, 0, 3)`, which is enough to unblock P2, and then P0. One request either "unlocks" a chain of completions, or it doesn't — and that's the entire difference between safe and unsafe.

---

## Summary Table: All Three Request Outcomes

| Test Case | Requesting Process | Request | Failed at | Outcome |
|-----------|--------------------|---------|-----------|---------|
| Negative  | P2 | 5, 0, 2 | Need check (R0: 5 > 2) | **Denied instantly** — invalid, never checked Available or safety |
| Unsafe    | P0 | 1, 2, 1 | Safety check (all processes stuck) | **Tentatively granted → rolled back** — must wait |
| Positive  | P1 | 1, 0, 0 | (passed everything) | **Granted permanently** — new state proven safe |

These three cases together demonstrate the complete decision logic of the Banker's Algorithm:
1. **Is the request even legal?** (≤ Need)
2. **Do we physically have enough right now?** (≤ Available)
3. **If we give it, does the system stay safe?** (run the full safety algorithm on the resulting state)

Only when all three pass does a request get granted for good.
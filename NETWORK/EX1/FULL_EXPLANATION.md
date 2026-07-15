# Bit Stuffing & Byte Stuffing — Full Code Explanation

---

## What This Program Does

This program simulates **Data Link Layer Framing** — the process of wrapping raw data with markers so a receiver knows where a frame starts and ends, and can detect errors.

Two protocols are implemented:
- **Bit Stuffing** — works at the bit level
- **Byte Stuffing** — works at the byte level

---

## Global Declarations

```c
int data[MAX_BITS], stuffed[MAX_BITS], destuffed[MAX_BITS];
int dataBytes[MAX_BYTES], stuffedBytes[MAX_BYTES], framedBytes[MAX_BYTES], destuffedBytes[MAX_BYTES];
```

These are declared globally so both functions can access them freely.

| Array | Purpose |
|-------|---------|
| `data[]` | Original bits from input text |
| `stuffed[]` | Bits after inserting stuffed zeros |
| `destuffed[]` | Bits after removing stuffed zeros at receiver |
| `dataBytes[]` | Original bytes from input text |
| `stuffedBytes[]` | Bytes after adding escape prefixes |
| `framedBytes[]` | stuffedBytes wrapped with flag bytes |
| `destuffedBytes[]` | Final recovered bytes at receiver |

```c
int flag[] = {0,1,1,1,1,1,1,0};  // = 01111110, used as frame boundary in bit stuffing
int flagLen = 8;
int flagByte, escByte;             // used in byte stuffing
```

`flag[]` is the **frame delimiter** in bit stuffing. It is the bit pattern `01111110`. The stuffing rule is designed specifically to make sure this pattern never appears inside data bits.

---

## Utility Functions

### `printBits(label, arr, n)`

```c
void printBits(char *label, int arr[], int n) {
    printf("%s", label);
    for (i = 0; i < n; i++) {
        printf("%d", arr[i]);
        if ((i + 1) % 8 == 0 && i != n - 1)
            printf(" ");   // space after every 8 bits
    }
    printf("\n");
}
```

Prints a bit array with a space after every 8 bits.

Example output for "hi":
```
Original Bin : 01101000 01101001
               h = 104   i = 105
```

---

### `printByteArray(byte)` and `printBytesAsBinary(label, arr, n)`

```c
void printByteArray(int byte) {
    for (i = 7; i >= 0; i--)
        printf("%d", (byte >> i) & 1);  // extract each bit from MSB to LSB
}
```

`(byte >> i) & 1` shifts the byte right by `i` positions and checks the last bit.

For byte = 65 (`A`):
```
i=7: 65>>7 = 0  → bit 0
i=6: 65>>6 = 1  → bit 1
...
Output: 01000001
```

`printBytesAsBinary` calls `printByteArray` for each byte with a space in between.

---

## BIT STUFFING — `bitStuffing()` Full Explanation

### The Problem It Solves

The flag `01111110` marks frame start/end. If the same pattern appears in the data, the receiver gets confused and thinks the frame ended early. So we **break up any sequence of five consecutive 1s** by inserting a 0 after them.

---

### Step 1 — Take Input and Convert to Bits

```c
scanf("%s", inputString);
str_to_ascii(inputString, ascii_arr, &str_len);
```

`str_to_ascii` converts `"hi"` → `ascii_arr = [104, 105]`, `str_len = 2`.

```c
for (i = 0; i < str_len; i++) {
    ascii_to_bin((char)ascii_arr[i], bin_str);
    for (b = 0; b < 8; b++) {
        data[n++] = (bin_str[b] == '1') ? 1 : 0;
    }
}
```

For each ASCII value:
- `ascii_to_bin` converts the integer to an 8-character binary string like `"01101000"`
- The inner loop extracts each character `'0'` or `'1'` and stores `0` or `1` in `data[]`

Result for `"hi"`:
```
data[] = [0,1,1,0,1,0,0,0, 0,1,1,0,1,0,0,1]
          h = 01101000       i = 01101001
n = 16
```

---

### Step 2 — Stuff the Bits

```c
ones = 0;
j = 0;
for (i = 0; i < n; i++) {
    stuffed[j++] = data[i];                        // always copy the current bit
    ones = (data[i] == 1) ? ones + 1 : 0;          // increment count if 1, reset if 0
    if (ones == 5) {
        stuffed[j++] = 0;                           // insert extra 0 after five 1s
        ones = 0;                                   // reset counter
    }
}
stuffedLen = j;
```

**Walkthrough logic:**
- Every bit is copied into `stuffed[]` first, no matter what
- After copying, if that bit was a `1`, increment `ones`; if it was a `0`, reset `ones` to 0
- The moment `ones` hits 5, immediately insert a `0` and reset the counter
- This guarantees no six consecutive 1s ever appear in stuffed data (which would look like the flag `01111110`)

**Example with a critical input** — suppose `data[]` has `...1,1,1,1,1,1...`:

| i | data[i] | copied | ones | Action |
|---|---------|--------|------|--------|
| 0 | 1 | → stuffed | 1 | - |
| 1 | 1 | → stuffed | 2 | - |
| 2 | 1 | → stuffed | 3 | - |
| 3 | 1 | → stuffed | 4 | - |
| 4 | 1 | → stuffed | 5 | **insert 0 into stuffed**, reset ones=0 |
| 5 | 1 | → stuffed | 1 | - |

Result: `1,1,1,1,1,[0],1` — the extra `[0]` breaks the run.

---

### Step 3 — Add Frame Flags

```c
k = 0;
for (i = 0; i < flagLen; i++) framed[k++] = flag[i];    // add start flag 01111110
for (i = 0; i < stuffedLen; i++) framed[k++] = stuffed[i]; // add stuffed data
for (i = 0; i < flagLen; i++) framed[k++] = flag[i];    // add end flag 01111110
framedLen = k;
```

Just concatenates: `[01111110] + [stuffed data] + [01111110]`

```
framed[] = 01111110  01101000 01101001  01111110
           ^start    ^data               ^end
```

This is what gets "transmitted".

---

### Step 4 — Receiver: Remove Stuffed Bits (Destuffing)

```c
ones = 0;
j = 0;
for (i = flagLen; i < framedLen - flagLen; i++) {   // skip start and end flags
    if (ones == 5) {
        if (framed[i] != 0) {
            printf("Error: Invalid stuffing detected\n");
            return;
        }
        ones = 0;
        continue;           // DROP this bit — it was stuffed, not real data
    }
    destuffed[j++] = framed[i];                     // copy real bit
    ones = (framed[i] == 1) ? ones + 1 : 0;
}
destuffedLen = j;
```

**Key points:**
- Loop starts at `i = flagLen` (= 8) to skip the start flag
- Loop ends before `framedLen - flagLen` to skip the end flag
- The logic mirrors stuffing: count consecutive 1s; when count hits 5, the **next** bit must be a stuffed 0 — so `continue` skips it instead of copying it
- If after 5 ones the next bit is NOT 0, that's an error — data was corrupted

---

### Step 5 — Convert Bits Back to String

```c
for (i = 0; i < destuffedLen; i += 8) {    // process 8 bits at a time
    for (b = 0; b < 8; b++) {
        bin_str[b] = (destuffed[i + b] == 1) ? '1' : '0';
    }
    bin_str[8] = '\0';
    out_ascii_arr[out_char_count++] = (int)bin_to_ascii(bin_str);
}
ascii_to_str(out_ascii_arr, out_char_count, outputString);
```

Every 8 bits are assembled into a binary string like `"01101000"`, then `bin_to_ascii` converts that back to the character `'h'`.

---

### Step 6 — Verify

```c
match = 1;
if (destuffedLen != n) match = 0;    // lengths must match
else {
    for (i = 0; i < n; i++) {
        if (destuffed[i] != data[i]) { match = 0; break; }  // bit-by-bit comparison
    }
}
printf(match ? "SUCCESS" : "FAIL");
```

Compares original `data[]` with `destuffed[]` bit by bit. If every bit matches → SUCCESS.

---

## BYTE STUFFING — `byteStuffing()` Full Explanation

### The Problem It Solves

Instead of bit-by-bit framing, here entire **bytes** are the unit. The flag byte `01111110` (= 126) marks frame boundaries. If that exact byte value appears inside the data, an escape byte is placed before it so the receiver knows it's data, not a delimiter.

---

### Step 1 — Get Flag and Escape Byte

```c
flagByte = 0b01111110;           // = 126 in decimal, always fixed
scanf("%s", ebits);              // user enters escape byte in binary e.g. "11111111"
escByte = (int)bin_to_ascii(ebits);   // converts binary string to integer, e.g. 255
```

`flagByte` is always `01111110`. The user defines the escape byte.

---

### Step 2 — Map Input Characters to Bytes

```c
for (i = 0; i < numBytes; i++) {
    if (inputString[i] == 'F') {
        dataBytes[i] = flagByte;       // treat 'F' as the flag byte value (126)
    } else if (inputString[i] == 'E') {
        dataBytes[i] = escByte;        // treat 'E' as the escape byte value
    } else {
        dataBytes[i] = ascii_arr[i];   // all other chars use their ASCII value
    }
}
```

This is a **convention**: since the user types text, we use `'F'` and `'E'` as stand-ins for the actual flag and escape byte values. This lets you test what happens when flag/escape values appear in the data.

Example for input `"HFi"` with flagByte=126, escByte=255:
```
'H' → 72
'F' → 126  (this IS the flag byte — needs stuffing!)
'i' → 105
dataBytes = [72, 126, 105]
```

---

### Step 3 — Stuff the Bytes

```c
j = 0;
for (i = 0; i < numBytes; i++) {
    if (dataBytes[i] == flagByte || dataBytes[i] == escByte) {
        stuffedBytes[j++] = escByte;       // insert escape byte BEFORE the special byte
        stuffedBytes[j++] = dataBytes[i];  // then copy the actual byte
    } else {
        stuffedBytes[j++] = dataBytes[i];  // normal byte — just copy
    }
}
stuffedLen = j;
```

**Rule:** Whenever the byte value equals `flagByte` OR `escByte`, prefix it with `escByte`.

Why prefix `escByte` too? Because `escByte` itself could be mistaken for the start of an escape sequence at the receiver. So it must also be escaped.

Tracing `"HFi"` (dataBytes = [72, 126, 105]):

| i | dataBytes[i] | Special? | stuffedBytes gets |
|---|-------------|----------|-------------------|
| 0 | 72 (H) | No | [72] |
| 1 | 126 (F = flag!) | YES | [255, 126] — escape + original |
| 2 | 105 (i) | No | [105] |

```
stuffedBytes = [72, 255, 126, 105]
```

---

### Step 4 — Add Frame Delimiters

```c
k = 0;
framedBytes[k++] = flagByte;                 // start flag
for (i = 0; i < stuffedLen; i++)
    framedBytes[k++] = stuffedBytes[i];      // stuffed data
framedBytes[k++] = flagByte;                 // end flag
framedLen = k;
```

Result:
```
framedBytes = [126, 72, 255, 126, 105, 126]
               ^flag H  esc  F    i   ^flag
```

---

### Step 5 — Validate Frame

```c
if (framedBytes[0] != flagByte || framedBytes[framedLen - 1] != flagByte) {
    printf("Error: frame missing flags\n");
    return;
}
```

Sanity check — first and last byte must be the flag. If not, something went wrong.

---

### Step 6 — Receiver: Remove Escape Bytes (Destuffing)

```c
m = 0;
for (i = 1; i < framedLen - 1; i++) {         // skip first and last flag byte
    if (framedBytes[i] == escByte) {           // found an escape byte
        if (i + 1 >= framedLen - 1) {
            printf("Dangling esc byte\n");
            error = 1; break;
        }
        i++;                                   // skip the escape byte itself
        destuffedBytes[m++] = framedBytes[i];  // copy the byte that follows
    } else if (framedBytes[i] == flagByte) {
        printf("Error: unexpected flag inside frame\n");
        error = 1; break;
    } else {
        destuffedBytes[m++] = framedBytes[i];  // normal byte — just copy
    }
}
```

**Three cases at receiver:**
1. **Escape byte found** → skip the escape, copy the next byte as real data (`i++` then copy)
2. **Flag byte found in middle** → error! A flag byte inside the frame means stuffing was done wrong
3. **Normal byte** → copy as-is

Tracing `framedBytes = [126, 72, 255, 126, 105, 126]`:

| i | framedBytes[i] | Case | Action |
|---|---------------|------|--------|
| 0 | 126 | — | skipped (loop starts at i=1) |
| 1 | 72 | Normal | destuffedBytes[0] = 72 |
| 2 | 255 | Escape! | i becomes 3, skip to next |
| 3 | 126 | (after escape) | destuffedBytes[1] = 126 |
| 4 | 105 | Normal | destuffedBytes[2] = 105 |
| 5 | 126 | — | skipped (loop ends before framedLen-1) |

```
destuffedBytes = [72, 126, 105]  =  original dataBytes ✅
```

---

### Step 7 — Map Back to String

```c
for (i = 0; i < destuffedLen; i++) {
    if (destuffedBytes[i] == flagByte) out_ascii_arr[i] = 'F';     // 126 → 'F'
    else if (destuffedBytes[i] == escByte) out_ascii_arr[i] = 'E'; // 255 → 'E'
    else out_ascii_arr[i] = destuffedBytes[i];                      // others → ASCII
}
ascii_to_str(out_ascii_arr, destuffedLen, outputString);
```

Reverse of Step 2: the byte values are mapped back to the characters the user typed.

---

### Step 8 — Verify

```c
if (destuffedLen == numBytes) {
    match = 1;
    for (i = 0; i < numBytes; i++) {
        if (destuffedBytes[i] != dataBytes[i]) { match = 0; break; }
    }
    printf(match ? "SUCCESS" : "FAIL");
} else {
    printf("Length mismatch after destuff\n");
}
```

Compares original `dataBytes[]` with `destuffedBytes[]` value by value. Both length and content must match.

---

## The Main Loop

```c
int main() {
    while (1) {              // runs forever
        print menu
        scanf choice

        switch (choice) {
            case 1: bitStuffing();    break;
            case 2: byteStuffing();   break;
            case 3: return 0;         // only exit here
            default: print error;     break;
        }
    }
}
```

The `while(1)` loop keeps the menu alive after each run. It only exits when the user explicitly types `3`. Any other input that isn't 1, 2, or 3 hits `default` and loops again.

---

## Full Flow Summary

```
INPUT TEXT
    │
    ▼
[str_to_ascii] → ASCII integers
    │
    ▼
[ascii_to_bin] → Flat bit array  (bit stuffing)
    or
    byte values  (byte stuffing)
    │
    ▼
[STUFF] → Insert 0 after 5 ones  (bit)
          OR escape before flag/esc bytes  (byte)
    │
    ▼
[FRAME] → Wrap with 01111110 flag  (both)
    │
    ▼ (transmitted)
    │
    ▼
[SKIP flags] → remove start/end markers
    │
    ▼
[DESTUFF] → Remove stuffed 0s  (bit)
            OR remove escape prefixes  (byte)
    │
    ▼
[Convert back] → bits to ASCII to string  (bit)
                 bytes to characters  (byte)
    │
    ▼
OUTPUT TEXT + VERIFICATION
```

---

## Comparison Table

| | Bit Stuffing | Byte Stuffing |
|---|---|---|
| Works on | Individual bits | Whole bytes (8-bit groups) |
| Delimiter | `01111110` (8 bits) | `01111110` (1 byte = 126) |
| Stuffing rule | Insert `0` after 5 consecutive `1`s | Insert escape byte before flag/escape byte |
| Destuffing rule | See 5 ones → next bit is fake, skip it | See escape byte → next byte is data, copy it |
| Overhead | +1 bit per 5 ones in data | +1 byte per special byte in data |
| Error detection | If bit after 5 ones is not 0 → error | If flag appears without escape → error |

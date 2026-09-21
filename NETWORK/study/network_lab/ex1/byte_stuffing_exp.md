# byte_stuffing.c — Byte (Character) Stuffing with SOF / EOF / ESC bytes

## What the file is about

In byte-oriented framing the frame is marked by special bytes: a **Start-Of-Frame (SOF)** byte at the
beginning and an **End-Of-Frame (EOF)** byte at the end (both default to `01111110` = 0x7E, like HDLC). If the
*data* happens to contain a byte equal to SOF or EOF, the receiver would think the frame ended early.

**Byte stuffing** fixes this with a third special byte, the **escape (ESC)**: before every data byte that looks
like SOF, EOF or ESC, the sender inserts an ESC. The receiver, whenever it sees ESC, throws it away and takes
the *next* byte literally as data.

The program simulates that. Because you cannot easily type the byte 0x7E or 0x1B on a keyboard, the program
uses a **typing convention**: in the input text, the capital letter `F` means "a byte equal to SOF" and `E`
means "a byte equal to ESC". (`run.md` sample: `AFEB` → A, [SOF-byte], [ESC-byte], B.)

Steps: read the three special bytes → read the text → build data bytes → stuff → frame → optional corruption →
receive (check, unstuff, compare) → print.

---

## Test cases (whole program)

Menu choice `2`. In all cases below: SOF `1` (default), EOF `1` (default), ESC `00011011` (= 27 = 0x1B).

### TC1 — Positive (run.md sample): data `AFEB`, no corruption

```
SOF  = 01111110
EOF  = 01111110
Esc  = 00011011
Original data: 01000001 01111110 00011011 01000010
Stuffed data : 01000001 00011011 01111110 00011011 00011011 01000010
Framed data  : 01111110 01000001 00011011 01111110 00011011 00011011 01000010 01111110
--- RECEIVER SIDE ---
Destuffed data: 01000001 01111110 00011011 01000010
Output Text   : ASEB
Data matches perfectly!
```

How: `F` was replaced by the SOF value (`01111110`), `E` by the ESC value (`00011011`). Stuffing puts an ESC
before each of them: 4 data bytes → 6 stuffed bytes. Framing adds SOF at the front and EOF at the back → 8
bytes. The receiver sees ESC, skips it and takes the next byte as data (twice) → 4 bytes, identical to the
original → success.

Why is the output `ASEB` and not `AFEB`? `mapChar()` prints special values as letters: SOF → `S`, EOF → `F`,
ESC → `E`. Since SOF and EOF are the same value here (both default), the check `b == sofByte` is tested first
and wins, so that byte prints as `S`. If you gave SOF and EOF different values you would see `F`.

### TC2 — Negative: corrupt index 0 (the SOF byte) to 65 (`A`)

```
Corrupted data: 01000001 01000001 00011011 01111110 00011011 00011011 01000010 01111110
--- RECEIVER SIDE ---
Actual data:    SAESEEBS
Changed data:   AAESEEBS
Both don't match, so message discarded.
```

How: `receive()` starts with `error = framedBytes[0] != sofByte || …` → the first byte is now `A`, not SOF →
error immediately, unstuffing loop is skipped, message discarded. The two text lines are the frame printed
through `mapChar` (S = SOF, E = ESC).

### TC3 — Negative: data `hello`, corrupt index 2 to 126 (a raw SOF value appears inside the data)

```
Framed data  : 01111110 01101000 01100101 01101100 01101100 01101111 01111110
Corrupted data: 01111110 01101000 01111110 01101100 01101100 01101111 01111110
Actual data:    ShelloS
Changed data:   ShSlloS
Both don't match, so message discarded.
```

How: an **unescaped** SOF/EOF value in the payload is illegal — a real receiver would think the frame ended.
The branch `else if (framedBytes[i] == sofByte || framedBytes[i] == eofByte) error = 1;` catches it.

### TC4 — Negative: data `AFEB`, corrupt index 2 (the ESC that protects the SOF-byte) to 65

```
Corrupted data: 01111110 01000001 01000001 01111110 00011011 00011011 01000010 01111110
Actual data:    SAESEEBS
Changed data:   SAASEEBS
Both don't match, so message discarded.
```

How: with the ESC gone, the next byte (`01111110`) is now a bare SOF inside the payload → same rule as TC3 → error.

### TC5 — Positive: plain text with a space: `hi there`, no corruption

```
Original data: 01101000 01101001 00100000 01110100 01101000 01100101 01110010 01100101
Stuffed data : (identical – nothing special inside)
Framed data  : 01111110 … 01111110
Destuffed data: (identical)
Output Text   : hi there
Data matches perfectly!
```

How: `scanf(" %99[^\n]", …)` reads the whole line including the space (unlike bit stuffing's `%s`). No byte
equals SOF/EOF/ESC, so stuffing is a no-op and framing just adds the two markers.

### TC6 — Edge: invalid corrupt index (e.g. `50`)

`Invalid index. Proceeding without byte manipulation.` → receiver gets the clean frame → success.

### TC7 — Edge: ESC as the very last payload byte with nothing after it

If corruption changes the byte just before EOF into the ESC value, the receiver sees ESC with no following
data byte: `if (i + 1 >= framedLen - 1) error = 1;` → discarded. (E.g. `hello`, corrupt index 5 → 27.)

### TC8 — Edge: corrupt a byte to the *same* value it already had

`receive()` finds no difference → "Data matches perfectly!". The program doesn't know a "corruption" happened
because nothing actually changed.

### TC9 — Edge: SOF, EOF and ESC all different, e.g. SOF `00000001`, EOF `00000010`, ESC `00000011`, data `AFEB`

Output text becomes `AFEB` (SOF-valued byte prints as `F` because `sofByte != eofByte` now). Same stuffing
logic applies.

---

## Constants and global data

```c
#define MAX_STR   100
#define MAX_BYTES 500
```
`MAX_STR` = longest input text; `MAX_BYTES` = size of the byte arrays (100 data bytes can at most double to
200 with stuffing, + 2 markers — 500 is generous).

```c
static int dataBytes[MAX_BYTES], stuffedBytes[MAX_BYTES], framedBytes[MAX_BYTES], destuffedBytes[MAX_BYTES];
```
The four stages of the frame as int arrays (one int per byte, values 0–255). File-scope so that `receive()`
can access them without long parameter lists.

```c
static int sofByte, eofByte, escByte;
```
The three special byte values chosen by the user. Global because `mapChar()`, `receive()` and
`byteStuffing()` all need them.

---

## Function 1: `printByte`

```c
static void printByte(int byte)
```

**What:** prints one integer as exactly 8 binary digits, MSB first.
**Input:** `byte` (0–255). **Output:** e.g. `01111110` on screen (no newline).
**Why:** this file never converts bytes to text strings; it prints the bits directly.
**Where called:** `printBytesAsBinary()` and the three `SOF/EOF/Esc =` lines in `byteStuffing()`.

```c
    for (int i = 7; i >= 0; i--)
        printf("%d", (byte >> i) & 1);
```
Loop from bit 7 down to bit 0. `byte >> i` shifts the wanted bit into position 0, `& 1` keeps just that bit
(0 or 1), `%d` prints it. For 126 (`01111110`): i=7 → 0, i=6 → 1, … i=0 → 0.

---

## Function 2: `printBytesAsBinary`

```c
static void printBytesAsBinary(const char *label, const int *arr, int n)
```

**What:** prints a label then `n` bytes in binary separated by spaces, then newline.
**Input:** label text, int array, count. **Output:** e.g. `Original data: 01000001 01111110 …`.
**Why:** used for Original / Stuffed / Framed / Corrupted / Destuffed lines.
**Where called:** `byteStuffing()` only.

```c
    printf("%s", label);
    for (int i = 0; i < n; i++) {
        printByte(arr[i]);
        printf(" ");
    }
    printf("\n");
```
Print label; for each byte call `printByte` and add a space; end line. (A trailing space is left after the last byte — harmless.)

---

## Function 3: `mapChar`

```c
static int mapChar(int b)
```

**What:** converts a byte value into a *displayable* character: special bytes become the letters `S`, `F`, `E`;
everything else is returned unchanged.
**Input:** byte value. **Output:** `'S'` if it equals SOF, `'F'` if EOF, `'E'` if ESC, otherwise `b` itself.
**Why:** SOF (0x7E is `~`, fine) but ESC (0x1B) is an invisible control character; printing letters makes the
frame readable in the "Actual/Changed data" lines and in the final output text.
**Where called:** `printText()` and the final output loop in `byteStuffing()`.

```c
    return b == sofByte ? 'S' : b == eofByte ? 'F' : b == escByte ? 'E' : b;
```
A chain of ternaries, evaluated left to right: first match wins. That ordering explains TC1's `ASEB`: when
SOF == EOF, the byte matches `sofByte` first and prints `S`.

---

## Function 4: `printText`

```c
static void printText(const char *label, const int *arr, int n)
```

**What:** prints a label then the bytes as characters (through `mapChar`). No newline at the end (callers add it).
**Where called:** the error branch of `byteStuffing()` ("Actual data" / "Changed data").

```c
    printf("%s", label);
    for (int i = 0; i < n; i++)
        printf("%c", (char)mapChar(arr[i]));
```
`%c` prints one character; the `(char)` cast narrows the int.

---

## Function 5: `readFrameByte`

```c
static int readFrameByte(const char *name)
```

**What:** asks the user for one 8-bit pattern (or `1` for the default `01111110`) and returns its numeric value.
**Input:** `name` — text used in the prompt ("Start of Frame" / "End of Frame"). Keyboard input.
**Output:** the byte value as an int (e.g. 126).
**Why:** both SOF and EOF are read the same way; one function avoids duplicated code.
**Where called:** twice at the top of `byteStuffing()`.

```c
    char s[9];
    printf("Enter %s (Press 1 for default 01111110): ", name);
    scanf("%8s", s);
```
Buffer of 9 (8 chars + `'\0'`); `%8s` limits input to 8 characters so it can't overflow.

```c
    return strcmp(s, "1") == 0 ? 0x7E : (int)bin_to_ascii(s);
```
If the user typed exactly `"1"`, return `0x7E` (= 126 = `01111110`). Otherwise convert the 8-character
pattern with `bin_to_ascii` (from `convert.c`) and cast to int. Note: if the user types fewer than 8 digits
(other than `1`), `bin_to_ascii` reads past the terminator (see `convert_exp.md` negative test 16) — the
program trusts the user here.

---

## Function 6: `receive`

```c
static int receive(int framedLen, int numBytes, int *destuffedLen)
```

**What:** receiver logic — verifies SOF/EOF, removes ESC bytes, rejects bare special bytes, compares with the original data.
**Input:** `framedLen` (bytes in the frame), `numBytes` (original data byte count), `destuffedLen` (output: bytes recovered). Reads the globals `framedBytes[]`, `dataBytes[]`; writes `destuffedBytes[]`.
**Output:** `1` if any error, `0` if perfect.
**Why:** isolates the receiver's rules so they are easy to read and test.
**Where called:** once in `byteStuffing()`.

```c
    int error = framedBytes[0] != sofByte || framedBytes[framedLen - 1] != eofByte, m = 0;
```
`error` is initialised directly from the two marker checks: first byte must be SOF and last byte must be EOF.
`m` counts recovered bytes. (TC2 sets `error = 1` here.)

```c
    for (int i = 1; !error && i < framedLen - 1; i++) {
```
Walk the payload — indices 1 … framedLen-2 (skipping the two markers). `!error` short-circuits the loop when a marker was bad.

```c
        if (framedBytes[i] == escByte) {
            if (i + 1 >= framedLen - 1)
                error = 1;
            else
                destuffedBytes[m++] = framedBytes[++i];   /* take the escaped byte as data */
        }
```
ESC seen. If it is the last payload byte there is nothing to escape → error (TC7). Otherwise `++i` moves to the
following byte *before* the read, and that byte is stored as data regardless of its value — that is the whole
point of escaping. The ESC itself is dropped.

```c
        else if (framedBytes[i] == sofByte || framedBytes[i] == eofByte)
            error = 1;
```
A SOF or EOF value that was **not** preceded by ESC is illegal inside the payload (TC3, TC4).

```c
        else
            destuffedBytes[m++] = framedBytes[i];
```
Ordinary byte → copy.

```c
    }
    *destuffedLen = m;
    return error || m != numBytes || memcmp(destuffedBytes, dataBytes, numBytes * sizeof(int)) != 0;
```
Return the length and the verdict: error flag, or wrong count, or bytes differ (`memcmp` compares raw memory of the two int arrays; 0 = identical).

---

## Function 7: `byteStuffing` (main flow)

```c
void byteStuffing(void)
```

**What:** runs the full demo. **Input:** keyboard. **Output:** console trace. **Where called:** `main.c` → `case 2`. Declared in `ex1.h`.

```c
    char inputString[MAX_STR], ebits[9], outputString[MAX_STR];
    int ascii_arr[MAX_STR], out_ascii_arr[MAX_STR], backup[MAX_BYTES];
    int numBytes, stuffedLen = 0, framedLen, destuffedLen, wantToCorrupt = 0;
```
`ebits` — 8-char ESC pattern typed by the user. `backup` — intact copy of the frame for the error printout. The rest are as in bit stuffing.

```c
    printf("\n--- BYTE STUFFING CONFIGURATION ---\n");
    sofByte = readFrameByte("Start of Frame");
    eofByte = readFrameByte("End of Frame");
```
Ask for the two frame markers.

```c
    printf("Enter Escape byte (8 bits): ");
    scanf("%8s", ebits);
    escByte = (int)bin_to_ascii(ebits);
```
Ask for the ESC pattern (no default) and convert it to a number.

```c
    printf("Enter the data input string: ");
    scanf(" %99[^\n]", inputString);
```
`%[^\n]` = "read every character that is not a newline" → whole line including spaces (TC5). The leading
space in the format skips the newline left over from the previous `scanf`. `99` limits the length.

```c
    str_to_ascii(inputString, ascii_arr, &numBytes);
```
ASCII codes of each typed character.

```c
    for (int i = 0; i < numBytes; i++)
        dataBytes[i] = inputString[i] == 'F' ? sofByte : inputString[i] == 'E' ? escByte : ascii_arr[i];
```
Apply the typing convention: `F` → SOF value, `E` → ESC value, anything else → its own ASCII code. This is how
special bytes get into the data without needing to type control characters.

```c
    printf("\n--- TRANSMITTER SIDE ---\n");
    printf("SOF  = "); printByte(sofByte);
    printf("\nEOF  = "); printByte(eofByte);
    printf("\nEsc  = "); printByte(escByte);
    printf("\n");
    printBytesAsBinary("Original data: ", dataBytes, numBytes);
```
Echo the configuration and the raw data.

```c
    for (int i = 0; i < numBytes; i++) {
        if (dataBytes[i] == sofByte || dataBytes[i] == eofByte || dataBytes[i] == escByte)
            stuffedBytes[stuffedLen++] = escByte;
        stuffedBytes[stuffedLen++] = dataBytes[i];
    }
```
**Stuffing.** For each data byte: if it equals any special value, first emit an ESC; then emit the byte itself.
ESC must be escaped too, otherwise the receiver couldn't tell a data-ESC from a real ESC.

```c
    printBytesAsBinary("Stuffed data : ", stuffedBytes, stuffedLen);
```

```c
    framedBytes[0] = sofByte;
    memcpy(framedBytes + 1, stuffedBytes, stuffedLen * sizeof(int));
    framedBytes[stuffedLen + 1] = eofByte;
    framedLen = stuffedLen + 2;
```
**Framing:** SOF, then the stuffed bytes copied starting at index 1, then EOF. Length = payload + 2.

```c
    printBytesAsBinary("Framed data  : ", framedBytes, framedLen);
    memcpy(backup, framedBytes, framedLen * sizeof(int));
```
Show and back up.

```c
    printf("\nDo you want to corrupt a byte to simulate a network error? (1 = Yes, 0 = No): ");
    scanf("%d", &wantToCorrupt);
    if (wantToCorrupt == 1) {
        int corruptIndex = 0, newValue = 0;
        printf("Enter byte index to change (0 to %d): ", framedLen - 1);
        scanf("%d", &corruptIndex);
        if (corruptIndex >= 0 && corruptIndex < framedLen) {
            printf("Enter new decimal value for this byte (0-255): ");
            scanf("%d", &newValue);
            framedBytes[corruptIndex] = newValue;
            printf("Byte at index %d altered successfully!\n", corruptIndex);
            printBytesAsBinary("Corrupted data: ", framedBytes, framedLen);
        } else
            printf("Invalid index. Proceeding without byte manipulation.\n");
    }
```
Optional corruption: unlike bit stuffing (which flips one bit), here you replace an entire byte with a new
decimal value. Range check on the index protects the array (TC6). The new value is **not** range-checked: `printByte`
only shows the low 8 bits, so typing 256 displays as `00000000` while the array actually holds 256 (which
matches no real byte, so the receiver will still report a mismatch).

```c
    printf("\n--- RECEIVER SIDE ---\n");
    if (receive(framedLen, numBytes, &destuffedLen)) {
        printText("Actual data:    ", backup, framedLen);
        printText("\nChanged data:   ", framedBytes, framedLen);
        printf("\nBoth don't match, so message discarded.\n");
        return;
    }
```
Receiver failed → print both frames as letters (S/F/E for specials) and leave.

```c
    printBytesAsBinary("Destuffed data: ", destuffedBytes, destuffedLen);
    for (int i = 0; i < destuffedLen; i++)
        out_ascii_arr[i] = mapChar(destuffedBytes[i]);
    ascii_to_str(out_ascii_arr, destuffedLen, outputString);
    printf("Output Text   : %s\n", outputString);
    printf("Data matches perfectly!\n");
```
Success path: show the recovered bytes, convert them to displayable characters via `mapChar` (so the
SOF/ESC-valued bytes show as letters rather than `~` / invisible control code), build the string, print.

---

## `#include` lines

- `<stdio.h>` — `printf`, `scanf`.
- `<string.h>` — `strcmp`, `memcpy`, `memcmp`.
- `"../convert/convert.h"` — `bin_to_ascii`, `str_to_ascii`, `ascii_to_str`.
- `"ex1.h"` — prototype of `byteStuffing` so `main.c` can call it.

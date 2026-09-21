# convert.h — Interface (header) for the conversion helpers

A header file contains **declarations**, not code. It tells every `.c` file that includes it: "these four
functions exist somewhere, here is how to call them". The actual bodies live in `convert.c`
(see `convert_exp.md` for their full explanation).

Files that include this header: `ex1/bit_stuffing.c`, `ex1/byte_stuffing.c`, `ex2/utils.c`
(all via `#include "../convert/convert.h"`) and `convert.c` itself.

---

## Test cases

A header has no runnable logic, so the "tests" are compile-time behaviours.

| # | Situation | Result | Why |
|---|-----------|--------|-----|
| 1 (positive) | `ex1/bit_stuffing.c` includes the header and calls `ascii_to_bin('A', buf)` | compiles; links when `../convert/convert.c` is also compiled | the prototype matches the definition in `convert.c`. |
| 2 (positive) | Two files in the same build both include `convert.h` (`bit_stuffing.c` and `byte_stuffing.c`) | no "redefinition" error | each `.c` file is compiled separately; within one file the include guard prevents a double include. |
| 3 (positive) | A file includes `convert.h` twice by accident (directly and through another header) | compiles cleanly | `#ifndef CONVERT_H` is false the second time, so the body is skipped. |
| 4 (negative) | Compile `gcc *.c -o ex1` inside `ex1/` **without** `../convert/convert.c` | `undefined reference to 'ascii_to_bin'` at link time | the header only promises the function; the linker cannot find its body. This is the error listed in `run.md` §4. |
| 5 (negative) | Calling `ascii_to_bin(65)` with one argument | compile error: too few arguments | the prototype says 2 parameters; the compiler enforces it. Without the header, C would have silently accepted the bad call. |
| 6 (negative) | Calling `bin_to_ascii(5)` (an int instead of a `const char *`) | compiler warning/error: incompatible pointer type | prototype type-checking again. |
| 7 (edge) | Move `convert/` somewhere else | `convert.h: No such file or directory` | the include path `../convert/convert.h` is relative to the experiment folder. |

---

## Line by line

```c
#ifndef CONVERT_H
#define CONVERT_H
```
**Include guard.** `#ifndef` = "if not defined". The first time the preprocessor sees this file, `CONVERT_H`
is not yet defined, so it defines it and processes the rest. If the same file is included again in the same
translation unit, `CONVERT_H` is already defined, everything down to `#endif` is skipped, and the compiler
never sees the declarations twice (which would otherwise be an error for some kinds of declarations, and
noise for the rest). The name `CONVERT_H` is a convention: file name in capitals with `.` → `_`.

```c
void ascii_to_bin(char ascii, char *bin_str);                 /* 'A' -> "01000001"  */
```
Prototype for the function that converts one character to an 8-character binary string.
- `void` — returns nothing; the result goes into `bin_str`.
- `char ascii` — the character to convert.
- `char *bin_str` — pointer to a buffer of **9** chars (8 digits + `'\0'`) that will be written.
The trailing comment shows an example so a reader doesn't need to open `convert.c`.

```c
char bin_to_ascii(const char *bin_str);                       /* "01000001" -> 'A'  */
```
Prototype for the inverse. Returns a `char`. Takes `const char *` — `const` documents that the input is only read, and lets the caller pass string literals safely.

```c
void str_to_ascii(const char *str, int *ascii_arr, int *len); /* string -> ASCII ints */
```
Prototype: whole string → int array. Two output parameters (`ascii_arr` gets filled, `*len` gets the count). Pointers are used because C functions can only *return* one value.

```c
void ascii_to_str(const int *ascii_arr, int len, char *str);  /* ASCII ints -> string */
```
Prototype: int array (with explicit length, since an int array has no terminator) → C string. `str` must have room for `len + 1` characters.

```c
#endif
```
Closes the `#ifndef` block opened on line 1.

---

## Why keep this in a separate header instead of declaring in each `.c` file?

- One place to change if a signature changes.
- The compiler checks that `convert.c`'s definitions agree with what callers expect (it includes its own header).
- Callers in different folders (`ex1`, `ex2`) get identical declarations.

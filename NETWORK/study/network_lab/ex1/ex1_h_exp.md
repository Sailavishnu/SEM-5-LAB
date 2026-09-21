# ex1.h — Header for experiment 1 (Bit / Byte stuffing)

## What the file is about

A tiny header that declares the two technique functions so that `main.c` can call them and so that
`bit_stuffing.c` / `byte_stuffing.c` define them with exactly the agreed signature.

Included by: `main.c`, `bit_stuffing.c`, `byte_stuffing.c`.

---

## Test cases

| # | Type | Situation | Result | Why |
|---|------|-----------|--------|-----|
| 1 | Positive | Build with `gcc *.c ../convert/convert.c -o ex1` | links and runs | `main.c` sees the prototypes; the linker finds the bodies in the other two `.c` files. |
| 2 | Positive | Include `ex1.h` twice in one file | no error | include guard `EX1_H`. |
| 3 | Negative | Delete `ex1.h` and compile | `ex1.h: No such file or directory` in all three files | every file includes it. |
| 4 | Negative | Remove `#include "ex1.h"` from `main.c` only | modern gcc: error "implicit declaration of function 'bitStuffing'" (C99 forbids calling undeclared functions) | the header is what declares the function to `main.c`. |
| 5 | Negative | Change the prototype to `void bitStuffing(int);` but leave the definition as `void bitStuffing(void)` | compile error "conflicting types" in `bit_stuffing.c` | `bit_stuffing.c` includes the header, so the compiler compares declaration and definition. This is the safety benefit of every `.c` including its own header. |
| 6 | Edge | Compile only `main.c` (`gcc main.c`) | `undefined reference to bitStuffing` | prototype present, definition missing → link error, not compile error. |

---

## Line by line

```c
#ifndef EX1_H
#define EX1_H
```
Include guard: the first inclusion defines `EX1_H`; any later inclusion in the same translation unit skips
everything to `#endif`.

```c
void bitStuffing(void);
```
Declares the bit-stuffing demo. Takes no parameters (`void` in the parentheses — in C an empty `()` would mean
"unspecified parameters", so `(void)` is the correct way to say "none"), returns nothing (all output is
printed inside). Defined in `bit_stuffing.c`; called from `main.c` case 1.

```c
void byteStuffing(void);
```
Same for the byte-stuffing demo. Defined in `byte_stuffing.c`; called from `main.c` case 2.

```c
#endif
```
Ends the include guard.

Note: constants like `MAX_STR`, `MAX_BITS` are **not** here — each technique file defines its own with
`#define`, because `bit_stuffing.c` needs `MAX_BITS 1000` while `byte_stuffing.c` needs `MAX_BYTES 500`
and neither needs the other's.

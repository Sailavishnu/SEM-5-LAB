# How to compile and run all the programs

## 1. Folder layout (keep it exactly like this)

```
network/
├── convert/   convert.c  convert.h  run.md      <- shared helper (ASCII <-> binary)
├── ex1/       main.c  bit_stuffing.c  byte_stuffing.c  ex1.h
├── ex2/       main.c  parity.c  crc.c  checksum.c  utils.c  ex2.h
├── ex3/       main.c  encode.c  decode.c  common.c  hamming.h
├── ex4/       main.c  matrix.c  distance_vector.c  link_state.c  router.h
└── ex5/       main.c  stop_and_wait.c  go_back_n.c  selective_repeat.c  window.h
```

* `ex1` and `ex2` use the `convert` folder, so it must stay **next to them** (they include `../convert/convert.h`).
* `ex3`, `ex4`, `ex5` are standalone.
* You need **gcc** (Linux/macOS: already there or `sudo apt install gcc`; Windows: MinGW / MSYS2 / the VS Code C setup you already use).
* Open a terminal **inside the experiment folder** first (`cd ex1`, `cd ex2` ...).

## 2. Compile and run

The main file of every experiment is `main.c` (it only has the menu). The other `.c` files are the individual techniques.
`*.c` means "compile every .c file in this folder", so you never have to list them.

| Exp | Topic | Compile (inside the folder) |
|-----|-------|-----------------------------|
| EX1 | Bit stuffing / Byte stuffing | `gcc *.c ../convert/convert.c -o ex1` |
| EX2 | Parity / CRC / Checksum | `gcc *.c ../convert/convert.c -o ex2` |
| EX3 | Hamming code | `gcc *.c -o ex3` |
| EX4 | Distance Vector / Link State | `gcc *.c -o ex4` |
| EX5 | Stop-and-Wait / Go-Back-N / Selective Repeat | `gcc *.c -o ex5` |

Run:

* Linux / macOS / Git Bash: `./ex1`
* Windows CMD: `ex1.exe`
* Windows PowerShell: `.\ex1.exe`

Example (EX1):

```
cd ex1
gcc *.c ../convert/convert.c -o ex1
./ex1
```

### Build everything in one go (Linux / macOS / Git Bash), from the `network` folder

```
(cd ex1 && gcc *.c ../convert/convert.c -o ex1)
(cd ex2 && gcc *.c ../convert/convert.c -o ex2)
(cd ex3 && gcc *.c -o ex3)
(cd ex4 && gcc *.c -o ex4)
(cd ex5 && gcc *.c -o ex5)
```

Want to study only one technique? Just open its own file (for example `ex1/bit_stuffing.c` or `ex2/crc.c`); `main.c` only calls it from the menu.

## 3. Sample inputs (type these when the program asks)

**EX1** - menu `1` = Bit stuffing, `2` = Byte stuffing, `3` = Exit
* Bit stuffing: choice `1`, string `~~z`, flip a bit? `0`   (try `1` then index `10` to see the error case)
* Byte stuffing: choice `2`, SOF `1`, EOF `1`, escape `00011011`, data `AFEB`, corrupt? `0`
  (`F` in the text means a SOF-valued byte, `E` means an ESC-valued byte)

**EX2** - menu `1` = Parity, `2` = CRC, `3` = Checksum, `4` = Exit
* Parity: choice `1`, message `Hi`, scheme `1` (even), bits to flip `0`
  (try `2` flips at positions `0` and `1` to see an even number of errors slip through)
* CRC: choice `2`, message `Hi`, generator `100000111`, error position `-1` (no error) or e.g. `5`
  Note: the default `1001` is always rejected by the generator checker (it divides x^3 + 1), so type a valid one such as `100000111`.
* Checksum: choice `3`, message `Hi`, block size `0` (default 8), error position `-1` or `5`
* Message can also be raw bits: put a `b` in front, e.g. `b1011010111`

**EX3** - Hamming code (asks once, then exits)
* Encode: choice `1`, parity `0` (even), data `1011010`
* Check/correct: choice `2`, parity `0`, received code `10101010111`

**EX4** - Routing simulator (option 1 first!)
* Option `1`, routers `4`, then the matrix (use `999` for no link):
  ```
  0 2 999 1
  2 0 3 999
  999 3 0 4
  1 999 4 0
  ```
* Then `3` (Distance Vector), `4` (Link State, source `A`), `6` (pair `A` and `D`), `5` (change a cost), `7` (exit)

**EX5** - Sliding window
* Choice `2` (Go-Back-N), frames `8`, frame to lose `3`, window size `4`
* Use `-1` as "frame to lose" for a run with no loss

## 4. Common problems

| Problem | Fix |
|---------|-----|
| `convert.h: No such file` | The `convert` folder must be next to `ex1` / `ex2`, and you must compile from inside `ex1` / `ex2`. |
| `undefined reference to ascii_to_bin ...` | You forgot `../convert/convert.c` in the compile command (EX1 / EX2 only). |
| `undefined reference to main` or `multiple definition of main` | Only one `main.c` per folder, and compile only the files of that folder. |
| `gcc` is not recognized (Windows) | Install MinGW/MSYS2 and add its `bin` folder to PATH, or compile from the VS Code terminal you normally use. |
| Emoji shows as `?` in EX5 (Windows CMD) | Only the clock emoji in Stop-and-Wait; run `chcp 65001` first or ignore it. |
| Program keeps asking for input in a loop | It is waiting for valid input; type a valid value (for example a valid CRC generator) or press Ctrl+C. |

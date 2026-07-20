# Error Detection Techniques — Test Cases & Answers

All outputs verified by running `main.c`. Menu: `1`=1D Parity, `2`=2D Parity, `3`=CRC, `4`=Checksum.

---

## 1. 1D Parity

Append 1 parity bit so total 1's matches chosen scheme. Error if count breaks the scheme.

| # | Scheme | Input | Data Sent | Received | 1's | Result | Reason |
|---|--------|-------|-----------|----------|-----|--------|--------|
| 1.1 | Even | Hi | `01001000011010010` | same (no flip) | 6 (even) | **No Error** | even count matches EVEN scheme |
| 1.2 | Even | Hi | `01001000011010010` | `11001000011010010` (bit 0 flipped) | 7 (odd) | **Error** | odd count breaks EVEN scheme |
| 1.3 | Odd | Hi | `01001000011010011` | same (no flip) | 7 (odd) | **No Error** | odd count matches ODD scheme |
| 1.4 | Odd | Hi | `01001000011010011` | `11001000011010011` (bit 0 flipped) | 8 (even) | **Error** | even count breaks ODD scheme |

---

## 2. 2D Parity

Each 8-bit char = one row; row parity + column parity added; matrix flattened to send.

| # | Scheme | Input | Data Sent | Received | Result | Reason |
|---|--------|-------|-----------|----------|--------|--------|
| 2.1 | Even | Hi | `010010000011010010001000010` | same (no flip) | **No Error** | every row/col has EVEN count |
| 2.2 | Even | Hi | `010010000011010010001000010` | bit 4 flipped | **Error** | Row 0 & Column 4 mismatch |
| 2.3 | Odd | Hi | `010010001011010011110111101` | same (no flip) | **No Error** | every row/col has ODD count |
| 2.4 | Odd | Hi | `010010001011010011110111101` | bit 4 flipped | **Error** | Row 0 & Column 4 mismatch |

---

## 3. CRC

Generator G(x) = `1001` (default). Append r=3 zeros, XOR-divide, remainder = CRC.
No error ⇔ remainder on receive = `000`.

| # | Input | Source Data | CRC | Transmitted | Received | Remainder | Result |
|---|-------|-------------|-----|-------------|----------|-----------|--------|
| 3.1 | Hi | `0100100001101001` | `101` | `0100100001101001101` | same (no flip) | `000` | **No Error** |
| 3.2 | Hi | `0100100001101001` | `101` | `0100100001101001101` | bit 1 flipped | `100` | **Error** |
| 3.3 | A | `01000001` | `000` | `01000001000` | same (no flip) | `000` | **No Error** |
| 3.4 | A | `01000001` | `000` | `01000001000` | bit 4 flipped | `001` | **Error** |

---

## 4. Checksum

Block size n=8 (default). Sum blocks (1's complement, end-around carry), checksum = complement of sum.
Receiver sums **all** blocks incl. checksum block; complement = `00000000` ⇒ No Error.

| # | Input | Blocks (data) | Checksum | Transmitted | Received | Receiver Sum | Complement | Result |
|---|-------|----------------|----------|-------------|----------|--------------|------------|--------|
| 4.1 | Hi | `01001000`, `01101001` | `01001110` | `010010000110100101001110` | same (no flip) | `11111111` | `00000000` | **No Error** |
| 4.2 | Hi | `01001000`, `01101001` | `01001110` | `010010000110100101001110` | bit 2 flipped | `00100000` | `11011111` | **Error** |
| 4.3 | A | `01000001` | `10111110` | `0100000110111110` | same (no flip) | `11111111` | `00000000` | **No Error** |
| 4.4 | A | `01000001` | `10111110` | `0100000110111110` | bit 5 flipped | `00000100` | `11111011` | **Error** |
# Lex Start Conditions — Only What You Need to Understand

## 1. What is a start condition?

A **start condition** is a mode/state that tells Lex which set of rules it should use at a particular point while scanning the input.

Think of it like a switch:

- **INITIAL mode:** use rules intended for the normal starting situation.
- **OP mode:** use rules intended for a situation where an operand has just been read.

The input is still scanned from left to right. A start condition changes which rules are eligible to match; it does not change the input itself.

## 2. Did we use start conditions in our programs?

**Yes. Both your Lex programs use a start condition named `OP`.**

They declare it using:

```lex
%s OP
```

This declares `OP` as an inclusive start condition. Lex begins in `INITIAL` by default.

In your programs, rules are separated by state-specific prefixes such as:

```lex
<INITIAL>"+"  { ... }
<OP>"+"       { ... }
```

These two rules match the same character `+`, but do different things depending on the current state.

- In `INITIAL`, `+` is labelled **unary plus**.
- In `OP`, `+` is labelled **binary plus**.

The programs also use this idea for `-`, `++`, and `--`.

## 3. How do we switch states?

Use:

```lex
BEGIN(INITIAL);
BEGIN(OP);
```

- `BEGIN(INITIAL)` switches to the normal state.
- `BEGIN(OP)` switches to the `OP` state.

In the supplied programs, identifiers, numbers, and Boolean literals generally switch to `OP` after they are recognized:

```lex
{NUMBER} { BEGIN(OP); /* return/print number */ }
{ID}     { BEGIN(OP); /* return/print identifier */ }
```

Why? After reading an operand such as `i`, `10`, or `true`, the next `+` or `-` is often a **binary** operator:

```text
i + 2
10 - 3
true && false
```

So the scanner switches to `OP`, where `+` and `-` are labelled binary.

When the scanner recognizes many operators or separators, it switches back to `INITIAL`. This makes the next `+` or `-` more likely to be treated as unary or prefix.

## 4. How does this distinguish unary and binary `+` and `-`?

### Unary operator: acts on one operand

Examples:

```text
+1
-5.5
```

The sign belongs to one value. The scanner is expected to be in `INITIAL`, so it uses:

```lex
<INITIAL>"+" { /* unary plus */ }
<INITIAL>"-" { /* unary minus */ }
```

### Binary operator: acts between two operands

Examples:

```text
i + 2
i - 2
```

The operator sits between a left operand and a right operand. After the scanner reads the left operand (`i`), it switches to `OP`. Therefore, it uses:

```lex
<OP>"+" { /* binary plus */ }
<OP>"-" { /* binary minus */ }
```

### Tiny trace

For:

```text
-5.5 + i - 2
```

The intended scanner-state idea is:

| Text | State before token | Token classification | State after token |
|---|---|---|---|
| `-` | `INITIAL` | unary minus | remains `INITIAL` in the supplied operator rule |
| `5.5` | `INITIAL` | number | switches to `OP` |
| `+` | `OP` | binary plus | switches to `INITIAL` |
| `i` | `INITIAL` | identifier | switches to `OP` |
| `-` | `OP` | binary minus | switches to `INITIAL` |
| `2` | `INITIAL` | number | switches to `OP` |

This is the intended pattern of state transitions. Exact labels depend on the rules and current state in the source program.

## 5. How does it distinguish prefix and postfix `++` / `--`?

`++` and `--` change a variable by one. Their position determines whether they are prefix or postfix.

- **Prefix:** operator comes before the variable: `++i`, `--i`.
- **Postfix:** operator comes after the variable: `i++`, `i--`.

The programs use state-specific rules like these:

```lex
<INITIAL>"++" { /* pre-increment */ }
<INITIAL>"--" { /* pre-decrement */ }

<OP>"++" { BEGIN(INITIAL); /* post-increment */ }
<OP>"--" { BEGIN(INITIAL); /* post-decrement */ }
```

The idea is:

- At the beginning of an expression, or after a separator/operator, the scanner is expected to be in `INITIAL`; `++`/`--` are labelled prefix.
- After reading a variable or number, the scanner is expected to be in `OP`; `++`/`--` are labelled postfix.

Examples:

| Input | Intended label |
|---|---|
| `++i` | pre-increment, then identifier `i` |
| `--i` | pre-decrement, then identifier `i` |
| `i++` | identifier `i`, then post-increment |
| `i--` | identifier `i`, then post-decrement |

## 6. The most important limitation: it is a heuristic

**A start condition does not understand grammar or meaning by itself.** It only remembers which mode the scanner is in.

These programs use a simple rule of thumb:

- `INITIAL` → likely beginning of an expression, so use unary/prefix labels.
- `OP` → an operand has just been read, so use binary/postfix labels.

This works for many examples, but it is not a complete solution for every expression. Parentheses, function calls, multiple operators, and more complicated syntax can expose mistakes if the rules do not switch states appropriately.

For example, the meaning of `-` in `a * -b` is unary minus even though an operator (`*`) came immediately before it. The scanner must switch to the correct state after `*` for the heuristic to work. If the multiplication rule leaves the state wrong, the `-` could be misclassified.

Also, `%s OP` declares an **inclusive** state. Rules without a state prefix remain eligible in `OP` as well; rules written `<INITIAL>` or `<OP>` are restricted to that state. Do not confuse this with an exclusive state, which is declared using `%x`.

## 7. Quick memory trick

Remember these two lines:

```text
INITIAL = expecting an operand -> unary +/-, prefix ++/--
OP      = operand was just read -> binary +/-, postfix ++/--
```

Then remember how to switch:

```lex
BEGIN(OP);       /* after an operand such as i, 10, or true */
BEGIN(INITIAL);  /* after an operator/separator when another operand is expected */
```

**One-line exam definition:** A Lex start condition is a scanner state that activates a selected set of lexical rules. In these programs, `INITIAL` and `OP` are used as a simple state-based heuristic to distinguish unary/binary `+` and `-`, and prefix/postfix `++` and `--`.

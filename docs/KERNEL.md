# Kernel — Core Primitives

## Pair (16-bit Word)

The pair `(a . d)` is the sole primitive data structure:

```c
typedef uint16_t Pair;

#define cons(a,d) ((((a) & 0xFF) << 8) | ((d) & 0xFF))
#define car(p)    (((p) >> 8) & 0xFF)
#define cdr(p)    ((p) & 0xFF)
```

- **car(p)**: high byte (address part)
- **cdr(p)**: low byte (data part)

## Kernel K(p, C)

```c
static inline Pair K(Pair p, Pair C) {
    return rotl(p, 1) ^ rotl(p, 3) ^ rotr(p, 2) ^ C;
}
```

- `rotl(p, n)`: rotate left by n bits
- `rotr(p, n)`: rotate right by n bits
- `C`: constitutional constant (0x1D)

## SID (Self-ID)

```c
#define CONSTITUTIONAL_C  0x1D

static inline Pair compute_sid(Pair nf) {
    return K(nf, CONSTITUTIONAL_C);
}
```

The SID is the kernel hash of the node fingerprint with the constitutional constant 0x1D.

## Properties

| Property | Value |
|----------|-------|
| Word size | 16 bits |
| Car/cdr | 8 bits each |
| K period | 8 (all inputs reach cycle) |
| Constitutional C | 0x1D (GS) |

## Source Files

- `esp32/main/tetra-kernel.h` — RISC-V implementation
- `polyform-env/include/polyform.h` — Self-contained types
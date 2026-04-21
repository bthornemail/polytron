# Witness — Replay Proof Generator

## Purpose

A witness pack is a cryptographic proof that:
- A computation was performed
- The result is correct
- The intent is context-invariant

This allows anyone to verify the computation without re-running it.

## Structure

```
┌─────────────────────┐
│ Witness Header     │
│ (magic, version)   │
├─────────────────────┤
│ Input Value        │
│ (the pair)         │
├─────────────────────┤
│ Kernel Trace       │
│ (the K steps)      │
├─────────────────────┤
│ Output SID         │
├─────────────────────┤
│ Holonomy           │
│ (cycle class)      │
├─────────────────────┤
│ Void-SID Proof     │
│ (if non-invariant) │
└─────────────────────┘
```

## Constants

```c
#define WITNESS_MAGIC     0x574954  /* "WIT" */
#define WITNESS_VERSION 1

/* Computation types */
#define COMP_K         0x01   /* Kernel K(p,C) */
#define COMP_SID       0x02   /* SID computation */
#define COMP_HASH      0x03   /* Hash computation */
#define COMP_VERIFY    0x04   /* Context verification */
```

## Header Structure

```c
typedef struct {
    uint32_t magic;
    uint8_t  version;
    uint8_t  comp_type;   /* COMP_* */
    uint8_t  steps;      /* Number of computation steps */
    Pair input;
    Pair output;
    Pair sid;
    Pair void_sid;     /* Void-SID if computation is non-invariant */
    uint8_t  holonomy;
    uint8_t  flags;
} WitnessHeader;
```

## Operations

```c
// Generate witness pack
WitnessPack* generate_witness(Pair input, CompType type);

// Verify witness
int verify_witness(const WitnessPack* pack);

// Replay computation from witness
Pair replay_witness(const WitnessPack* pack);
```

## Void-SID Proof

If a computation is not context-invariant (produces different results in different contexts), the Void-SID proof provides:
- The computed result
- The context that produces the Void result
- A merkle path proving the computation

## Source Files

- `tetra-witness.h` — Full implementation
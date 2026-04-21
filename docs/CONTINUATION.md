# Continuation — Appendix G FSM

## Appendix G Modes (Code 16K)

```c
typedef enum {
    MODE_A = 0,           /* Control chars (0-95) — Reset */
    MODE_B = 1,          /* Printable ASCII (32-127) — Propagate */
    MODE_C = 2,          /* Numeric double-density — Generate */
    MODE_C_FNC1 = 3,     /* FNC1 + numerics — AND gate */
    MODE_B_FNC1 = 4,     /* FNC1 alone — OR gate */
    MODE_C_SHIFT_B = 5,  /* Odd numerics (3+) — Lookahead */
    MODE_C_DOUBLE_SHIFT_B = 6  /* Non-numeric + even numerics — Double lookahead */
} AppendixG_Mode;
```

## 4-Channel Carry Lookahead

```c
typedef struct {
    uint8_t fs;   /* XOR gate — sum */
    uint8_t gs;   /* AND gate — carry generate */
    uint8_t rs;   /* OR gate — carry propagate */
    uint8_t us;   /* Shift register — lookahead logic */
    uint8_t carry_in;
    uint8_t carry_out;
} CarryLookahead4;
```

- **FS**: XOR — sum output
- **GS**: AND — carry generate
- **RS**: OR — carry propagate  
- **US**: shift register — lookahead

## Logic Gates (Appendix G Rules)

```c
// Rule 1a: FNC1 + 2+ numerics → AND gate (generate)
static inline uint8_t rule_1a(uint8_t a, uint8_t b) {
    return a & b;  /* AND — GS channel */
}

// Rule 1b: FNC1 alone → OR gate (propagate)
static inline uint8_t rule_1b(uint8_t a, uint8_t b) {
    return a | b;  /* OR — RS channel */
}

// Rule 1c: Even numerics → XOR (sum)
static inline uint8_t rule_1c(uint8_t a, uint8_t b) {
    return a ^ b;  /* XOR — FS channel */
}
```

## 4-Bit Carry Lookahead Adder

```c
static CarryLookahead4 compute_4bit_adder(
    uint8_t A[4], uint8_t B[4], uint8_t carry_in);
```

Implements fast carry propagation for Code 16K data channels.

## FSM State Machine

The Appendix G FSM maps Code 16K row data to continuation operations:
- Mode A: Reset state
- Mode B: Propagate (pass through)
- Mode C: Generate (produce output)
- Mode with FNC1: AND/OR gates

## Source Files

- `polyform-env/include/continuation.h` — FSM definitions
- `polyform-env/include/barcode-frame.h` — Uses 4-bit adder
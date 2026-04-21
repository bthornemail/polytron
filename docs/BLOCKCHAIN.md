# Block Chain — Constitutional Layers

## Layer Naming (ASCII Separators)

| Layer | ASCII | Name | Description |
|-------|-------|------|-------------|
| FS | 0x1C | File Separator | Hex×Decimal plane |
| GS | 0x1D | Group Separator | Hex×Binary plane [CONSTITUTIONAL_C] |
| RS | 0x1E | Record Separator | Decimal×Binary plane |
| US | 0x1F | Unit Separator | All three planes, s=1 |

## Chain Structure

```
[FS] <-- [GS] <-- [RS] <-- [US]
```

Each layer is a COBS frame with specific s-bit and plane. The combined SID verifies the entire chain.

## Block Layer Constants

```c
#define LAYER_FS  ((BlockLayer)0x1C)  /* File Separator — base layer */
#define LAYER_GS  ((BlockLayer)0x1D)  /* Group Separator — overlay 1 */
#define LAYER_RS  ((BlockLayer)0x1E)  /* Record Separator — overlay 2 */
#define LAYER_US  ((BlockLayer)0x1F)  /* Unit Separator — active layer */
```

## Block Header

```c
#define MAX_PAYLOAD 240

typedef struct {
    uint8_t layer;        /* FS, GS, RS, or US */
    uint8_t seq;          /* Sequence number */
    uint8_t s_bit;        /* Structural bit */
    uint16_t prev_sid;    /* Previous block SID */
    uint16_t nonce;       /* Proof of work */
    uint16_t payload_len;
    uint8_t payload[MAX_PAYLOAD];
} BlockHeader;
```

## Chain Operations

```c
// Create a new block
BlockHeader make_block(BlockLayer layer, const uint8_t* data, size_t len);

// Verify chain integrity
int verify_chain(const BlockHeader* blocks, int count);

// Compute chain SID
uint16_t chain_sid(const BlockHeader* blocks, int count);
```

## Gauge Transition Table

The block chain uses the gauge transition table for layer transitions:

```
┌──────┬─────────────────────────────────────────┐
│ FS    │ Hex×Decimal plane (base)              │
├──────┼─────────────────────────────────────────┤
│ GS    │ Hex×Binary plane (overlay 1)           │
├──────┼─────────────────────────────────────────┤
│ RS    │ Decimal×Binary plane (overlay 2)       │
├──────┼─────────────────────────────────────────┤
│ US    │ All three planes, s=1 (active)         │
└──────┴─────────────────────────────────────────┘
```

## QEMU Images

| Image | Layer | Purpose |
|-------|-------|---------|
| fs.qcow2 | FS | Base layer |
| gs.qcow2 | GS | Overlay 1 |
| rs.qcow2 | RS | Overlay 2 |
| us.qcow2 | US | Active layer |

## Source Files

- `tetra-block.h` — Block chain protocol
- `tetra-block-chain.sh` — QEMU setup script
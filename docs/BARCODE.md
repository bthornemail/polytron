# Barcode Frame — Polyform Encoding

## MaxiCode (15-bit Type Declaration)

```c
typedef struct {
    uint8_t basis : 4;        /* 0-11: square, triangle, hex, cube */
    uint8_t degree : 4;       /* 1-16: n-omino */
    uint8_t dimension : 2;    /* 0=1D, 1=2D, 2=2.5D, 3=3D */
    uint8_t group : 4;       /* Symmetry group */
    uint8_t reserved : 1;    /* Future */
    uint16_t checksum : 10;  /* Error check */
} MaxiCode;
```

## Aztec (40-bit Full State)

```c
typedef struct {
    uint64_t codepoint : 40;  /* 40-bit virtual address */
    uint8_t layers;            /* Aztec layers (1-32) */
    uint8_t mode_message;      /* Encoded layer count */
} AztecCode;
```

## BEEtag (25-bit Cell Packets)

```c
#define BEETAG_CELLS_PER_PACKET 5

typedef struct {
    uint8_t sequence;                     /* 0-31: packet order */
    uint8_t cells[BEETAG_CELLS_PER_PACKET];  /* 5 cells, 2-of-5 */
} BEEtagPacket;
```

## Code 16K (4-Channel Carry Lookahead)

```c
// Encode: two bytes → 4 channels
Code16K encode_code16k(uint8_t a, uint8_t b, uint8_t carry_in);

// Decode: 4 channels → two bytes + carry
void decode_code16k(Code16K code, uint8_t* sum, uint8_t* carry_out);
```

| Channel | Operation |
|---------|-----------|
| FS | XOR (sum) |
| GS | AND (generate) |
| RS | OR (propagate) |
| US | Lookahead |

## Unified Barcode Frame

```c
typedef struct {
    MaxiCode type_decl;       /* What is this? */
    BEEtagPacket* packets;  /* Cell packets */
    int packet_count;
    AztecCode full_state;  /* Complete state */
    Code16K channel_data;  /* 4-channel control */
} BarcodeFrame;
```

## Serialization

```c
// Polyform → BarcodeFrame
BarcodeFrame serialize_to_frame(const Polyform* poly);

// BarcodeFrame → Polyform
Polyform* instantiate_from_frame(const BarcodeFrame* frame);
```

## Source Files

- `polyform-env/include/barcode-frame.h` — All encodings
- `polyform-env/include/continuation.h` — Uses 4-bit adder
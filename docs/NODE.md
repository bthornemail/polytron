# Tetra-Node — UDP Server

## Overview

A single node that:
- Listens on UDP port
- Receives COBS frames
- Applies K(p,C)
- Computes SID
- Returns response

## Compile & Run

```bash
gcc -o tetra-node tetra-node.c -Wall
./tetra-node [port]  # Default: 31415
```

## Message Format

**Request:**
```
[length:1][COBS frame][checksum:2]
```

**Response:**
```
[type:1][SID:2][s_bit:1][holonomy:1]
```

## Protocol

1. Receive UDP packet
2. Decode COBS frame
3. Apply K(p, C) kernel
4. Compute SID
5. Classify holonomy
6. Return response

## Constants

```c
#define PORT 31415
#define MAX_PACKET 1024
#define MAX_FRAME 256

#define CONSTITUTIONAL_C  0x1D
#define COBS_DELIMITER   0x00
#define COBS_STUFF      0x01
```

## Holonomy Response

| Code | Meaning |
|------|---------|
| 0 | VOID (no valid cells) |
| 1 | SIMPLE (single cell) |
| 2 | CHAIN (linear) |
| 3 | RING (closed loop) |
| 4 | TREE (branching) |
| 5 | COMPLEX (multiple) |

## Source Files

- `tetra-node.c` — UDP server
- `tetra-client.c` — Test client
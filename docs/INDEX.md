# Tetragrammatron Documentation

## Overview

The Tetragrammatron is a constitutional computing framework where the 16-bit word (pair) is the only primitive data structure. Built from primitives up through polyforms, barcode frames, and PNG as the base object.

## Documentation Index

| Doc | Description |
|-----|-------------|
| [KERNEL.md](KERNEL.md) | Core primitives: pair, kernel K(p,C), SID |
| [POLYFORM.md](POLYFORM.md) | 1D/2D/2.5D/3D dimensional types |
| [CONTINUATION.md](CONTINUATION.md) | Appendix G FSM, 4-bit carry lookahead |
| [BARCODE.md](BARCODE.md) | Aztec/MaxiCode/BEEtag/Code16K encodings |
| [PNG.md](PNG.md) | PNG as constitutional base object |
| [NODE.md](NODE.md) | UDP server with COBS framing |
| [GOSSIP.md](GOSSIP.md) | Epidemic gossip protocol |
| [BLOCKCHAIN.md](BLOCKCHAIN.md) | Constitutional block chain |
| [NRR.md](NRR.md) | Non-repudiation receipts |
| [WITNESS.md](WITNESS.md) | Witness pack generator |
| [ESP32.md](ESP32.md) | ESP32 hardware implementation |
| [HOLEMAP.md](HOLEMAP.md) | Lattice visualization |
| [GPU.md](GPU.md) | Full GPU + IVSHMEM pipeline |
| [TCG.md](TCG.md) | TCG integration + cardinality/chirality |
| [CLOCK.md](CLOCK.md) | Clock tree visualization (Graphviz) |
| [GNOMON.md](GNOMON.md) | GNOMON 8-step cycle animation |

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     CONSTITUTION                             │
│                  (FS ← GS ← RS ← US)                      │
└─────────────────────────────────────────────────────────────┘
                              │
         ┌────────────────────┼────────────────────┐
         ▼                    ▼                    ▼
   ┌───────────┐       ┌───────────┐       ┌───────────┐
   │   Pair    │       │   PNG     │       │  Barcode   │
   │ (16-bit)  │       │ (image)   │       │ ( Aztec)  │
   └───────────┘       └───────────┘       └───────────┘
         │                                       │
         ▼                                       ▼
   ┌───────────┐                        ┌───────────┐
   │  Kernel   │                        │ Code 16K  │
   │ K(p,C)    │                        │ (FS/GS/  │
   │           │                        │  RS/US)   │
   └───────────┘                        └───────────┘
```

## Build & Test

```bash
# Polyform tests
cd /home/main/Documents/Tron/polyform-env
make test
./test

# Hole map visualization
cd /home/main/Documents/Tron
gcc -o holemap holemap.c -lm
./holemap > holemap.svg

# UDP node
gcc -o tetra-node tetra-node.c -Wall
./tetra-node
```
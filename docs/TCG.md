# TCG — Tiny Code Generator Integration

## Overview

Implements the constitutional kernel K(p,C) as native TCG opcodes for QEMU.

## Cardinality

| Set | Cardinality | Description |
|-----|-------------|-------------|
| Clock layers | 4 | FS, GS, RS, US |
| Dimensions | 4 | 1D, 2D, 2.5D, 3D |
| Polyform basis | 6 | Square, Triangle, Hex, Rhombus, Cube, Stick |
| GNOMON period | 8 | K(p,C) cycle length |
| Omicron cubes | 8 | Dalí cross (octacube) |

## Chirality

### Kernel K(p,C)

```c
K(p,C) = rotl(p,1) ^ rotl(p,3) ^ rotr(p,2) ^ C
```

| Operation | Chirality | Description |
|-----------|-----------|-------------|
| rotl(p,1) | Left | Rotate left 1 bit |
| rotl(p,3) | Left | Rotate left 3 bits |
| rotr(p,2) | Right | Rotate right 2 bits |
| ^ | XOR | Fold (sum mod 2) |

### Channel Operations

| Channel | Operation | Description |
|---------|-----------|-------------|
| FS | XOR | Sum output |
| GS | AND | Carry generate |
| RS | OR | Carry propagate |
| US | SHIFT | Lookahead |

## TCG Opcode Definition

```c
/*
 * TCG Operation: tetra_kernel_i32
 * 
 * Input:
 *   t0 = arg (i32)
 *   t1 = C (i32 constant)
 * Output:
 *   t0 = rotl(arg,1) ^ rotl(arg,3) ^ rotr(arg,2) ^ C
 */
```

## GNOMON Growth

```
seed → K(seed,C) → K(K(seed,C),C) → ... → cycle
```

The GNOMON period is 8 — all starting points reach a cycle within 8 steps.

## Omicron (Dalí Cross)

```
    □
  □ □ □
    □
  + corner (1,1,1)
```

8 voxels with octahedral symmetry.

## TCG Backend Assembly

### x86

```asm
mov eax, [arg]
mov ebx, eax
rol eax, 1
mov ecx, ebx
rol ebx, 3
mov edx, ebx
ror edx, 2
xor eax, ebx
xor eax, edx
xor eax, [C]
ret
```

### ARM (AArch64)

```asm
lsl x1, x0, #1
ror x1, x1, #63
lsl x2, x0, #3
ror x2, x2, #61
lsr x3, x0, #2
ror x3, x3, #62
eor x0, x1, x2
eor x0, x0, x3
eor x0, x0, x4
ret
```

## Files

| File | Description |
|------|-------------|
| `tetra-tcg.h` | TCG integration header |
| `tetra-clock.h` | Clock tree model |

## Stack

```
┌─────────────────────────────────────────┐
│  TCG Backend (x86, ARM, RISC-V)       │
├─────────────────────────────────────────┤
│  tetra_kernel_i32 / _i64              │
├─────────────────────────────────────────┤
│  K(p,C) = rotl⊕rotl⊕rotr⊕C           │
├─────────────────────────────────────────┤
│  Pair (16-bit) → Word (32-bit)        │
└─────────────────────────────────────────┘
```

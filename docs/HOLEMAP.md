# Hole Map — Lattice Visualization

## Overview

Renders topological singularities with their deterministic Void-SIDs. Each hole gets a unique 16-bit identifier computed from its lattice position.

## Compile & Run

```bash
gcc -o holemap holemap.c -lm
./holemap > holemap.svg
```

## Lattice Parameters

```c
#define LATTICE_W 16
#define LATTICE_H 16
#define CELL 48
```

## Planes

| Plane | Symbol | Description |
|-------|--------|-------------|
| U | Binary | 8-bit binary |
| D | Decimal | Decimal encoding |
| H | Hex | Hexadecimal |
| B | Mixed | Mixed encoding |

## Void Classification

The hole map classifies each position as:
- **VOID**: Invalid cell (holonomy = 0)
- **Valid**: Cell has valid state

## Hash Function

```c
static Pair hash7(Pair seed) {
    Pair h = seed;
    for (int i = 0; i < 7; i++) h = K(h, CONSTITUTIONAL_C);
    return h;
}
```

Applies K(p, C) 7 times to generate unique Void-SID.

## SVG Output

Generates an SVG with:
- Colored cells for valid states
- Empty cells for VOID states
- Grid overlay
- Labels for each cell's SID

## Source Files

- `holemap.c` — Visualizer
- `holemap.svg` — Generated output
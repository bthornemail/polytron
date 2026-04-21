# Polyform — Dimensional Types

## Dimension Kinds

```c
typedef enum {
    DIM_1D,      /* Polysticks — edges only */
    DIM_2D,      /* Polyominoes, polyiamonds, polyhexes */
    DIM_2_5D,    /* Extruded 2D — height from Aztec layers */
    DIM_3D       /* Polycubes */
} Dimension;
```

## Basis Kinds (Cell Shapes)

```c
typedef enum {
    BASIS_SQUARE = 0,       /* Polyomino */
    BASIS_TRIANGLE = 1,     /* Polyiamond */
    BASIS_HEXAGON = 2,     /* Polyhex */
    BASIS_RHOMBUS = 3,     /* Polyrhomb */
    BASIS_CUBE = 4,        /* Polycube (3D) */
    BASIS_STICK = 5        /* Polystick (1D) */
} Basis;
```

## 1D — Polystick

```c
typedef struct {
    uint8_t length;           /* Number of edges */
    uint8_t angles[32];       /* 0=straight, 1=90° turn, 2=180° */
    Pair start_sid;           /* SID of origin */
    Pair end_sid;             /* SID of terminus */
} PolyStick;
```

A 1-dimensional polyform: a chain of edges.

## 2D — Polyomino

```c
typedef struct {
    uint8_t degree;          /* n for n-omino (1-16) */
    Basis basis;            /* Cell shape */
    uint8_t count;          /* Actual cells */
    Cell2D* cells;         /* Cell coordinates */
    Pair polyform_sid;      /* SID */
} Polyform2D;
```

A 2-dimensional polyform: cells on a grid.

## 2.5D — Extruded

```c
typedef struct {
    Polyform2D* base;        /* 2D base */
    uint8_t height;          /* Extrusion depth */
    Pair extruded_sid;        /* SID */
} Polyform2_5D;
```

A 2D polyform with extrusion depth (z-axis).

## 3D — Polycube

```c
typedef struct {
    uint8_t degree;          /* n for n-cube (1-16) */
    uint8_t count;          /* Actual voxels */
    Voxel3D* voxels;       /* Voxel coordinates */
    Pair polycube_sid;     /* SID */
} Polycube3D;
```

A 3-dimensional polyform: voxels in space.

## Unified Polyform

```c
typedef struct {
    Dimension dim;           /* Which dimension */
    PolyStick stick;
    Polyform2D poly2d;
    Polyform2_5D poly2_5d;
    Polycube3D poly3d;
    Pair unified_sid;
} Polyform;
```

## Channel Extraction

```c
// FS: XOR of all coords (sum)
// GS: AND of all coords (generate)
// RS: OR of all coords (propagate)
// US: height (extrusion)
```

## Source Files

- `polyform-env/include/polyform.h` — Complete type definitions
- `polyform-env/test.c` — Tests
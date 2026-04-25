# Axiom 6: The Pure Dot Notation System

## The Fundamental Principle

> Don't describe numbers in numbers. Don't describe characters in characters.

This is the **inverse tautology** - the system must be expressed entirely in its own terms, with no external reference.

---

## The Alphabet: 64 Codepoints

No letters. No variables. Only:

```
NUL SOH STX ETX EOT ENQ ACK BEL BS  HT  LF  VT  FF  CR  SO  SI
DLE DC1 DC2 DC3 DC4 NAK SYN ETB CAN EM  SUB ESC FS  GS  RS  US
SP  !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /
0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?
```

---

## The Delta Law in Pure Dot Notation

No x. No rotl. No XOR. No C. Only the 64 codepoints and cons.

The state is a list of the four channels:

```
(FS . (GS . (RS . (US . nil))))
```

### rotl(_,1) — rotate left by 1 position:

```
(FS . (GS . (RS . (US . nil))))
        ↓
(GS . (RS . (US . (FS . nil))))
```

### rotl(_,3) — rotate left by 3 positions:

```
(FS . (GS . (RS . (US . nil))))
        ↓
(US . (FS . (GS . (RS . nil))))
```

### rotr(_,2) — rotate right by 2 positions:

```
(FS . (GS . (RS . (US . nil))))
        ↓
(RS . (US . (FS . (GS . nil))))
```

### XOR — symmetric difference of channel lists

### C — the constant channel (always GS)

---

## The Complete Rewrite Rule

```
(FS . GS . RS . US) 
    → 
(XOR (rotl 1) (XOR (rotl 3) (XOR (rotr 2) GS)))
```

In pure dot notation with 64 codepoints:

```
( ( ( (FS . (GS . (RS . (US . nil)))) .
      (GS . (RS . (US . (FS . nil)))) .
      ( (GS . (RS . (US . (FS . nil)))) .
        (RS . (US . (FS . (GS . nil)))) ) ) .
    ( ( (RS . (US . (FS . (GS . nil)))) .
        (US . (FS . (GS . (RS . nil)))) ) .
      ( ( (US . (FS . (GS . (RS . nil)))) .
          (FS . (GS . (RS . (US . nil)))) ) .
        (GS . nil) ) ) )
```

---

## The First Test: Printing in Non-Printing

To print character "A":
- A is described by SOH
- Emit SOH
- The renderer (outside system) maps SOH → "A"

The system never contains "A". It only contains SOH. The printing character is described by the non-printing character.

This is the inverse tautology:
- "A" is true if and only if SOH is true
- But SOH is not "A"
- SOH is SOH

The mapping is outside the system.

---

## The Fixed Points: Rationalized Planck Units

The fixed points of the delta law satisfy:

```
state = delta(state)
```

These are the gauge-invariant states - the rationalized Planck units:

```
c = 1  (via FS channel)
G = 1  (via GS channel)  
ħ = 1  (via RS channel)
kB = 1 (via US channel)
```

In pure dot notation, the fixed point is:

```
(FS . (GS . (RS . (US . nil))))
```

---

## The Omicron: NUL as Axis

The NUL character (0x00) is not zero. It is the **axis** - the origin from which all positions are measured.

In the dotted pair `(NUL . x)`:
- NUL is the CAR of the entire structure
- It is not a value
- It is the position from which all other positions are measured

This is bijective representation - there is no zero digit. NUL is the axis.

---

## The 4-Channel Modem Multiplexer

```
NUL ──────────────────────────────── ESC
 │                                     │
 └─ FS (0x1C) ─ GS (0x1D) ─ RS (0x1E) ─ US (0x1F) ─┘
                          │
                    8-bit data path
                          │
                    3-bit ECC
```

### Channels:
- **FS**: File Separator — advance to next file
- **GS**: Group Separator — advance to next group  
- **RS**: Record Separator — advance to next record
- **US**: Unit Separator — advance to next unit

### Escapes:
- **NUL**: The axis (origin of coordinate system)
- **ESC**: Embedding (allows lists within lists)

---

## The 74LS192 as Meta-Circular Interpreter

The 74LS192 is a synchronous 4-bit up/down decade counter. Its pinout corresponds to the 4-channel carry lookahead:

| Pin | Signal | Channel |
|-----|--------|---------|
| 2   | P0     | FS      |
| 3   | P1     | GS      |
| 4   | P2     | RS      |
| 5   | P3     | US      |
| 11  | C3     | **Omicron** (missing - determines up/down) |

The missing C3 at pin 11 is the **Omicron** - the pre-header that replaces "1" and "0" in "10".

---

## Fractal Differential

The system captures the fractal differential between:

| Dimension | Polytope | Elements |
|-----------|----------|----------|
| 0D | Point | Vertex |
| 1D | Edge | Line |
| 2D | Face | Polygon |
| 3D | Cell | Polyhedron |

| Solid Type | Examples | Elements |
|------------|----------|----------|
| Platonic | Tetrahedron, Cube, Octahedron, Dodecahedron, Icosahedron | Regular |
| Archimedean | Truncated Tetrahedron, Cuboctahedron | Semi-regular |
| Catalan | Triakistetrahedron, Rhombic Dodecahedron | Dual semi-regular |
| Snub | Snub Cube, Snub Dodecahedron | Chiral |

| 4D Simplex | Vertices | Edges | Faces | Cells |
|------------|----------|-------|-------|-------|
| 5-cell | 5 | 10 | 10 | 5 |

| 4D Cross-polytope | Vertices | Edges | Faces | Cells |
|-------------------|----------|-------|-------|-------|
| 16-cell | 16 | 32 | 24 | 8 |

The resolution emerges through **cardinality** (how many elements) and **chirality** (handedness - CW or CCW).

---

## The Complete System in One Expression

```
(NUL . (FS . (GS . (RS . (US . nil)))))
```

With the rewrite rule:

```
(FS . GS . RS . US) 
    → 
(GS . RS . US . FS) XOR (US . FS . GS . RS) XOR (RS . US . FS . GS) XOR GS
```

That's it. That's the entire system.

No numbers. No letters. Only the 64 codepoints and cons.

The printing characters are described by the non-printing characters. The Omicron is the axis. The delta law is the rewrite. The Planck units are the fixed points.

---

## The Discovery

> **Don't describe numbers in numbers. Don't describe characters in characters.**

This is the first test - the inverse tautology available to us.

The fundamental constants of physics are the fixed points of the delta law on the 4-channel ASCII control character cons structure.

And this discovery is expressible entirely without numbers and without letters - only the 64 codepoints and the cons.

The Omicron is the axis NUL.

The delta law is the rewrite rule.

The barcodes are the projections.

The QEMU trace is the execution.

All of it reduces to:

```
(NUL . (FS . (GS . (RS . (US . nil)))))
```

---

# The Axiom: NULL != 10

## A Complete System Built from Nothing

---

## I. The Single Axiom

```
NULL != 10
```

This means: **NULL is not equal to the string "10"** (the base-2 representation of two).

We don't use positional notation. We don't use digits. We use symbols earned through permutation count.

---

## II. The Derivation

### Step 0: NULL — The Axiom

NULL is the axis. It is not zero. It is not one. It is not the string "10". It is the position from which all positions are measured.

### Step 1: The First Pair

```
(NULL . NULL)
```

This is the first structure. Two elements. Permutation count: 2.

### Step 2: Earning SOH

At 2! permutations, we earn the right to use a new symbol: SOH.

```
NULL SOH
```

### Step 3: Earning More Symbols

| Permutations | Symbol | Codepoint |
|:------------|:-------|:---------|
| 2! = 2 | SOH | 0x01 |
| 3! = 6 | STX | 0x02 |
| 4! = 24 | ETX | 0x03 |
| 5! = 120 | EOT | 0x04 |
| 6! = 720 | ENQ | 0x05 |
| 7! = 5040 | ACK | 0x06 |
| 8! = 40320 | BEL | 0x07 |

### Step 4: The Delta Law

We define one operation on 8-element sequences:

```
Δ(x) = rotl(x,1) ⊕ rotl(x,3) ⊕ rotr(x,2) ⊕ C
```

| Component | Meaning |
|:----------|:--------|
| rotl(x,n) | Rotate left by n (no bits lost) |
| ⊕ | XOR (reversible) |
| rotr(x,n) | Rotate right by n |
| C | Constant = STX (breaks zero fixed point) |

### Step 5: The Prime Emerges

The delta law has period 8.

The smallest prime with decimal period 8 is 73.

Nobody chose 73. The period is 8. The prime follows.

The digits of 1/73: [0,1,3,6,9,8,6,3]
Sum: 36

### Step 6: The Break Points

| Expression | Meaning | What Breaks |
|:-----------|:--------|:-----------|
| NULL != 1 | NULL ≠ unary "1" | Unary representation |
| NULL != 10 | NULL ≠ binary "10" | Binary representation |
| NULL != 100 | NULL ≠ "100" | Nibble (4-bit) |
| NULL != 1000 | NULL ≠ "1000" | Byte (8-bit) |
| NULL != 10^10 | NULL ≠ "10^10" | Word (16-bit) |
| NULL != 10^10^10 | NULL ≠ "10^10^10" | Double word (32-bit) |
| NULL != 10^100 | NULL ≠ "10^100" | 64-bit |
| NULL != 10^100^1000 | NULL ≠ "10^100^1000" | The Monster |

Each break creates a new symbol—the Aegean numerals.

### Step 7: The Aegean Numerals

| Value | Symbol | Codepoint |
|:-----|:-------|:---------|
| 1 | 𐄇 | U+10107 |
| 2 | 𐄈 | U+10108 |
| 3 | 𐄉 | U+10109 |
| 4 | 𐄊 | U+1010A |
| 5 | 𐄋 | U+1010B |
| 6 | 𐄌 | U+1010C |
| 7 | 𐄍 | U+1010D |
| 8 | 𐄎 | U+1010E |
| 9 | 𐄏 | U+1010F |

| Omicron Markers | Symbol | Codepoint |
|:---------------|:-------|:---------|
| Word separator | 𐄀 | U+10100 |
| Dot | 𐄁 | U+10101 |
| Double dot | 𐄂 | U+10102 |

---

## III. The Components

### A. The Transformer (One Instruction)

```lisp
(λ (x) (rotl x 1) ⊕ (rotl x 3) ⊕ (rotr x 2) ⊕ STX)
```

- Input: 8-element state
- Output: next 8-element state
- Only knows: bits, rotations, XOR

### B. The Stream

```lisp
(FS (𐄇 𐄈 𐄉) GS (d o g) RS 𐄀 US (𐄇 𐄈) ETX)
```

- FS: Start of synset
- 𐄇𐄈𐄉: Synset ID (123 in base-9)
- GS: Start of lemma
- d o g: Lemma
- RS: Part of speech
- 𐄀: Noun marker
- US: Relation
- 𐄇𐄈: Hypernym ID
- ETX: End of synset

Every token announces its own type. No variables defined.

### C. The Map

```lisp
((𐄀 . point)
 (𐄁 . line)
 (𐄂 . triangle)
 (𐄇 . tetrahedron)
 (𐄈 . 5-cell)
 (𐄉 . 8-cell)
 (𐄊 . 16-cell)
 (𐄋 . 24-cell)
 (𐄌 . 120-cell)
 (𐄍 . 600-cell)
 (𐄎 . hopf-fiber)
 (𐄏 . s7)
 (𐄐 . s15)
 ...)
```

Pointer → Geometry. Arbitrary assignment by us.

### D. The Renderer

```lisp
(defun render (symbol state)
  (case symbol
    (𐄀 (draw-point state))
    (𐄁 (draw-line state))
    (𐄂 (draw-triangle state))
    (𐄇 (draw-tetrahedron state))
    ...))
```

Takes symbol + state → draws geometry.

---

## IV. The Complete System

```
NULL                                        ; Axiom (0! = 1)
     ↓
(NULL . NULL)                              ; First pair
     ↓
SOH                                        ; Earned at 2!
     ↓
STX ETX EOT ENQ ACK BEL                    ; Earned at 3!-8!
     ↓
Δ(x) = rotl(x,1) ⊕ rotl(x,3) ⊕ rotr(x,2) ⊕ STX  ; Delta law
     ↓
Period = 8 → Prime = 73 → Block = [0,1,3,6,9,8,3]  ; Emerges
     ↓
Aegean numerals (𐄇-𐄏)                      ; Break symbols
     ↓
Omicron markers (𐄀-𐄂)                      ; Frame markers
     ↓
Geometry map                                ; Assigned by us
     ↓
Renderer                                 ; Interprets state + symbol
```

---

## V. The Point

**We don't define variables. We derive them.**

**We don't define geometries. We assign them.**

**We don't define the transformer. It's the delta law.**

**We don't define the stream. It's just symbols.**

NULL is the axiom. The dot is the operation. The break points create symbols. The map assigns geometries. The renderer draws them.

That's it. That's everything.

---

## VI. The Breakthrough

The system works because:

1. The delts law has period 8
2. The prime 73 emerges from the period
3. The break points create new symbols
4. The map assigns meaning
5. The renderer produces output

No variables. No definitions. Just:

```
(λ (x) rotl(x,1) ⊕ rotl(x,3) ⊕ rotr(x,2) ⊕ C
```

Applied to itself forever. The state cycles. The stream flows. The map assigns. The renderer draws.

The matrix breaks into tangent encoding. The universal codeword is NULL. The Negafibonacci is SOH|DEL. The combinatorics is the complete 64-codepoint space.

That's the Polyform Logic Engine.
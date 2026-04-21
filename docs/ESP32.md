# ESP32 — Hardware Implementation

## Philosophy

No Python. No external dependencies. Pure C.

This is the foundation:
```
pair → kernel → COBS → SID → witness → log → chain → mesh
```

## Layer 0: Pair (16-bit Primitive)

```c
typedef uint16_t Pair;

#define cons(a,d) ((((a) & 0xFF) << 8) | ((d) & 0xFF))
#define car(p)    (((p) >> 8) & 0xFF)
#define cdr(p)    ((p) & 0xFF)
```

- Bits 15-8: car (left projection)
- Bits 7-0: cdr (right projection)

## Layer 1: Kernel K(p,C)

```c
static inline Pair K(Pair p, Pair C) {
    return rotl(p, 1) ^ rotl(p, 3) ^ rotr(p, 2) ^ C;
}
```

On ESP32 (Xtensa), this compiles to ~6 instructions.

## Layer 2: SID and Context Invariance

```c
static inline Pair compute_sid(Pair nf) {
    return K(nf, CONSTITUTIONAL_C);
}

static inline int is_context_invariant(Pair value) {
    Pair v0 = cons(car(value), cdr(value) | 0x80);
    Pair v1 = cons(car(value), cdr(value) & 0x7F);
    return compute_sid(v0) == compute_sid(v1);
}
```

## Constitutional Constants

```c
#define CONSTITUTIONAL_C  0x1D  /* GS - Group Separator */
#define COBS_DELIMITER     0x00
#define COBS_ESCAPE       0x01
#define LAYER_FS          0x1C  /* File Separator */
#define LAYER_GS          0x1D  /* Group Separator */
#define LAYER_RS          0x1E  /* Record Separator */
#define LAYER_US          0x1F  /* Unit Separator */
```

## ESP-IDF Project Structure

```
esp32/
├── CMakeLists.txt
├── sdkconfig.defaults
├── README.md
└── main/
    ├── CMakeLists.txt
    ├── tetra-kernel.h   /* Core kernel */
    └── tetra-main.c     /* ESP32 app */
```

## Build

```bash
cd /home/main/Documents/Tron/esp32
idf.py build
```

## Flash

```bash
idf.py -p /dev/ttyUSB0 flash
```

## Source Files

- `esp32/main/tetra-kernel.h` — Core kernel
- `esp32/main/tetra-main.c` — ESP32 application
- `esp32/README.md` — ESP-IDF setup
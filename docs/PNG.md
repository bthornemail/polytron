# PNG — Constitutional Base Object

## PNG Signature

```c
static const png_byte PNG_SIG[8] = {137, 80, 78, 71, 13, 10, 26, 10};
```

Standard PNG signature (137 80 78 71 0D 0A 1A 0A).

## Constitutional Constant 0x1D

The constitutional constant `0x1D` (GS - Group Separator) appears in the PLTE palette:

```c
#define PNG_SIG_GS_OFFSET 4
#define PNG_SIG_GS_VALUE 0x1DU
```

Actually appears in palette at index 1: `{0x1D, 0x1D, 0x1D}` (constitutional gray).

## Constitutional Palette (16 colors)

```c
static const png_color constitutional_palette[16] = {
    {0x00, 0x00, 0x00},  /* 0: FS - black */
    {0x1D, 0x1D, 0x1D},  /* 1: GS - constitutional gray */
    {0xFF, 0x00, 0x00},  /* 2: RS - red */
    {0x00, 0xFF, 0x00},  /* 3: US - green */
    {0x00, 0x00, 0xFF},  /* 4: p - blue */
    {0xFF, 0xFF, 0x00},  /* 5: q - yellow */
    {0xFF, 0x00, 0xFF},  /* 6: r - magenta */
    {0x00, 0xFF, 0xFF},  /* 7: s - cyan */
    {0x80, 0x80, 0x80},  /* 8: gray */
    {0xC0, 0x00, 0x00},  /* 9: dark red */
    {0x00, 0xC0, 0x00},  /* 10: dark green */
    {0x00, 0x00, 0xC0},  /* 11: dark blue */
    {0x80, 0x00, 0x00},  /* 12: maroon */
    {0x00, 0x80, 0x00},  /* 13: dark green */
    {0x00, 0x00, 0x80},  /* 14: navy */
    {0x80, 0x80, 0x00}   /* 15: olive */
};

static const png_byte constitutional_alpha[16] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};
```

## Write PNG

```c
static int polyform_write_png(const Polyform2D* poly, const char* filename);
```

- IHDR: 8x8, 8-bit paletted, Adam7 interlace
- PLTE: constitutional palette
- tRNS: alpha channel
- tEXt: SID, degree, dimension metadata
- IDAT: image data

## Read PNG

```c
static Polyform2D* polyform_read_png(const char* filename);
```

Verifies:
1. Valid PNG signature
2. Constitutional palette contains 0x1D1D1D
3. Extracts SID from tEXt

## HEX Conversion

```c
static void sid_to_hex(Pair sid, char* out);
static Pair hex_to_sid(const char* hex);
```

## Dalí Cross (Octacube)

```c
static Polycube3D make_dali_cross(void);
```

8 voxels arranged as a cross — the GNOMON fixed point in 3D.

```text
    □
  □ □ □
    □
  + 5 more for 8 total
```

## Dalí PNG Layers

```c
static int dali_cross_write_png(const Polycube3D* cross, const char* basename);
```

Writes `basename_z0.png` through `basename_z3.png`.

## Source Files

- `polyform-env/include/tetra-png.h` — PNG functions
- `polyform-env/test.c` — Tests
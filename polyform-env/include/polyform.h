/*
 * POLYFORM.H — Constitutional Polyform Types (Self-Contained)
 * 
 * 1D: Polysticks (edges)
 * 2D: Polyominoes, Polyiamonds, Polyhexes
 * 2.5D: Extruded polyforms (height from Aztec layers)
 * 3D: Polycubes
 */

#ifndef POLYFORM_H
#define POLYFORM_H

#include <stdint.h>
#include <stdlib.h>

/* -------------------------------------------------------------------------- */
/* KERNEL TYPES (from tetra-kernel.h)                                      */
/* -------------------------------------------------------------------------- */

typedef uint16_t Pair;

#define cons(a,d) ((((a) & 0xFF) << 8) | ((d) & 0xFF))
#define car(p)    (((p) >> 8) & 0xFF)
#define cdr(p)    ((p) & 0xFF)

#define CONSTITUTIONAL_C  0x1D

static inline Pair rotl(Pair x, int n) {
    n &= 15;
    return (Pair)((x << n) | (x >> (16 - n)));
}

static inline Pair rotr(Pair x, int n) {
    n &= 15;
    return (Pair)((x >> n) | (x << (16 - n)));
}

static inline Pair K(Pair p, Pair C) {
    return rotl(p, 1) ^ rotl(p, 3) ^ rotr(p, 2) ^ C;
}

static inline Pair compute_sid(Pair nf) {
    return K(nf, CONSTITUTIONAL_C);
}

/* -------------------------------------------------------------------------- */
/* DIMENSION KINDS                                                            */
/* -------------------------------------------------------------------------- */

typedef enum {
    DIM_1D,      /* Polysticks — edges only */
    DIM_2D,      /* Polyominoes, polyiamonds, polyhexes */
    DIM_2_5D,    /* Extruded 2D — height from Aztec layers */
    DIM_3D       /* Polycubes */
} Dimension;

/* -------------------------------------------------------------------------- */
/* BASIS KINDS (Cell Shapes)                                                  */
/* -------------------------------------------------------------------------- */

typedef enum {
    BASIS_SQUARE = 0,       /* Polyomino */
    BASIS_TRIANGLE = 1,     /* Polyiamond */
    BASIS_HEXAGON = 2,     /* Polyhex */
    BASIS_RHOMBUS = 3,     /* Polyrhomb */
    BASIS_CUBE = 4,        /* Polycube (3D) */
    BASIS_STICK = 5        /* Polystick (1D) */
} Basis;

/* -------------------------------------------------------------------------- */
/* 1D POLYSTICK                                                               */
/* -------------------------------------------------------------------------- */

typedef struct {
    uint8_t length;           /* Number of edges */
    uint8_t angles[32];       /* 0=straight, 1=90° turn, 2=180° */
    Pair start_sid;           /* SID of origin */
    Pair end_sid;             /* SID of terminus */
} PolyStick;

/* -------------------------------------------------------------------------- */
/* 2D CELL                                                                   */
/* -------------------------------------------------------------------------- */

typedef struct {
    int16_t x, y;             /* Grid coordinates */
    uint8_t orientation;      /* 0-3 for squares, 0-5 for hex, etc. */
    Basis basis;              /* Cell shape */
    Pair cell_sid;            /* SID of this cell */
} Cell2D;

typedef struct {
    Cell2D* cells;
    uint16_t count;
    uint16_t degree;          /* n-omino */
    Basis basis;
    Pair polyform_sid;        /* Combined SID of all cells */
} Polyform2D;

/* -------------------------------------------------------------------------- */
/* 2.5D EXTRUDED POLYFORM                                                     */
/* -------------------------------------------------------------------------- */

typedef struct {
    Polyform2D* base;         /* 2D footprint */
    uint8_t height;           /* Extrusion height (Aztec layers) */
    uint8_t layer_sids[32];   /* SID per layer */
    Pair extruded_sid;       /* Combined 2.5D SID */
} Polyform2_5D;

/* -------------------------------------------------------------------------- */
/* 3D POLYCUBE                                                                */
/* -------------------------------------------------------------------------- */

typedef struct {
    int16_t x, y, z;
    uint8_t orientation;    /* 0-23 (24 orientations in cubic lattice) */
    Pair voxel_sid;
} Voxel3D;

typedef struct {
    Voxel3D* voxels;
    uint16_t count;
    uint16_t degree;          /* n-cube */
    Pair polycube_sid;
} Polycube3D;

/* -------------------------------------------------------------------------- */
/* UNIFIED POLYFORM                                                           */
/* -------------------------------------------------------------------------- */

typedef struct {
    Dimension dim;
    union {
        PolyStick stick;
        Polyform2D poly2d;
        Polyform2_5D poly2_5d;
        Polycube3D poly3d;
    };
    Pair unified_sid;         /* Dimension-agnostic SID */
} Polyform;

/* -------------------------------------------------------------------------- */
/* POLYFORM CONTINUATION                                                      */
/* -------------------------------------------------------------------------- */

typedef struct PolyformCont {
    Polyform* value;                    /* Current polyform */
    struct PolyformCont* next;        /* Forward continuation */
    struct PolyformCont* prev;        /* Backward (for circular mode) */
    uint8_t control_mode;           /* 0=asymmetric, 1=circular */
    Pair continuation_sid;          /* SID of the continuation chain */
} PolyformCont;

/* -------------------------------------------------------------------------- */
/* CONSTRUCTORS                                                               */
/* -------------------------------------------------------------------------- */

/* 1D: Create a polystick from a path */
static PolyStick make_polystick(const uint8_t* angles, uint8_t length, Pair origin) {
    PolyStick stick;
    stick.length = length;
    stick.start_sid = origin;
    
    for (int i = 0; i < length && i < 32; i++) {
        stick.angles[i] = angles[i];
    }
    
    /* Compute end SID by folding kernel over angles */
    Pair current = origin;
    for (int i = 0; i < length; i++) {
        current = K(current, cons(angles[i], 0));
    }
    stick.end_sid = current;
    
    return stick;
}

/* 2D: Create a polyomino by gnomon growth */
static Polyform2D make_polyomino(uint8_t degree, Pair seed) {
    Polyform2D poly;
    poly.cells = NULL;
    poly.count = 0;
    poly.degree = degree;
    poly.basis = BASIS_SQUARE;
    poly.polyform_sid = seed;
    
    if (degree == 0) return poly;
    
    /* Add first cell (monomino) */
    Cell2D first = { .x = 0, .y = 0, .orientation = 0, .basis = BASIS_SQUARE, .cell_sid = seed };
    poly.cells = (Cell2D*)malloc(sizeof(Cell2D));
    poly.cells[0] = first;
    poly.count = 1;
    poly.polyform_sid = K(seed, CONSTITUTIONAL_C);
    
    /* Grow to desired degree */
    for (int i = 1; i < degree; i++) {
        /* Find lowest set bit (the growth point) */
        Pair lowest = poly.polyform_sid & -poly.polyform_sid;
        
        /* Add new cell */
        Cell2D new_cell;
        new_cell.x = poly.count % 4;
        new_cell.y = poly.count / 4;
        new_cell.orientation = 0;
        new_cell.basis = BASIS_SQUARE;
        new_cell.cell_sid = K(poly.polyform_sid, lowest);
        
        poly.cells = (Cell2D*)realloc(poly.cells, (poly.count + 1) * sizeof(Cell2D));
        poly.cells[poly.count++] = new_cell;
        poly.polyform_sid = K(poly.polyform_sid, new_cell.cell_sid);
    }
    
    return poly;
}

/* 2.5D: Extrude a 2D polyform */
static Polyform2_5D extrude_polyform(const Polyform2D* base, uint8_t layers) {
    Polyform2_5D extruded;
    extruded.base = (Polyform2D*)base;
    extruded.height = layers;
    extruded.extruded_sid = base->polyform_sid;
    
    /* Each layer adds to the SID */
    for (int i = 0; i < layers && i < 32; i++) {
        extruded.layer_sids[i] = K(base->polyform_sid, cons(i, 0));
        extruded.extruded_sid = K(extruded.extruded_sid, extruded.layer_sids[i]);
    }
    
    return extruded;
}

/* 3D: Create a polycube by 3D gnomon */
static Polycube3D make_polycube(uint8_t degree, Pair seed) {
    Polycube3D poly;
    poly.voxels = NULL;
    poly.count = 0;
    poly.degree = degree;
    poly.polycube_sid = seed;
    
    if (degree == 0) return poly;
    
    /* First voxel */
    Voxel3D first = { .x = 0, .y = 0, .z = 0, .orientation = 0, .voxel_sid = seed };
    poly.voxels = (Voxel3D*)malloc(sizeof(Voxel3D));
    poly.voxels[0] = first;
    poly.count = 1;
    poly.polycube_sid = K(seed, CONSTITUTIONAL_C);
    
    for (int i = 1; i < degree; i++) {
        /* 3D growth */
        Pair lowest = poly.polycube_sid & -poly.polycube_sid;
        
        Voxel3D new_voxel;
        new_voxel.x = poly.count % 4;
        new_voxel.y = (poly.count / 4) % 4;
        new_voxel.z = poly.count / 16;
        new_voxel.orientation = 0;
        new_voxel.voxel_sid = K(poly.polycube_sid, lowest);
        
        poly.voxels = (Voxel3D*)realloc(poly.voxels, (poly.count + 1) * sizeof(Voxel3D));
        poly.voxels[poly.count++] = new_voxel;
        poly.polycube_sid = K(poly.polycube_sid, new_voxel.voxel_sid);
    }
    
    return poly;
}

/* Continuation: Chain two polyforms */
static PolyformCont* chain_polyforms(Polyform* first, Polyform* second, uint8_t mode) {
    PolyformCont* cont = (PolyformCont*)malloc(sizeof(PolyformCont));
    cont->value = first;
    cont->next = (PolyformCont*)malloc(sizeof(PolyformCont));
    cont->next->value = second;
    cont->next->next = NULL;
    cont->next->prev = cont;
    cont->prev = NULL;
    cont->control_mode = mode;
    
    /* Compute continuation SID */
    Pair sid1 = first->unified_sid;
    Pair sid2 = second->unified_sid;
    cont->continuation_sid = K(sid1, sid2);
    
    return cont;
}

/* -------------------------------------------------------------------------- */
/* SID COMPUTATION                                                            */
/* -------------------------------------------------------------------------- */

/* Compute SID for a polystick */
static Pair sid_polystick(const PolyStick* stick) {
    return stick->end_sid;
}

/* Compute SID for a 2D polyform */
static Pair sid_polyform2d(const Polyform2D* poly) {
    return poly->polyform_sid;
}

/* Compute SID for a 2.5D extruded polyform */
static Pair sid_polyform2_5d(const Polyform2_5D* poly) {
    return poly->extruded_sid;
}

/* Compute SID for a 3D polycube */
static Pair sid_polycube3d(const Polycube3D* poly) {
    return poly->polycube_sid;
}

/* Compute unified SID for any polyform */
static Pair sid_polyform(const Polyform* poly) {
    switch (poly->dim) {
        case DIM_1D:   return sid_polystick(&poly->stick);
        case DIM_2D:   return sid_polyform2d(&poly->poly2d);
        case DIM_2_5D: return sid_polyform2_5d(&poly->poly2_5d);
        case DIM_3D:   return sid_polycube3d(&poly->poly3d);
        default:       return 0;
    }
}

/* Compute continuation SID (fold K over chain) */
static Pair sid_continuation(const PolyformCont* cont) {
    Pair combined = 0;
    for (const PolyformCont* c = cont; c; c = c->next) {
        combined = K(combined, sid_polyform(c->value));
    }
    return combined;
}

/* -------------------------------------------------------------------------- */
/* CHANNEL EXTRACTION                                                         */
/* -------------------------------------------------------------------------- */

/* Extract FS channel (XOR) from polyform cells */
static uint8_t extract_fs_channel(const Polyform2D* poly) {
    uint8_t fs = 0;
    for (int i = 0; i < poly->count; i++) {
        fs ^= (uint8_t)((uint8_t)poly->cells[i].x ^ (uint8_t)poly->cells[i].y);
    }
    return fs;
}

/* Extract GS channel (AND) from polyform cells */
static uint8_t extract_gs_channel(const Polyform2D* poly) {
    uint8_t gs = 0xFF;
    for (int i = 0; i < poly->count; i++) {
        gs &= (uint8_t)((uint8_t)poly->cells[i].x & (uint8_t)poly->cells[i].y);
    }
    return gs;
}

/* Extract RS channel (OR) from polyform cells */
static uint8_t extract_rs_channel(const Polyform2D* poly) {
    uint8_t rs = 0;
    for (int i = 0; i < poly->count; i++) {
        rs |= (uint8_t)((uint8_t)poly->cells[i].x | (uint8_t)poly->cells[i].y);
    }
    return rs;
}

/* Extract US channel (Shift) from polyform layers */
static uint8_t extract_us_channel(const Polyform2_5D* poly) {
    return poly->height;
}

/* -------------------------------------------------------------------------- */
/* CLEANUP                                                                    */
/* -------------------------------------------------------------------------- */

/* Free polyform2D */
static void free_polyform2d(Polyform2D* poly) {
    if (poly->cells) free(poly->cells);
    poly->cells = NULL;
    poly->count = 0;
}

/* Free polycube3D */
static void free_polycube3d(Polycube3D* poly) {
    if (poly->voxels) free(poly->voxels);
    poly->voxels = NULL;
    poly->count = 0;
}

/* Free continuation */
static void free_continuation(PolyformCont* cont) {
    for (PolyformCont* c = cont; c; ) {
        PolyformCont* next = c->next;
        free(c);
        c = next;
    }
}

#endif /* POLYFORM_H */
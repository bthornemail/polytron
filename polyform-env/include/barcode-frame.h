/*
 * BARCODE-FRAME.H — Barcode Instantiation for Polyforms
 * 
 * Aztec: Full state (40-bit codepoint)
 * MaxiCode: Type declaration (15-bit identity + 10-bit check)
 * BEEtag: Cell packets (25-bit, 5 cells per packet)
 * Code 16K: Stacked rows (FS/GS/RS/US channels)
 */

#ifndef BARCODE_FRAME_H
#define BARCODE_FRAME_H

#include <stdint.h>
#include "polyform.h"
#include "continuation.h"

/* -------------------------------------------------------------------------- */
/* MAXICODE — 15-bit Type Declaration                                         */
/* -------------------------------------------------------------------------- */

typedef struct {
    uint8_t basis : 4;        /* 0-11: square, triangle, hex, cube, etc. */
    uint8_t degree : 4;       /* 1-16: n-omino */
    uint8_t dimension : 2;    /* 0=1D, 1=2D, 2=2.5D, 3=3D */
    uint8_t group : 4;       /* Symmetry group */
    uint8_t reserved : 1;    /* Future */
    uint16_t checksum : 10;  /* Error check */
} MaxiCode;

/* Encode polyform type to MaxiCode */
static MaxiCode encode_maxicode(const Polyform* poly) {
    MaxiCode mc = {0};
    
    switch (poly->dim) {
        case DIM_1D:
            mc.basis = BASIS_STICK;
            mc.dimension = 0;
            mc.degree = poly->stick.length;
            break;
        case DIM_2D:
            mc.basis = poly->poly2d.basis;
            mc.dimension = 1;
            mc.degree = poly->poly2d.degree;
            break;
        case DIM_2_5D:
            mc.basis = poly->poly2_5d.base->basis;
            mc.dimension = 2;
            mc.degree = poly->poly2_5d.base->degree;
            break;
        case DIM_3D:
            mc.basis = BASIS_CUBE;
            mc.dimension = 3;
            mc.degree = poly->poly3d.degree;
            break;
    }
    
    /* Simple checksum: XOR of all fields */
    uint16_t raw = (mc.basis << 11) | (mc.degree << 7) | (mc.dimension << 5) | mc.group;
    mc.checksum = raw ^ (raw >> 5) ^ 0x1D;
    
    return mc;
}

/* -------------------------------------------------------------------------- */
/* BEETAG — 25-bit Cell Packets (5 cells × 5 bits)                            */
/* -------------------------------------------------------------------------- */

#define BEETAG_CELLS_PER_PACKET 5

typedef struct {
    uint8_t sequence;                     /* 0-31: packet order */
    uint8_t cells[BEETAG_CELLS_PER_PACKET];  /* 5 cells, 2-of-5 encoded */
} BEEtagPacket;

/* Encode polyform cells into BEEtag packets */
static BEEtagPacket* encode_beetag_packets(const Polyform2D* poly, int* packet_count) {
    *packet_count = (poly->count + BEETAG_CELLS_PER_PACKET - 1) / BEETAG_CELLS_PER_PACKET;
    BEEtagPacket* packets = (BEEtagPacket*)malloc(*packet_count * sizeof(BEEtagPacket));
    
    for (int p = 0; p < *packet_count; p++) {
        packets[p].sequence = p;
        for (int c = 0; c < BEETAG_CELLS_PER_PACKET; c++) {
            int cell_idx = p * BEETAG_CELLS_PER_PACKET + c;
            if (cell_idx < poly->count) {
                /* 2-of-5 encoding: position + orientation */
                uint8_t pos = (poly->cells[cell_idx].x & 0x03) | ((poly->cells[cell_idx].y & 0x03) << 2);
                packets[p].cells[c] = (pos << 1) | (poly->cells[cell_idx].orientation & 0x01);
            } else {
                packets[p].cells[c] = 0x1F;  /* Pad */
            }
        }
    }
    
    return packets;
}

/* -------------------------------------------------------------------------- */
/* AZTEC — 40-bit Codepoint (Full State)                                      */
/* -------------------------------------------------------------------------- */

typedef struct {
    uint64_t codepoint : 40;  /* 40-bit virtual address */
    uint8_t layers;            /* Aztec layers (1-32) */
    uint8_t mode_message;      /* Encoded layer count */
} AztecCode;

/* Encode polyform to Aztec codepoint */
static AztecCode encode_aztec(const Polyform* poly) {
    AztecCode az = {0};
    Pair sid = sid_polyform(poly);
    
    /* 40-bit codepoint = SID | dimension << 16 | degree << 20 */
    az.codepoint = sid;
    az.codepoint |= ((uint64_t)poly->dim << 16);
    
    switch (poly->dim) {
        case DIM_1D:
            az.codepoint |= ((uint64_t)poly->stick.length << 20);
            break;
        case DIM_2D:
            az.codepoint |= ((uint64_t)poly->poly2d.degree << 20);
            break;
        case DIM_2_5D:
            az.codepoint |= ((uint64_t)poly->poly2_5d.height << 20);
            break;
        case DIM_3D:
            az.codepoint |= ((uint64_t)poly->poly3d.degree << 20);
            break;
    }
    
    /* Layers determined by codepoint magnitude */
    az.layers = 1;
    uint64_t cp = az.codepoint;
    while (cp > 0xFF && az.layers < 32) {
        cp >>= 8;
        az.layers++;
    }
    
    az.mode_message = az.layers - 1;
    
    return az;
}

/* -------------------------------------------------------------------------- */
/* CODE 16K — 4-Channel Carry Lookahead                                       */
/* -------------------------------------------------------------------------- */

/* Encode Code 16K from two bytes */
static Code16K encode_code16k(uint8_t a, uint8_t b, uint8_t carry_in) {
    uint8_t A[4] = { (a & 1), (a >> 1) & 1, (a >> 2) & 1, (a >> 3) & 1 };
    uint8_t B[4] = { (b & 1), (b >> 1) & 1, (b >> 2) & 1, (b >> 3) & 1 };
    
    Adder4Bit adder = compute_4bit_adder(A, B, carry_in);
    return adder_to_code16k(&adder);
}

/* Decode Code 16K back to bytes */
static void decode_code16k(Code16K code, uint8_t* sum, uint8_t* carry_out) {
    *sum = 0;
    for (int i = 0; i < 4; i++) {
        *sum |= (code.channels[0] & (1 << i)) ? (1 << i) : 0;
    }
    *carry_out = code.channels[3];
}

/* -------------------------------------------------------------------------- */
/* UNIFIED BARCODE FRAME                                                      */
/* -------------------------------------------------------------------------- */

typedef struct {
    MaxiCode type_decl;           /* What is this? */
    BEEtagPacket* packets;        /* Cell packets */
    int packet_count;
    AztecCode full_state;         /* Complete state */
    Code16K channel_data;        /* 4-channel control */
} BarcodeFrame;

/* Instantiate polyform from barcode frame */
static Polyform* instantiate_from_frame(const BarcodeFrame* frame) {
    /* Allocate based on MaxiCode type */
    Polyform* poly = (Polyform*)malloc(sizeof(Polyform));
    poly->dim = frame->type_decl.dimension;
    
    Pair seed = compute_sid(frame->type_decl.checksum);
    
    switch (poly->dim) {
        case DIM_1D: {
            uint8_t angles[32] = {0};
            poly->stick = make_polystick(angles, frame->type_decl.degree, seed);
            break;
        }
        case DIM_2D:
            poly->poly2d = make_polyomino(frame->type_decl.degree, seed);
            break;
        case DIM_2_5D:
            poly->poly2_5d.base = (Polyform2D*)malloc(sizeof(Polyform2D));
            *poly->poly2_5d.base = make_polyomino(frame->type_decl.degree, seed);
            poly->poly2_5d = extrude_polyform(poly->poly2_5d.base, frame->full_state.layers);
            break;
        case DIM_3D:
            poly->poly3d = make_polycube(frame->type_decl.degree, seed);
            break;
    }
    
    poly->unified_sid = sid_polyform(poly);
    return poly;
}

/* Serialize polyform to barcode frame */
static BarcodeFrame serialize_to_frame(const Polyform* poly) {
    BarcodeFrame frame = {0};
    
    frame.type_decl = encode_maxicode(poly);
    frame.full_state = encode_aztec(poly);
    
    if (poly->dim == DIM_2D) {
        frame.packets = encode_beetag_packets(&poly->poly2d, &frame.packet_count);
    }
    
    /* Compute 4-channel adder from polyform SID */
    uint8_t a = car(poly->unified_sid);
    uint8_t b = cdr(poly->unified_sid);
    frame.channel_data = encode_code16k(a, b, 0);
    
    return frame;
}

/* Free barcode frame */
static void free_frame(BarcodeFrame* frame) {
    if (frame->packets) free(frame->packets);
}

#endif /* BARCODE_FRAME_H */
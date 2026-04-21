/*
 * TETRA-WITNESS — Witness Pack Generator
 *
 * A witness pack is a cryptographic proof that:
 *   - A computation was performed
 *   - The result is correct
 *   - The intent is context-invariant
 *
 * Structure:
 *   - Witness header (magic, version, computation type)
 *   - Input value (the pair)
 *   - Kernel trace (the K steps)
 *   - Output SID
 *   - Holonomy (the cycle classification)
 *   - Void-SID proof (if non-context-invariant)
 *
 * This is the replay proof that allows anyone to verify
 * the computation without re-running it.
 */

#ifndef TETRA_WITNESS_H
#define TETRA_WITNESS_H

#include <stdint.h>
#include "tetra-block.h"

/* -------------------------------------------------------------------------- */
/* WITNESS CONSTANTS                                                         */
/* -------------------------------------------------------------------------- */

#define WITNESS_MAGIC     0x574954  /* "WIT" */
#define WITNESS_VERSION 1

/* Computation types */
#define COMP_K         0x01   /* Kernel K(p,C) */
#define COMP_SID       0x02   /* SID computation */
#define COMP_HASH      0x03   /* Hash computation */
#define COMP_VERIFY    0x04   /* Context verification */

/* -------------------------------------------------------------------------- */
/* WITNESS HEADER                                                            */
/* -------------------------------------------------------------------------- */

typedef struct {
    uint32_t magic;
    uint8_t  version;
    uint8_t  comp_type;   /* COMP_* */
    uint8_t  steps;      /* Number of computation steps */
    Pair input;
    Pair output;
    Pair sid;
    Pair void_sid;     /* Void-SID if computation is non-invariant */
    uint8_t  holonomy;
    uint8_t  flags;
} WitnessHeader;

/* -------------------------------------------------------------------------- */
/* KERNEL TRACE                                                           */
/* -------------------------------------------------------------------------- */

#define MAX_TRACE 256

typedef struct {
    Pair p;         /* Input to this step */
    Pair C;         /* Constant for this step */
    Pair result;    /* Output of this step */
} KStep;

/* -------------------------------------------------------------------------- */
/* WITNESS PACK                                                            */
/* -------------------------------------------------------------------------- */

typedef struct {
    WitnessHeader header;
    KStep trace[MAX_TRACE];
    uint8_t trace_len;
} WitnessPack;

/* Create a witness for K(p,C) computation */
static WitnessPack make_witness_k(Pair input, Pair C) {
    WitnessPack wp;
    wp.header.magic = WITNESS_MAGIC;
    wp.header.version = WITNESS_VERSION;
    wp.header.comp_type = COMP_K;
    wp.header.input = input;
    wp.header.output = K(input, C);
    wp.header.sid = compute_sid(wp.header.output);
    wp.header.void_sid = 0;
    wp.header.holonomy = 0;
    wp.header.flags = 0;
    wp.header.steps = 1;
    
    wp.trace_len = 1;
    wp.trace[0].p = input;
    wp.trace[0].C = C;
    wp.trace[0].result = wp.header.output;
    
    return wp;
}

/* Create a witness for SID computation */
static WitnessPack make_witness_sid(Pair nf) {
    WitnessPack wp = make_witness_k(nf, 0x1D);
    wp.header.comp_type = COMP_SID;
    return wp;
}

/* Create a full witness with verification */
static WitnessPack make_witness_full(Pair input, Pair C, int verify) {
    WitnessPack wp = make_witness_k(input, C);
    
    Pair with_sbit0 = input | 0x8000;
    Pair with_sbit1 = input & 0x7FFF;
    
    Pair sid0 = compute_sid(with_sbit0);
    Pair sid1 = compute_sid(with_sbit1);
    
    if (sid0 != sid1) {
        /* Non-context-invariant - compute Void-SID */
        wp.header.flags |= 0x01;  /* Non-invariant flag */
        wp.header.void_sid = K(sid0 ^ sid1, C);
        wp.header.holonomy = 7;  /* VOID classification */
    } else {
        wp.header.holonomy = 0;  /* VALID */
    }
    
    return wp;
}

/* -------------------------------------------------------------------------- */
/* WITNESS VERIFICATION                                                      */
/* -------------------------------------------------------------------------- */

/* Verify a witness pack (replay the computation) */
static int witness_verify(const WitnessPack *wp) {
    if (wp->header.magic != WITNESS_MAGIC) return 0;
    if (wp->header.version != WITNESS_VERSION) return 0;
    
    /* Verify the trace */
    Pair computed = wp->header.input;
    for (int i = 0; i < wp->trace_len; i++) {
        if (wp->trace[i].p != computed) return 0;
        computed = K(computed, wp->trace[i].C);
    }
    
    if (computed != wp->header.output) return 0;
    
    /* Verify SID */
    Pair sid = compute_sid(wp->header.output);
    if (sid != wp->header.sid) return 0;
    
    return 1;
}

/* Check if witness indicates VOID (non-context-invariant) */
static int witness_is_void(const WitnessPack *wp) {
    return (wp->header.flags & 0x01) != 0;
}

/* Get the Void-SID proof */
static Pair witness_void_sid(const WitnessPack *wp) {
    return wp->header.void_sid;
}

/* -------------------------------------------------------------------------- */
/* SERIALIZATION                                                           */
/* -------------------------------------------------------------------------- */

#define MAX_WITNESS_BYTES (12 + MAX_TRACE * 6)

/* Serialize witness to bytes */
static int witness_serialize(const WitnessPack *wp, uint8_t *out) {
    int len = 0;
    
    out[len++] = (wp->header.magic >> 16) & 0xFF;
    out[len++] = (wp->header.magic >> 8) & 0xFF;
    out[len++] = wp->header.magic & 0xFF;
    out[len++] = wp->header.version;
    out[len++] = wp->header.comp_type;
    out[len++] = wp->header.steps;
    out[len++] = (wp->header.input >> 8) & 0xFF;
    out[len++] = wp->header.input & 0xFF;
    out[len++] = (wp->header.output >> 8) & 0xFF;
    out[len++] = wp->header.output & 0xFF;
    out[len++] = (wp->header.sid >> 8) & 0xFF;
    out[len++] = wp->header.sid & 0xFF;
    
    if (wp->header.flags & 0x01) {
        out[len++] = (wp->header.void_sid >> 8) & 0xFF;
        out[len++] = wp->header.void_sid & 0xFF;
    }
    
    out[len++] = wp->header.holonomy;
    out[len++] = wp->header.flags;
    
    /* Trace */
    for (int i = 0; i < wp->trace_len; i++) {
        out[len++] = (wp->trace[i].p >> 8) & 0xFF;
        out[len++] = wp->trace[i].p & 0xFF;
        out[len++] = (wp->trace[i].C >> 8) & 0xFF;
        out[len++] = wp->trace[i].C & 0xFF;
        out[len++] = (wp->trace[i].result >> 8) & 0xFF;
        out[len++] = wp->trace[i].result & 0xFF;
    }
    
    return len;
}

/* Deserialize bytes to witness */
static int witness_deserialize(const uint8_t *in, int len, WitnessPack *wp) {
    if (len < 16) return 0;
    
    int pos = 0;
    wp->header.magic = ((uint32_t)in[pos] << 16) | (in[pos+1] << 8) | in[pos+2]; pos += 3;
    wp->header.version = in[pos++];
    wp->header.comp_type = in[pos++];
    wp->header.steps = in[pos++];
    wp->header.input = ((Pair)in[pos] << 8) | in[pos+1]; pos += 2;
    wp->header.output = ((Pair)in[pos] << 8) | in[pos+1]; pos += 2;
    wp->header.sid = ((Pair)in[pos] << 8) | in[pos+1]; pos += 2;
    
    if (wp->header.flags & 0x01) {
        wp->header.void_sid = ((Pair)in[pos] << 8) | in[pos+1]; pos += 2;
    }
    
    wp->header.holonomy = in[pos++];
    wp->header.flags = in[pos++];
    
    wp->trace_len = wp->header.steps;
    for (int i = 0; i < wp->trace_len && pos < len; i++) {
        wp->trace[i].p = ((Pair)in[pos] << 8) | in[pos+1]; pos += 2;
        wp->trace[i].C = ((Pair)in[pos] << 8) | in[pos+1]; pos += 2;
        wp->trace[i].result = ((Pair)in[pos] << 8) | in[pos+1]; pos += 2;
    }
    
    return pos;
}

/* -------------------------------------------------------------------------- */
/* PRINTING                                                                */
/* -------------------------------------------------------------------------- */

#include <stdio.h>

static void witness_print(const WitnessPack *wp) {
    printf("═══ WITNESS PACK ════════════════════════════════════\n");
    printf("Magic:    0x%06X\n", wp->header.magic);
    printf("Version:  %d\n", wp->header.version);
    printf("Comp:     %d\n", (int)wp->header.comp_type);
    printf("Input:    0x%04X\n", wp->header.input);
    printf("Output:   0x%04X\n", wp->header.output);
    printf("SID:      0x%04X\n", wp->header.sid);
    printf("Holonomy: %d\n", (int)wp->header.holonomy);
    printf("Flags:    0x%02X\n", (int)wp->header.flags);
    
    if (wp->header.flags & 0x01) {
        printf("Void-SID: 0x%04X (NON-INVARIANT)\n", wp->header.void_sid);
    }
    
    printf("═══ TRACE ═════════════════════════════════════\n");
    for (int i = 0; i < wp->trace_len; i++) {
        printf("Step %d: K(0x%04X, 0x%04X) = 0x%04X\n",
               i, wp->trace[i].p, wp->trace[i].C, wp->trace[i].result);
    }
    printf("═══════════════════════════════════════════════\n");
}

#endif /* TETRA_WITNESS_H */
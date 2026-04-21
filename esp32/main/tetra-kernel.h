/*
 * TETRA-KERNEL — Constitutional Core for ESP32
 * 
 * No Python. No external dependencies. Pure C.
 * 
 * This is the foundation: pair → kernel → COBS → SID → witness → log → chain → mesh
 */

#ifndef TETRA_KERNEL_H
#define TETRA_KERNEL_H

#include <stdint.h>

/* -------------------------------------------------------------------------- */
/* LAYER 0: PAIR (16-bit primitive)                                           */
/* -------------------------------------------------------------------------- */
/*
 * A Pair is a 16-bit word.
 * Bits 15-8: car (left projection)
 * Bits 7-0:  cdr (right projection)
 */

typedef uint16_t Pair;

#define cons(a,d) ((((a) & 0xFF) << 8) | ((d) & 0xFF))
#define car(p)    (((p) >> 8) & 0xFF)
#define cdr(p)    ((p) & 0xFF)

/* Constitutional constants from ASCII separator table */
#define CONSTITUTIONAL_C  0x1D  /* GS - Group Separator */
#define COBS_DELIMITER     0x00
#define COBS_ESCAPE       0x01
#define LAYER_FS          0x1C  /* File Separator */
#define LAYER_GS          0x1D  /* Group Separator */
#define LAYER_RS          0x1E  /* Record Separator */
#define LAYER_US          0x1F  /* Unit Separator */

/* -------------------------------------------------------------------------- */
/* LAYER 1: KERNEL K(p,C)                                                   */
/* -------------------------------------------------------------------------- */
/*
 * K(p, C) = rotl(p,1) ^ rotl(p,3) ^ rotr(p,2) ^ C
 * 
 * This is the atomic kernel. It operates on pairs and produces pairs.
 * On ESP32, this compiles to ~6 Xtensa instructions.
 */

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

/* -------------------------------------------------------------------------- */
/* LAYER 2: SID AND CONTEXT INVARIANCE                                      */
/* -------------------------------------------------------------------------- */
/*
 * SID is the Semantic Identifier: hash of the NormalForm.
 * A computation is context-invariant if SID(s=0) == SID(s=1).
 */

static inline Pair compute_sid(Pair nf) {
    return K(nf, CONSTITUTIONAL_C);
}

static inline int is_context_invariant(Pair value) {
    Pair v0 = cons(car(value), (uint8_t)(cdr(value) | 0x80));
    Pair v1 = cons(car(value), (uint8_t)(cdr(value) & 0x7F));
    return compute_sid(v0) == compute_sid(v1);
}

/* Holonomy classification: 0=valid, 1-6=cycle lengths, 7=VOID */
static inline int holonomy_of(Pair p) {
    Pair k1 = K(p, CONSTITUTIONAL_C);
    Pair k2 = K(k1, CONSTITUTIONAL_C);
    Pair k3 = K(k2, CONSTITUTIONAL_C);
    uint8_t h = ((k1 ^ k2 ^ k3) >> 8) & 0xFF;
    if (h < 64) return 7;      /* VOID hole - singular */
    if (h < 128) return 1;     /* Singularity */
    if (h < 160) return 2;     /* 2-cycle */
    if (h < 192) return 4;     /* 4-cycle */
    return 0;                  /* Valid */
}

/* -------------------------------------------------------------------------- */
/* COBS FRAMING                                                       */
/* -------------------------------------------------------------------------- */
/*
 * A COBS frame is a switching unit: [0x00][payload with s-bit][0x00]
 * The s-bit is constant within the payload.
 */

#define MAX_FRAME 256

typedef struct {
    uint8_t data[MAX_FRAME];
    int length;
    int s_bit;
} COBSFrame;

static COBSFrame cobs_encode(Pair value, int s_bit) {
    COBSFrame frame = { .length = 0, .s_bit = s_bit };
    
    frame.data[frame.length++] = COBS_DELIMITER;
    
    uint8_t b0 = s_bit ? (car(value) | 0x80) : car(value);
    if (b0 == COBS_DELIMITER) frame.data[frame.length++] = COBS_ESCAPE;
    frame.data[frame.length++] = b0;
    
    uint8_t b1 = s_bit ? (cdr(value) | 0x80) : cdr(value);
    if (b1 == COBS_DELIMITER) frame.data[frame.length++] = COBS_ESCAPE;
    frame.data[frame.length++] = b1;
    
    frame.data[frame.length++] = COBS_DELIMITER;
    return frame;
}

static Pair cobs_decode(const uint8_t *data, int len, int *s_bit_out) {
    if (len < 3) return 0;
    if (data[0] != COBS_DELIMITER || data[len-1] != COBS_DELIMITER) return 0;
    
    uint8_t decoded[2] = {0};
    int out_len = 0, s_bit = 0;
    
    for (int i = 1; i < len - 1 && out_len < 2; i++) {
        uint8_t b = data[i];
        if (b == COBS_ESCAPE) {
            b = COBS_DELIMITER;
        } else if (b & 0x80) {
            s_bit = 1;
            b &= 0x7F;
        }
        decoded[out_len++] = b;
    }
    
    *s_bit_out = s_bit;
    return cons(decoded[0], decoded[1]);
}

/* -------------------------------------------------------------------------- */
/* BLOCK CHAIN (FS ← GS ← RS ← US)                                          */
/* -------------------------------------------------------------------------- */

#define MAX_PAYLOAD 240

typedef struct {
    uint8_t layer;
    Pair base_sid;
    Pair delta_sid;
    Pair combined_sid;
    uint8_t s_bit;
    uint8_t payload_len;
    uint8_t payload[MAX_PAYLOAD];
} BlockHeader;

typedef struct {
    BlockHeader blocks[4];
    int count;
    Pair root_sid;
    Pair chain_sid;
} BlockChain;

static void chain_init(BlockChain *chain, Pair initial) {
    chain->count = 0;
    chain->root_sid = compute_sid(initial);
    chain->chain_sid = chain->root_sid;
}

static void chain_add(BlockChain *chain, BlockHeader *blk) {
    if (chain->count >= 4) return;
    blk->base_sid = chain->chain_sid;
    chain->chain_sid = K(chain->chain_sid, blk->delta_sid);
    blk->combined_sid = chain->chain_sid;
    chain->blocks[chain->count++] = *blk;
}

/* -------------------------------------------------------------------------- */
/* NRR LOG ENTRY                                                      */
/* -------------------------------------------------------------------------- */

#define NRR_MAGIC 0x4E525230  /* "NRR0" */

typedef struct {
    Pair entry_sid;
    Pair prev_sid;
    Pair confirm_sid;
    uint8_t flags;
    uint8_t issuer;
    uint32_t timestamp;
    uint8_t content_len;
    uint8_t content[MAX_PAYLOAD];
} NRREntry;

typedef struct {
    uint32_t magic;
    uint8_t version;
    uint8_t entry_count;
    Pair root_sid;
    Pair tip_sid;
    Pair genesis_sid;
    NRREntry entries[256];
    uint32_t count;
} NRRLog;

static void nrr_init(NRRLog *log) {
    log->magic = (uint32_t)NRR_MAGIC;
    log->version = 1;
    log->entry_count = 0;
    log->genesis_sid = K((Pair)0, CONSTITUTIONAL_C);
    log->root_sid = log->genesis_sid;
    log->tip_sid = log->root_sid;
    log->count = 0;
}

static Pair nrr_append(NRRLog *log, uint8_t *content, int len, uint8_t issuer) {
    if (log->count >= 256) return 0;
    
    Pair content_sid = len >= 2 ? 
        ((Pair)content[0] << 8) | content[1] : 
        ((Pair)content[0] << 8);
    
    NRREntry e = {
        .entry_sid = content_sid,
        .prev_sid = log->tip_sid,
        .confirm_sid = 0,
        .flags = 0,
        .issuer = issuer,
        .timestamp = (uint32_t)(log->tip_sid ^ content_sid),
        .content_len = (uint8_t)(len < MAX_PAYLOAD ? len : MAX_PAYLOAD)
    };
    for (int i = 0; i < e.content_len; i++) {
        e.content[i] = content[i];
    }
    
    e.confirm_sid = K(e.entry_sid, e.timestamp);
    log->tip_sid = K(log->tip_sid, e.confirm_sid);
    log->entries[log->count++] = e;
    log->entry_count = (uint8_t)log->count;
    
    return e.confirm_sid;
}

/* -------------------------------------------------------------------------- */
/* WITNESS PACK                                                      */
/* -------------------------------------------------------------------------- */

#define WITNESS_MAGIC 0x574954  /* "WIT" */

typedef struct {
    Pair input;
    Pair output;
    Pair sid;
    Pair void_sid;
    uint8_t holonomy;
    uint8_t flags;
} WitnessHeader;

static WitnessHeader make_witness(Pair input, Pair C, int verify) {
    WitnessHeader wp = {
        .input = input,
        .output = K(input, C),
        .sid = 0,
        .void_sid = 0,
        .holonomy = 0,
        .flags = 0
    };
    
    wp.sid = compute_sid(wp.output);
    wp.holonomy = holonomy_of(wp.output);
    
    if (verify) {
        Pair with_sbit0 = input | 0x8000;
        Pair with_sbit1 = input & 0x7FFF;
        Pair sid0 = compute_sid(with_sbit0);
        Pair sid1 = compute_sid(with_sbit1);
        
        if (sid0 != sid1) {
            wp.flags = 0x01;
            wp.void_sid = K(sid0 ^ sid1, C);
            wp.holonomy = 7;
        }
    }
    
    return wp;
}

#endif /* TETRA_KERNEL_H */
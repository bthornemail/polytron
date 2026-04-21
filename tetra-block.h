/*
 * TETRA-BLOCK — Constitutional Block Chain Protocol
 *
 * Block layers named after ASCII separators from gauge transition table:
 *   FS (0x1C): File Separator — Hex×Decimal plane
 *   GS (0x1D): Group Separator — Hex×Binary plane  [CONSTITUTIONAL_C]
 *   RS (0x1E): Record Separator — Decimal×Binary plane
 *   US (0x1F): Unit Separator — All three planes, s=1
 *
 * Chain: [FS] <-- [GS] <-- [RS] <-- [US]
 *
 * Each layer is a COBS frame with specific s-bit and plane.
 * The combined SID verifies the entire chain.
 */

#ifndef TETRA_BLOCK_H
#define TETRA_BLOCK_H

#include <stdint.h>

/* -------------------------------------------------------------------------- */
/* CONSTITUTIONAL LAYERS                                                         */
/* -------------------------------------------------------------------------- */

typedef uint8_t BlockLayer;

#define LAYER_FS  ((BlockLayer)0x1C)  /* File Separator — base layer */
#define LAYER_GS  ((BlockLayer)0x1D)  /* Group Separator — overlay 1 */
#define LAYER_RS  ((BlockLayer)0x1E)  /* Record Separator — overlay 2 */
#define LAYER_US  ((BlockLayer)0x1F)  /* Unit Separator — active layer */

#define LAYER_COUNT 4

static const char *layer_name(uint8_t layer) {
    if (layer == 0x1C) return "FS";
    if (layer == 0x1D) return "GS";
    if (layer == 0x1E) return "RS";
    if (layer == 0x1F) return "US";
    return "??";
}

/* -------------------------------------------------------------------------- */
/* KERNEL (must match kernel.c)                                              */
/* -------------------------------------------------------------------------- */

typedef uint16_t Pair;

static Pair rotl(Pair x, int n) {
    n &= 15;
    return (Pair)((x << n) | (x >> (16 - n)));
}

static Pair rotr(Pair x, int n) {
    n &= 15;
    return (Pair)((x >> n) | (x << (16 - n)));
}

static Pair K(Pair p, Pair C) {
    return rotl(p,1) ^ rotl(p,3) ^ rotr(p,2) ^ C;
}

static Pair compute_sid(Pair nf) {
    return K(nf, 0x1D);  /* GS is the constitutional constant */
}

/* -------------------------------------------------------------------------- */
/* BLOCK HEADER                                                            */
/* -------------------------------------------------------------------------- */

#define MAX_PAYLOAD 240

typedef struct {
    BlockLayer layer;
    Pair base_sid;       /* SID of the backing layer */
    Pair delta_sid;     /* SID of this layer's delta */
    Pair combined_sid;   /* SID of combined state */
    uint8_t s_bit;      /* Structural sign (0 for FS/GS/RS, 1 for US) */
    uint8_t payload_len;
    uint8_t payload[MAX_PAYLOAD];
} BlockHeader;

/* Create a block header */
static BlockHeader make_block(BlockLayer layer, Pair delta, 
                              const uint8_t *payload, int len) {
    BlockHeader blk = {
        .layer = layer,
        .base_sid = 0,
        .delta_sid = delta,
        .combined_sid = 0,
        .s_bit = (layer == (BlockLayer)0x1F) ? (uint8_t)1 : (uint8_t)0,
        .payload_len = (uint8_t)len
    };
    for (int i = 0; i < len && i < MAX_PAYLOAD; i++) {
        blk.payload[i] = payload[i];
    }
    /* Compute the delta SID from payload */
    if (len > 0) {
        Pair p = (len >= 2) ? 
            ((Pair)payload[0] << 8) | payload[1] : 
            ((Pair)payload[0] << 8);
        blk.delta_sid = compute_sid(p);
    }
    return blk;
}

/* -------------------------------------------------------------------------- */
/* BLOCK CHAIN                                                            */
/* -------------------------------------------------------------------------- */

typedef struct {
    BlockHeader blocks[LAYER_COUNT];
    int count;
    Pair root_sid;       /* SID of FS layer */
    Pair chain_sid;      /* SID of entire chain */
} BlockChain;

/* Initialize a block chain */
static void chain_init(BlockChain *chain, Pair initial_state) {
    chain->count = 0;
    chain->root_sid = compute_sid(initial_state);
    chain->chain_sid = chain->root_sid;
}

/* Add a layer to the chain */
static void chain_add(BlockChain *chain, BlockHeader *blk) {
    if (chain->count >= LAYER_COUNT) return;
    
    /* The base is the previous layer's combined SID */
    blk->base_sid = chain->chain_sid;
    
    /* Compute the new combined SID */
    Pair prev = chain->chain_sid;
    chain->chain_sid = K(prev, blk->delta_sid);
    blk->combined_sid = chain->chain_sid;
    
    chain->blocks[chain->count++] = *blk;
}

/* Verify the entire chain */
static int chain_verify(BlockChain *chain) {
    Pair computed = chain->root_sid;
    
    for (int i = 0; i < chain->count; i++) {
        if (chain->blocks[i].base_sid != computed) return 0;
        computed = K(computed, chain->blocks[i].delta_sid);
        if (computed != chain->blocks[i].combined_sid) return 0;
    }
    
    return computed == chain->chain_sid;
}

/* Get the active SID (US layer) */
static Pair chain_active_sid(BlockChain *chain) {
    return chain->chain_sid;
}

/* -------------------------------------------------------------------------- */
/* SERIALIZATION                                                           */
/* -------------------------------------------------------------------------- */

#define MAX_MESSAGE (1 + 1 + 2 + 1 + 1 + 1 + MAX_PAYLOAD)

/* Serialize a block to bytes */
static int block_serialize(const BlockHeader *blk, uint8_t *out) {
    int len = 0;
    out[len++] = blk->layer;
    out[len++] = blk->s_bit;
    out[len++] = (blk->base_sid >> 8) & 0xFF;
    out[len++] = blk->base_sid & 0xFF;
    out[len++] = (blk->delta_sid >> 8) & 0xFF;
    out[len++] = blk->delta_sid & 0xFF;
    out[len++] = blk->payload_len;
    for (int i = 0; i < blk->payload_len && i < MAX_PAYLOAD; i++) {
        out[len++] = blk->payload[i];
    }
    return len;
}

/* Deserialize bytes to block */
static int block_deserialize(const uint8_t *in, int len, BlockHeader *blk) {
    if (len < 7) return 0;
    int pos = 0;
    blk->layer = in[pos++];
    blk->s_bit = in[pos++];
    blk->base_sid = ((Pair)in[pos] << 8) | in[pos+1]; pos += 2;
    blk->delta_sid = ((Pair)in[pos] << 8) | in[pos+1]; pos += 2;
    blk->payload_len = in[pos++];
    for (int i = 0; i < blk->payload_len && pos < len; i++) {
        blk->payload[i] = in[pos++];
    }
    blk->combined_sid = K(blk->base_sid, blk->delta_sid);
    return pos;
}

/* -------------------------------------------------------------------------- */
/* PRINTING                                                                */
/* -------------------------------------------------------------------------- */

#include <stdio.h>

static void chain_print(const BlockChain *chain) {
    printf("═══ BLOCK CHAIN ═════════════════════════════\n");
    printf("Root SID:    0x%04X\n", chain->root_sid);
    printf("Chain SID:   0x%04X\n", chain->chain_sid);
    printf("Layer count: %d\n\n", chain->count);
    
    for (int i = 0; i < chain->count; i++) {
        const BlockHeader *b = &chain->blocks[i];
        printf("[%s] layer: s_bit=%d, base=0x%04X, delta=0x%04X, combined=0x%04X\n",
               layer_name(b->layer), b->s_bit, b->base_sid, b->delta_sid, b->combined_sid);
    }
    printf("═══════════════════════════════════════════════\n");
}

#endif /* TETRA_BLOCK_H */
/*
 * kernel.h — The Kernel Function on Pairs
 *
 * K(p, C) = rotl(p,1) ^ rotl(p,3) ^ rotr(p,2) ^ C
 *
 * Circular feed: C mutates each step from fold of term.
 */

#ifndef KERNEL_H
#define KERNEL_H

#include "pair.h"

/* 16-bit rotation */
static inline Pair krotl(Pair p, int n) {
    n &= 15;
    return (Pair)((p << n) | (p >> (16 - n)));
}

static inline Pair krotr(Pair p, int n) {
    n &= 15;
    return (Pair)((p >> n) | (p << (16 - n)));
}

/* K: the only transformation */
static inline Pair K(Pair p, Pair C) {
    return krotl(p,1) ^ krotl(p,3) ^ krotr(p,2) ^ C;
}

/* Operator selection from kernel byte */
typedef enum { OP_AND=0, OP_OR, OP_XOR, OP_NOT, OP_LSFT, OP_RSFT } Op;

static Op byte_op(unsigned char b) {
    int hi = (b >> 4) & 0xF;
    if (hi <= 1)  return OP_AND;
    if (hi <= 3)  return OP_OR;
    if (hi <= 5)  return OP_XOR;
    if (hi <= 7)  return OP_NOT;
    if (hi <= 11) return OP_LSFT;
    return OP_RSFT;
}

/* Apply operator to atom */
static Pair apply_op(Op op, Pair atom) {
    unsigned int a = car(atom);
    unsigned int d = cdr(atom);
    unsigned int mask = (d == 0) ? 0x7F : 0xFF;
    switch (op) {
        case OP_AND:  a &= 0x1F; break;
        case OP_OR:   a |= 0x01; break;
        case OP_XOR:  a ^= 0x01; break;
        case OP_NOT:  a = (~a) & mask; break;
        case OP_LSFT: a = (a << 1) & mask; break;
        case OP_RSFT: a = a >> 1; break;
    }
    return cons(a, d);
}

/* Braille conversion */
static unsigned int brl_hexwt(unsigned char b) {
    static const unsigned int w[8] = {1,2,4,64,16,8,32,128};
    unsigned int r = 0; int i;
    for (i = 0; i < 8; i++) if (b & (1u<<i)) r += w[i];
    return r & 0xFF;
}

/* Byte to pair atom */
static Pair byte_to_pair(unsigned char b) {
    int sel = (b >> 6) & 3;
    if (sel == 1) return cons(brl_hexwt(b), 3);    /* Braille */
    if (sel == 2) return cons(b % 52, 4);        /* Aegean */
    return cons(b & 0x7F, 0);                   /* ASCII */
}

/* Fold term to new C */
static Pair fold_term(Pair term) {
    Pair acc = NIL;
    while (is_ptr(term)) {
        Pair a = mem_car(term);
        acc = (Pair)(acc ^ a);
        term = mem_cdr(term);
    }
    return acc;
}

/* Rewriter state */
typedef struct {
    Pair  state;       /* current 16-bit kernel state */
    Pair  C;           /* current constant */
    Pair  term;        /* current rewrite term */
    int   tick;
    int   max_len;
} KRewriter;

static void krewriter_init(KRewriter *rw, Pair seed, Pair C, int max_len) {
    rw->state   = seed;
    rw->C       = C;
    rw->term    = NIL;
    rw->tick    = 0;
    rw->max_len = max_len;
}

/* One step */
static Pair krewriter_step(KRewriter *rw) {
    rw->state = K(rw->state, rw->C);
    unsigned char b = (unsigned char)(rw->state & 0xFF);
    Pair atom = byte_to_pair(b);
    Op op = byte_op(b);
    Pair transformed = apply_op(op, atom);
    rw->term = mem_alloc(transformed, rw->term);
    if (rw->max_len > 0 && pair_length(rw->term) > rw->max_len) {
        Pair cur = rw->term;
        int i;
        for (i = 0; i < rw->max_len - 1 && is_ptr(mem_cdr(cur)); i++)
            cur = mem_cdr(cur);
        mem_setcdr(cur, NIL);
    }
    rw->C = fold_term(rw->term);
    rw->tick++;
    return transformed;
}

#endif
/*
 * CONTINUATION.H — Polyform Logic Continuations
 * 
 * Appendix G of Code 16K as a finite state machine for carry propagation.
 * Maps FS/GS/RS/US channels to continuation operations.
 */

#ifndef CONTINUATION_H
#define CONTINUATION_H

#include <stdint.h>
#include "polyform.h"

/* -------------------------------------------------------------------------- */
/* APPENDIX G MODES (from Code 16K)                                           */
/* -------------------------------------------------------------------------- */

typedef enum {
    MODE_A = 0,           /* Control chars (0-95) — Reset */
    MODE_B = 1,          /* Printable ASCII (32-127) — Propagate */
    MODE_C = 2,          /* Numeric double-density — Generate */
    MODE_C_FNC1 = 3,     /* FNC1 + numerics — AND gate */
    MODE_B_FNC1 = 4,     /* FNC1 alone — OR gate */
    MODE_C_SHIFT_B = 5,  /* Odd numerics (3+) — Lookahead */
    MODE_C_DOUBLE_SHIFT_B = 6  /* Non-numeric + even numerics — Double lookahead */
} AppendixG_Mode;

/* -------------------------------------------------------------------------- */
/* 4-CHANNEL CARRY LOOKAHEAD STATE                                            */
/* -------------------------------------------------------------------------- */

typedef struct {
    uint8_t fs;   /* XOR gate — sum */
    uint8_t gs;   /* AND gate — carry generate */
    uint8_t rs;   /* OR gate — carry propagate */
    uint8_t us;   /* Shift register — lookahead logic */
    uint8_t carry_in;
    uint8_t carry_out;
} CarryLookahead4;

/* -------------------------------------------------------------------------- */
/* APPENDIX G RULES AS LOGIC GATES                                            */
/* -------------------------------------------------------------------------- */

/* Rule 1a: FNC1 + 2+ numerics → AND gate (generate) */
static inline uint8_t rule_1a(uint8_t a, uint8_t b) {
    return a & b;  /* AND — GS channel */
}

/* Rule 1b: FNC1 alone → OR gate (propagate) */
static inline uint8_t rule_1b(uint8_t a, uint8_t b) {
    return a | b;  /* OR — RS channel */
}

/* Rule 1c: Even numerics → XOR (sum) */
static inline uint8_t rule_1c(uint8_t a, uint8_t b) {
    return a ^ b;  /* XOR — FS channel */
}

/* Rule 1d-1g: Shift rules → Carry lookahead */
static inline uint8_t rule_lookahead(uint8_t g, uint8_t p, uint8_t cin) {
    return g | (p & cin);  /* US channel — carry lookahead */
}

/* -------------------------------------------------------------------------- */
/* 4-BIT CARRY LOOKAHEAD USING APPENDIX G                                     */
/* -------------------------------------------------------------------------- */

typedef struct {
    uint8_t G[4];  /* Generate signals (GS channel) */
    uint8_t P[4];  /* Propagate signals (RS channel) */
    uint8_t S[4];  /* Sum signals (FS channel) */
    uint8_t C[5];  /* Carry chain (US channel) — C[0] = carry_in */
} Adder4Bit;

/* Compute 4-bit addition using Appendix G rules */
static Adder4Bit compute_4bit_adder(uint8_t A[4], uint8_t B[4], uint8_t carry_in) {
    Adder4Bit adder;
    adder.C[0] = carry_in;
    
    for (int i = 0; i < 4; i++) {
        uint8_t a_bit = (A[i] >> i) & 1;
        uint8_t b_bit = (B[i] >> i) & 1;
        
        adder.G[i] = rule_1a(a_bit, b_bit);  /* AND — GS channel */
        adder.P[i] = rule_1b(a_bit, b_bit);  /* OR — RS channel */
        adder.S[i] = rule_1c(a_bit, b_bit) ^ adder.C[i];  /* XOR — FS channel */
        adder.C[i+1] = rule_lookahead(adder.G[i], adder.P[i], adder.C[i]);  /* US channel */
    }
    
    return adder;
}

/* -------------------------------------------------------------------------- */
/* CODE 16K CHANNEL CONSTANTS                                                 */
/* -------------------------------------------------------------------------- */

#define CH_FS 0x1C   /* File Separator — XOR (sum) */
#define CH_GS 0x1D  /* Group Separator — AND (generate) */
#define CH_RS 0x1E  /* Record Separator — OR (propagate) */
#define CH_US 0x1F  /* Unit Separator — Shift (lookahead) */

/* -------------------------------------------------------------------------- */
/* CODE 16K ENCODING/DECODING                                                 */
/* -------------------------------------------------------------------------- */

typedef struct {
    uint8_t rows;             /* 2-16 rows */
    uint8_t channels[4];      /* FS, GS, RS, US data per row */
    uint8_t start_mode;       /* Appendix G starting mode (0-6) */
    uint8_t check_c1;         /* First check character */
    uint8_t check_c2;         /* Second check character */
} Code16K;

/* Encode 4-channel carry lookahead to Code 16K */
static Code16K adder_to_code16k(const Adder4Bit* adder) {
    Code16K code;
    code.rows = 4;
    code.start_mode = MODE_C;
    
    for (int i = 0; i < 4; i++) {
        code.channels[0] |= (adder->S[i] << i);  /* FS */
        code.channels[1] |= (adder->G[i] << i);  /* GS */
        code.channels[2] |= (adder->P[i] << i);  /* RS */
    }
    code.channels[3] = adder->C[4];  /* US: carry out */
    
    /* Check characters (modulo 107) */
    uint16_t sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += code.channels[i] * (i + 2);
    }
    code.check_c1 = sum % 107;
    code.check_c2 = (sum + code.check_c1) % 107;
    
    return code;
}

/* Decode Code 16K back to 4-channel adder outputs */
static Adder4Bit code16k_to_adder(const Code16K* code) {
    Adder4Bit adder;
    
    for (int i = 0; i < 4; i++) {
        adder.S[i] = (code->channels[0] >> i) & 1;
        adder.G[i] = (code->channels[1] >> i) & 1;
        adder.P[i] = (code->channels[2] >> i) & 1;
    }
    adder.C[0] = 0;
    adder.C[4] = code->channels[3];
    
    return adder;
}

/* -------------------------------------------------------------------------- */
/* POLYFORM FSM                                                               */
/* -------------------------------------------------------------------------- */

typedef struct PolyformFSM {
    AppendixG_Mode current_mode;
    PolyformCont* continuation;
    CarryLookahead4 carry_state;
    uint8_t input_buffer[256];
    uint16_t input_len;
} PolyformFSM;

/* Initialize FSM with a polyform continuation */
static PolyformFSM* fsm_init(PolyformCont* cont) {
    PolyformFSM* fsm = (PolyformFSM*)malloc(sizeof(PolyformFSM));
    fsm->current_mode = MODE_B;
    fsm->continuation = cont;
    fsm->carry_state.fs = 0;
    fsm->carry_state.gs = 0;
    fsm->carry_state.rs = 0;
    fsm->carry_state.us = 0;
    fsm->carry_state.carry_in = 0;
    fsm->carry_state.carry_out = 0;
    fsm->input_len = 0;
    return fsm;
}

/* Step the FSM with an input symbol */
static AppendixG_Mode fsm_step(PolyformFSM* fsm, uint8_t symbol) {
    if (fsm->input_len < 256) {
        fsm->input_buffer[fsm->input_len++] = symbol;
    }
    
    /* Apply Appendix G rules */
    if (symbol >= '0' && symbol <= '9') {
        fsm->current_mode = MODE_C;
    } else if (symbol < 32) {
        fsm->current_mode = MODE_A;
    } else {
        fsm->current_mode = MODE_B;
    }
    
    /* Update carry state based on mode */
    switch (fsm->current_mode) {
        case MODE_C:
            fsm->carry_state.fs ^= symbol;  /* XOR */
            break;
        case MODE_A:
            fsm->carry_state.gs &= symbol;  /* AND */
            break;
        case MODE_B:
            fsm->carry_state.rs |= symbol;  /* OR */
            break;
        default:
            break;
    }
    
    return fsm->current_mode;
}

/* Evaluate the continuation to completion */
static Pair fsm_evaluate(PolyformFSM* fsm) {
    /* Compute final SID from carry state */
    Pair combined = cons(fsm->carry_state.fs, fsm->carry_state.gs);
    combined = K(combined, cons(fsm->carry_state.rs, fsm->carry_state.us));
    combined = K(combined, cons(fsm->carry_state.carry_in, fsm->carry_state.carry_out));
    
    /* Fold through continuation */
    combined = K(combined, sid_continuation(fsm->continuation));
    
    return combined;
}

/* Free FSM */
static void fsm_free(PolyformFSM* fsm) {
    if (fsm) free(fsm);
}

#endif /* CONTINUATION_H */
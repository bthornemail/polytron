#ifndef POLYTRON_ACCELERATOR_H
#define POLYTRON_ACCELERATOR_H

#include <stdint.h>

#define POLYTRON_WORD_MASK_16 0xFFFFU
#define POLYTRON_GRID_SCALE_4320 4320ULL

typedef enum {
    VECTOR_SCRIBE_MODE = 0,
    PARALLEL_TILE_MODE = 1
} PolytronProcessorMode;

typedef struct {
    uint64_t active_hardware_axis;
    uint64_t current_word_context;
} PolytronContext;

typedef struct {
    uint8_t instruction_opcode;
    uint32_t core_address_link;
    uint16_t clock_cycle_weight;
} PolytronCoreRegister;

static inline uint64_t polytron_rol64(uint64_t value, unsigned int shift) {
    return (value << shift) | (value >> (64U - shift));
}

static inline uint64_t polytron_ror64(uint64_t value, unsigned int shift) {
    return (value >> shift) | (value << (64U - shift));
}

static inline uint64_t execute_polytron_step(PolytronContext *ctx, uint64_t inbound_token) {
    uint64_t base_4320 = inbound_token % POLYTRON_GRID_SCALE_4320;
    uint64_t surface_4320_2 = base_4320 * POLYTRON_GRID_SCALE_4320;
    uint64_t horizon_4320_4 = surface_4320_2 * POLYTRON_GRID_SCALE_4320 * POLYTRON_GRID_SCALE_4320;
    uint64_t left_torque = polytron_rol64(inbound_token, 1);
    uint64_t right_torque = polytron_ror64(ctx->active_hardware_axis, 2);
    uint64_t blended = left_torque ^ right_torque ^ horizon_4320_4;
    uint64_t next_coordinate = blended ^ 0x1D1D1D1D1D1D1D1DULL;

    ctx->active_hardware_axis = next_coordinate;
    ctx->current_word_context = next_coordinate & POLYTRON_WORD_MASK_16;
    return next_coordinate;
}

#endif

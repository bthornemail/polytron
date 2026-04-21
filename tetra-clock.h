/*
 * TETRA-CLOCK — Constitutional Clock Tree Model
 *
 * Based on QEMU's clock framework, adapted for Tetragrammatron.
 * Models clock distribution in the constitutional mesh.
 *
 * Clock periods are stored as 2^-32 ns fractions.
 * 0 = disabled/gated.
 *
 * Hierarchy:
 *   FS (0x1C) — Base clock
 *   GS (0x1D) — Constitutional clock (CONSTITUTIONAL_C)
 *   RS (0x1E) — Reference clock
 *   US (0x1F) — Unit clock (active)
 */

#ifndef TETRA_CLOCK_H
#define TETRA_CLOCK_H

#include <stdint.h>
#include <stdlib.h>

/* -------------------------------------------------------------------------- */
/* EMBEDDED KERNEL (required for clock hashing)                              */
/* -------------------------------------------------------------------------- */

typedef uint16_t Pair;

#define cons(a,d) ((((a) & 0xFF) << 8) | ((d) & 0xFF))
#define car(p)    (((p) >> 8) & 0xFF)
#define cdr(p)    ((p) & 0xFF)

#define CONSTITUTIONAL_C  0x1D

static Pair rotl(Pair x, int n) {
    n &= 15;
    return (Pair)((x << n) | (x >> (16 - n)));
}

static Pair rotr(Pair x, int n) {
    n &= 15;
    return (Pair)((x >> n) | (x << (16 - n)));
}

static Pair K(Pair p, Pair C) {
    return rotl(p, 1) ^ rotl(p, 3) ^ rotr(p, 2) ^ C;
}

/* -------------------------------------------------------------------------- */
/* CLOCK LAYERS (ASCII separators)                                           */
/* -------------------------------------------------------------------------- */

typedef uint8_t ClockLayer;

#define CLOCK_FS  ((ClockLayer)0x1C)  /* File Separator — base */
#define CLOCK_GS  ((ClockLayer)0x1D)  /* Group Separator — constitutional */
#define CLOCK_RS  ((ClockLayer)0x1E)  /* Record Separator — reference */
#define CLOCK_US  ((ClockLayer)0x1F)  /* Unit Separator — active */

/* -------------------------------------------------------------------------- */
/* CLOCK PERIOD REPRESENTATION                                                */
/* -------------------------------------------------------------------------- */

/*
 * Clock period stored as Q32.32 fixed point (units of 2^-32 ns).
 * 0 means disabled/gated.
 */
typedef uint64_t ClockPeriod;

/* Convert ns to internal representation */
static ClockPeriod clock_ns_to_period(uint32_t ns) {
    return ((ClockPeriod)ns << 32);
}

/* Convert internal representation to ns */
static uint32_t clock_period_to_ns(ClockPeriod period) {
    return (uint32_t)(period >> 32);
}

/* Convert Hz to internal representation */
static ClockPeriod clock_hz_to_period(uint32_t hz) {
    if (hz == 0) return 0;
    /* period = 10^9 * 2^32 / hz */
    return ((ClockPeriod)1000000000ULL << 32) / hz;
}

/* Convert internal representation to Hz */
static uint32_t clock_period_to_hz(ClockPeriod period) {
    if (period == 0) return 0;
    return (uint32_t)(((ClockPeriod)1000000000ULL << 32) / period);
}

/* -------------------------------------------------------------------------- */
/* CLOCK EVENTS                                                              */
/* -------------------------------------------------------------------------- */

typedef enum {
    CLOCK_PRE_UPDATE,   /* Period about to change */
    CLOCK_UPDATE        /* Period has changed */
} ClockEvent;

/* -------------------------------------------------------------------------- */
/* CLOCK OBJECT                                                              */
/* -------------------------------------------------------------------------- */

typedef struct TetraClock {
    const char* name;       /* Clock name */
    ClockLayer layer;       /* FS/GS/RS/US */
    ClockPeriod period;     /* Current period (Q32.32) */
    uint32_t multiplier;    /* Clock multiplier */
    uint32_t divider;      /* Clock divider */
    struct TetraClock* source;  /* Connected source clock */
    void (*callback)(void* opaque, ClockEvent event);
    void* opaque;
} TetraClock;

/* -------------------------------------------------------------------------- */
/* CLOCK INITIALIZATION                                                      */
/* -------------------------------------------------------------------------- */

static TetraClock* clock_new(const char* name, ClockLayer layer) {
    TetraClock* clk = (TetraClock*)calloc(1, sizeof(TetraClock));
    clk->name = name;
    clk->layer = layer;
    clk->period = 0;  /* Disabled by default */
    clk->multiplier = 1;
    clk->divider = 1;
    return clk;
}

static void clock_free(TetraClock* clk) {
    if (clk->source) {
        clk->source = NULL;
    }
    free(clk);
}

/* -------------------------------------------------------------------------- */
/* CLOCK CALLBACK                                                            */
/* -------------------------------------------------------------------------- */

static void clock_set_callback(TetraClock* clk, 
                               void (*callback)(void*, ClockEvent), 
                               void* opaque) {
    clk->callback = callback;
    clk->opaque = opaque;
}

/* -------------------------------------------------------------------------- */
/* CLOCK PERIOD SET/GET                                                      */
/* -------------------------------------------------------------------------- */

static void clock_set_period(TetraClock* clk, ClockPeriod period) {
    if (clk->callback) {
        clk->callback(clk->opaque, CLOCK_PRE_UPDATE);
    }
    clk->period = period;
    if (clk->callback) {
        clk->callback(clk->opaque, CLOCK_UPDATE);
    }
}

static ClockPeriod clock_get_period(const TetraClock* clk) {
    return clk->period;
}

static void clock_set_ns(TetraClock* clk, uint32_t ns) {
    clock_set_period(clk, clock_ns_to_period(ns));
}

static uint32_t clock_get_ns(const TetraClock* clk) {
    return clock_period_to_ns(clk->period);
}

static void clock_set_hz(TetraClock* clk, uint32_t hz) {
    clock_set_period(clk, clock_hz_to_period(hz));
}

static uint32_t clock_get_hz(const TetraClock* clk) {
    return clock_period_to_hz(clk->period);
}

/* -------------------------------------------------------------------------- */
/* CLOCK MULTIPLIER/DIVIDER                                                  */
/* -------------------------------------------------------------------------- */

static void clock_set_mul_div(TetraClock* clk, uint32_t mul, uint32_t div) {
    clk->multiplier = mul;
    clk->divider = div;
}

/* Get effective period (with mul/div applied) */
static ClockPeriod clock_get_effective_period(const TetraClock* clk) {
    if (clk->period == 0) return 0;
    return (clk->period * clk->multiplier) / clk->divider;
}

/* -------------------------------------------------------------------------- */
/* CLOCK CONNECTIONS (propagation)                                           */
/* -------------------------------------------------------------------------- */

static void clock_set_source(TetraClock* clk, TetraClock* source) {
    clk->source = source;
    /* Immediately propagate if source has a period */
    if (source && source->period != 0) {
        clock_set_period(clk, clock_get_effective_period(source));
    }
}

/* Propagate period to all connected clocks */
static void clock_propagate(TetraClock* clk) {
    if (!clk || clk->period == 0) return;
    /* Propagation handled via source links */
}

/* -------------------------------------------------------------------------- */
/* CLOCK HASH (for SID computation)                                         */
/* -------------------------------------------------------------------------- */

/* Hash a clock state to produce a SID */
static Pair clock_hash(const TetraClock* clk) {
    Pair p = cons((uint8_t)clk->layer, (uint8_t)clock_period_to_ns(clk->period));
    return K(p, CONSTITUTIONAL_C);
}

/* -------------------------------------------------------------------------- */
/* PREDEFINED CONSTITUTIONAL CLOCKS                                          */
/* -------------------------------------------------------------------------- */

typedef struct {
    TetraClock fs;   /* File Separator — base clock */
    TetraClock gs;   /* Group Separator — constitutional */
    TetraClock rs;   /* Record Separator — reference */
    TetraClock us;   /* Unit Separator — active */
} TetraClockTree;

/* Initialize a complete clock tree */
static void clock_tree_init(TetraClockTree* tree) {
    tree->fs.name = "FS";
    tree->fs.layer = CLOCK_FS;
    tree->fs.period = 0;
    tree->fs.multiplier = 1;
    tree->fs.divider = 1;
    tree->fs.source = NULL;
    tree->fs.callback = NULL;
    tree->fs.opaque = NULL;

    tree->gs.name = "GS";
    tree->gs.layer = CLOCK_GS;
    tree->gs.period = 0;
    tree->gs.multiplier = 1;
    tree->gs.divider = 1;
    tree->gs.source = NULL;
    tree->gs.callback = NULL;
    tree->gs.opaque = NULL;

    tree->rs.name = "RS";
    tree->rs.layer = CLOCK_RS;
    tree->rs.period = 0;
    tree->rs.multiplier = 1;
    tree->rs.divider = 1;
    tree->rs.source = NULL;
    tree->rs.callback = NULL;
    tree->rs.opaque = NULL;

    tree->us.name = "US";
    tree->us.layer = CLOCK_US;
    tree->us.period = 0;
    tree->us.multiplier = 1;
    tree->us.divider = 1;
    tree->us.source = NULL;
    tree->us.callback = NULL;
    tree->us.opaque = NULL;
}

/* Connect clock tree layers: FS → GS → RS → US */
static void clock_tree_connect(TetraClockTree* tree) {
    clock_set_source(&tree->gs, &tree->fs);
    clock_set_source(&tree->rs, &tree->gs);
    clock_set_source(&tree->us, &tree->rs);
}

/* Get a clock by layer */
static TetraClock* clock_tree_get(TetraClockTree* tree, ClockLayer layer) {
    switch (layer) {
        case CLOCK_FS: return &tree->fs;
        case CLOCK_GS: return &tree->gs;
        case CLOCK_RS: return &tree->rs;
        case CLOCK_US: return &tree->us;
        default: return NULL;
    }
}

/* Compute combined SID from all clocks */
static Pair clock_tree_sid(const TetraClockTree* tree) {
    Pair sid = 0;
    sid = K(sid, clock_hash(&tree->fs));
    sid = K(sid, clock_hash(&tree->gs));
    sid = K(sid, clock_hash(&tree->rs));
    sid = K(sid, clock_hash(&tree->us));
    return sid;
}

/* -------------------------------------------------------------------------- */
/* CLOCK DEBUGGING                                                           */
/* -------------------------------------------------------------------------- */

static const char* clock_layer_name(ClockLayer layer) {
    switch (layer) {
        case CLOCK_FS: return "FS";
        case CLOCK_GS: return "GS";
        case CLOCK_RS: return "RS";
        case CLOCK_US: return "US";
        default: return "??";
    }
}

static void clock_dump(const TetraClock* clk) {
    printf("Clock %s (layer 0x%02X):\n", clk->name, clk->layer);
    printf("  period: %llu (0x%016llX)\n", 
           (unsigned long long)clk->period, 
           (unsigned long long)clk->period);
    printf("  ns: %u\n", clock_get_ns(clk));
    printf("  Hz: %u\n", clock_get_hz(clk));
    printf("  mul/div: %u/%u\n", clk->multiplier, clk->divider);
    printf("  effective Hz: %u\n", 
           clock_period_to_hz(clock_get_effective_period(clk)));
    if (clk->source) {
        printf("  source: %s\n", clk->source->name);
    }
}

static void clock_tree_dump(const TetraClockTree* tree) {
    printf("═══ CLOCK TREE ═══\n");
    clock_dump(&tree->fs);
    clock_dump(&tree->gs);
    clock_dump(&tree->rs);
    clock_dump(&tree->us);
    printf("Combined SID: 0x%04X\n", clock_tree_sid(tree));
    printf("══════════════════════\n");
}

#endif /* TETRA_CLOCK_H */

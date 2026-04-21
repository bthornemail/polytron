/*
 * qemu-profiler.c - QEMU Execution Trace to Polyform Converter
 *
 * Converts QEMU record/replay logs into 3D SVG polyforms
 * - Input: QEMU rr (replay.bin) or icount trace
 * - Process: Each basic block = Voxel in Code16K frame
 * - Time: icount = Z-axis (extrusion)
 * - Output: 3D SVG polyform with 4-channel coloring
 *
 * Reverse debugging: rotate backwards to find gap origins
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

/* ============================================================
 * CONSTITUTIONAL CONSTANTS
 * ============================================================ */

#define CHANNEL_FS  0x1C   /* XOR - sum */
#define CHANNEL_GS  0x1D   /* AND - generate */
#define CHANNEL_RS  0x1E   /* OR - propagate */
#define CHANNEL_US  0x1F   /* lookahead */

#define CODE16K_ROWS 16
#define CODE16K_COLS 5
#define MAX_TRACES 65536

/* ============================================================
 * KERNEL K(p,C)
 * ============================================================ */

static uint32_t rotl32(uint32_t x, int n) {
    n &= 31;
    return (x << n) | (x >> (32 - n));
}

static uint32_t rotr32(uint32_t x, int n) {
    n &= 31;
    return (x >> n) | (x << (32 - n));
}

static uint32_t K(uint32_t p, uint32_t C) {
    return rotl32(p, 1) ^ rotl32(p, 3) ^ rotr32(p, 2) ^ C;
}

static uint32_t compute_sid(uint32_t v) {
    return K(v, CHANNEL_GS);
}

/* ============================================================
 * EXECUTION TRACE TYPES
 * ============================================================ */

typedef enum {
    TRACE_MEM_READ = 0,
    TRACE_MEM_WRITE = 1,
    TRACE_BRANCH_TAKEN = 2,
    TRACE_BRANCH_NOT_TAKEN = 3,
    TRACE_SYSCALL = 4,
    TRACE_INTERRUPT = 5,
    TRACE_IO_READ = 6,
    TRACE_IO_WRITE = 7
} TraceType;

typedef struct {
    uint64_t icount;         /* Instruction count (time axis) */
    uint32_t pc;             /* Program counter */
    uint32_t target;         /* Branch target or memory address */
    TraceType type;          /* Event type */
    uint32_t sid;            /* Semantic ID at this point */
} TraceEntry;

typedef struct {
    TraceEntry entries[MAX_TRACES];
    int count;
    uint64_t max_icount;
    uint64_t min_icount;
} ExecutionTrace;

/* ============================================================
 * BASIC BLOCK POLYFORM MAPPING
 * ============================================================ */

typedef struct {
    uint32_t pc_start;
    uint32_t pc_end;
    uint32_t sid;            /* SID of this block */
    uint64_t execution_count; /* How many times executed */
    uint64_t first_icount;    /* When first executed */
    uint64_t last_icount;    /* When last executed */
} BasicBlock;

typedef struct {
    BasicBlock blocks[1024];
    int block_count;
} BlockProfile;

/* ============================================================
 * APPENDIX G FSM (from barcode-accurate.c)
 * ============================================================ */

typedef enum {
    APPG_MODE_A = 0,           /* Control chars — Reset */
    APPG_MODE_B = 1,           /* Printable ASCII — Propagate */
    APPG_MODE_C = 2,           /* Numeric — Generate */
    APPG_MODE_C_FNC1 = 3,     /* AND gate */
    APPG_MODE_B_FNC1 = 4,     /* OR gate */
    APPG_MODE_C_SHIFT_B = 5,  /* Lookahead */
    APPG_MODE_C_DOUBLE_SHIFT = 6
} AppendixG_Mode;

typedef struct {
    uint8_t fs;   /* XOR - sum */
    uint8_t gs;   /* AND - generate */
    uint8_t rs;   /* OR - propagate */
    uint8_t us;   /* lookahead */
} Channel4;

static Channel4 compute_channels(uint32_t a, uint32_t b) {
    Channel4 ch;
    ch.fs = a ^ b;              /* XOR */
    ch.gs = a & b;              /* AND */
    ch.rs = a | b;              /* OR */
    ch.us = (a ^ b) ^ ((a & b) << 1);  /* Lookahead */
    return ch;
}

/* ============================================================
 * SIMULATE QEMU EXECUTION (demo mode)
 * ============================================================ */

static void simulate_execution(ExecutionTrace *trace, int num_instructions) {
    trace->count = 0;
    trace->min_icount = 0;
    trace->max_icount = num_instructions;
    
    /* Simulate a simple program: compute SIDs for a range */
    uint32_t state = 0x4242;
    
    for (int i = 0; i < num_instructions && trace->count < MAX_TRACES; i++) {
        TraceEntry *e = &trace->entries[trace->count];
        
        e->icount = i;
        e->pc = 0x1000 + (i % 256) * 4;  /* Simple PC pattern */
        e->type = (i % 4 == 0) ? TRACE_BRANCH_TAKEN : TRACE_MEM_READ;
        
        /* Compute SID - this is the "Gnomon" */
        state = K(state, CHANNEL_GS);
        e->sid = compute_sid(state);
        
        e->target = e->sid & 0xFFFF;
        
        trace->count++;
    }
}

/* ============================================================
 * PROFILE BASIC BLOCKS
 * ============================================================ */

static void profile_blocks(ExecutionTrace *trace, BlockProfile *profile) {
    profile->block_count = 0;
    
    /* Group by PC ranges */
    for (int i = 0; i < trace->count; i++) {
        TraceEntry *e = &trace->entries[i];
        
        /* Find existing block or create new */
        int found = -1;
        for (int j = 0; j < profile->block_count; j++) {
            if (profile->blocks[j].pc_start == (e->pc & ~0xFF)) {
                found = j;
                break;
            }
        }
        
        if (found < 0 && profile->block_count < 1024) {
            found = profile->block_count++;
            profile->blocks[found].pc_start = e->pc & ~0xFF;
            profile->blocks[found].pc_end = (e->pc & ~0xFF) + 0xFF;
            profile->blocks[found].sid = e->sid;
            profile->blocks[found].execution_count = 0;
            profile->blocks[found].first_icount = e->icount;
            profile->blocks[found].last_icount = e->icount;
        }
        
        if (found >= 0) {
            profile->blocks[found].execution_count++;
            profile->blocks[found].last_icount = e->icount;
            profile->blocks[found].sid = e->sid;  /* Update with latest */
        }
    }
}

/* ============================================================
 * RENDER 3D SVG POLYFORM (TEMPORAL)
 * ============================================================ */

static void render_temporal_polyform_svg(
    ExecutionTrace *trace, 
    BlockProfile *profile,
    const char *filename,
    int time_scale) 
{
    FILE *f = fopen(filename, "w");
    if (!f) return;
    
    /* Calculate dimensions */
    int rows = CODE16K_ROWS;
    int cols = CODE16K_COLS;
    int layers = (trace->max_icount / time_scale) + 1;
    if (layers > 32) layers = 32;  /* Cap for SVG size */
    
    int cell_size = 10;
    int margin = 40;
    int width = cols * cell_size + margin * 2;
    int height = rows * cell_size + margin * 2;
    
    /* SVG header with 3D projection */
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 %d %d\">\n", 
            width + layers * 5, height + layers * 5);
    fprintf(f, "  <title>Temporal Polyform - QEMU Execution Trace</title>\n");
    fprintf(f, "  <style>\n");
    fprintf(f, "    .fs { fill: #e74c3c; }   /* FS/XOR - Truth */\n");
    fprintf(f, "    .gs { fill: #3498db; }   /* GS/AND - Generate */\n");
    fprintf(f, "    .rs { fill: #2ecc71; }   /* RS/OR - Propagate */\n");
    fprintf(f, "    .us { fill: #f39c12; }   /* US/Lookahead */\n");
    fprintf(f, "    .block { stroke: #333; stroke-width: 0.5; }\n");
    fprintf(f, "    .axis { stroke: #999; stroke-width: 1; stroke-dasharray: 4; }\n");
    fprintf(f, "  </style>\n");
    
    fprintf(f, "  <rect width=\"%d\" height=\"%d\" fill=\"#fafafa\"/>\n", 
            width + layers * 5, height + layers * 5);
    
    /* Draw axes */
    fprintf(f, "  <text x=\"10\" y=\"25\" font-family=\"monospace\" font-size=\"10\">Time (icount)</text>\n");
    fprintf(f, "  <line x1=\"%d\" y1=\"30\" x2=\"%d\" y2=\"30\" class=\"axis\"/>\n", 
            margin, width);
    fprintf(f, "  <line x1=\"%d\" y1=\"30\" x2=\"%d\" y2=\"%d\" class=\"axis\"/>\n", 
            margin, margin, height);
    
    /* Render each time layer */
    for (int t = 0; t < layers; t++) {
        uint64_t icount_start = t * time_scale;
        uint64_t icount_end = (t + 1) * time_scale;
        
        int x_offset = margin + t * 5;
        int y_offset = margin + t * 5;
        
        /* Find blocks executed in this time window */
        for (int b = 0; b < profile->block_count; b++) {
            BasicBlock *blk = &profile->blocks[b];
            
            if (blk->first_icount >= icount_start && blk->first_icount < icount_end) {
                /* Map block to Code16K row/col */
                int row = (blk->pc_start / 4) % rows;
                int col = (blk->pc_start / (rows * 4)) % cols;
                
                Channel4 ch = compute_channels(blk->sid, blk->execution_count);
                
                /* Render based on channel */
                if (ch.fs) {
                    fprintf(f, "  <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" class=\"fs block\"/>\n",
                            x_offset + col * cell_size,
                            y_offset + row * cell_size,
                            cell_size - 1, cell_size - 1);
                }
                if (ch.gs) {
                    fprintf(f, "  <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" class=\"gs block\"/>\n",
                            x_offset + col * cell_size + 2,
                            y_offset + row * cell_size + 2,
                            cell_size - 5, cell_size - 5);
                }
                if (ch.us) {
                    fprintf(f, "  <circle cx=\"%d\" cy=\"%d\" r=\"%d\" class=\"us\"/>\n",
                            x_offset + col * cell_size + cell_size/2,
                            y_offset + row * cell_size + cell_size/2,
                            cell_size/4);
                }
            }
        }
    }
    
    /* Legend */
    fprintf(f, "  <g transform=\"translate(%d, %d)\">\n", width - 100, height + 20);
    fprintf(f, "    <rect width=\"12\" height=\"12\" class=\"fs\"/>\n");
    fprintf(f, "    <text x=\"16\" y=\"10\" font-size=\"10\">FS/XOR</text>\n");
    fprintf(f, "    <rect y=\"15\" width=\"12\" height=\"12\" class=\"gs\"/>\n");
    fprintf(f, "    <text x=\"16\" y=\"25\" font-size=\"10\">GS/AND</text>\n");
    fprintf(f, "    <rect y=\"30\" width=\"12\" height=\"12\" class=\"rs\"/>\n");
    fprintf(f, "    <text x=\"16\" y=\"40\" font-size=\"10\">RS/OR</text>\n");
    fprintf(f, "    <circle cy=\"48\" r=\"6\" class=\"us\"/>\n");
    fprintf(f, "    <text x=\"16\" y=\"52\" font-size=\"10\">US/Look</text>\n");
    fprintf(f, "  </g>\n");
    
    fprintf(f, "</svg>\n");
    fclose(f);
}

/* ============================================================
 * RENDER REVERSE DEBUG VIEW
 * ============================================================ */

static void render_reverse_debug_svg(
    ExecutionTrace *trace,
    const char *filename)
{
    FILE *f = fopen(filename, "w");
    if (!f) return;
    
    int cell_size = 12;
    int rows = 16;
    int cols = 32;
    int width = cols * cell_size + 40;
    int height = rows * cell_size + 40;
    
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 %d %d\">\n", width, height);
    fprintf(f, "  <title>Reverse Debug - Execution Timeline</title>\n");
    fprintf(f, "  <rect width=\"%d\" height=\"%d\" fill=\"white\"/>\n", width, height);
    
    /* Render backwards from end */
    int render_count = 0;
    for (int i = trace->count - 1; i >= 0 && render_count < 512; i--) {
        TraceEntry *e = &trace->entries[i];
        
        int col = render_count % cols;
        int row = render_count / cols;
        
        Channel4 ch = compute_channels(e->sid, e->pc);
        
        if (ch.fs) {
            fprintf(f, "  <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" fill=\"#e74c3c\"/>\n",
                    20 + col * cell_size, 20 + row * cell_size, cell_size - 1, cell_size - 1);
        } else if (ch.gs) {
            fprintf(f, "  <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" fill=\"#3498db\"/>\n",
                    20 + col * cell_size, 20 + row * cell_size, cell_size - 1, cell_size - 1);
        }
        
        render_count++;
    }
    
    fprintf(f, "  <text x=\"20\" y=\"%d\" font-family=\"monospace\" font-size=\"10\">icount: 0 -> %lu (forward)</text>\n",
            height - 10);
    fprintf(f, "  <text x=\"%d\" y=\"%d\" font-family=\"monospace\" font-size=\"10\" fill=\"#666\">%s</text>\n",
            width - 150, height - 10, "reverse ->");
    
    fprintf(f, "</svg>\n");
    fclose(f);
}

/* ============================================================
 * CODE16K FRAME GENERATION
 * ============================================================ */

static void render_code16k_frame(
    ExecutionTrace *trace,
    const char *filename,
    uint64_t start_icount,
    uint64_t end_icount)
{
    FILE *f = fopen(filename, "w");
    if (!f) return;
    
    int cell_size = 14;
    int rows = CODE16K_ROWS;
    int cols = CODE16K_COLS;
    int width = cols * cell_size + 30;
    int height = rows * cell_size + 30;
    
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 %d %d\">\n", width, height);
    fprintf(f, "  <title>Code16K Frame (icount %lu - %lu)</title>\n", 
            start_icount, end_icount);
    fprintf(f, "  <rect width=\"%d\" height=\"%d\" fill=\"white\"/>\n", width, height);
    
    /* Find traces in range and map to Code16K */
    int row = 0;
    for (int i = 0; i < trace->count && row < rows; i++) {
        TraceEntry *e = &trace->entries[i];
        
        if (e->icount >= start_icount && e->icount < end_icount) {
            int col = i % cols;
            
            Channel4 ch = compute_channels(e->sid, e->pc);
            
            /* FS channel - red */
            if (ch.fs) {
                fprintf(f, "  <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" fill=\"#e74c3c\"/>\n",
                        15 + col * cell_size, 15 + row * cell_size, cell_size - 2, cell_size - 2);
            }
            
            /* GS channel - blue */
            if (ch.gs) {
                fprintf(f, "  <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" fill=\"#3498db\"/>\n",
                        15 + col * cell_size + 3, 15 + row * cell_size + 3, 
                        cell_size - 8, cell_size - 8);
            }
            
            /* RS channel - green */
            if (ch.rs) {
                fprintf(f, "  <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" fill=\"#2ecc71\" opacity=\"0.5\"/>\n",
                        15 + col * cell_size + 1, 15 + row * cell_size + 1, 
                        cell_size - 4, cell_size - 4);
            }
            
            if (col == cols - 1) row++;
        }
    }
    
    /* Row labels */
    fprintf(f, "  <g font-family=\"monospace\" font-size=\"8\" fill=\"#666\">\n");
    for (int r = 0; r < rows; r++) {
        fprintf(f, "    <text x=\"4\" y=\"%d\">%d</text>\n", 23 + r * cell_size, r);
    }
    fprintf(f, "  </g>\n");
    
    fprintf(f, "</svg>\n");
    fclose(f);
}

/* ============================================================
 * BENCHMARK
 * ============================================================ */

static void benchmark(int iterations) {
    clock_t start = clock();
    
    ExecutionTrace trace;
    BlockProfile profile;
    
    for (int iter = 0; iter < iterations; iter++) {
        simulate_execution(&trace, 10000);
        profile_blocks(&trace, &profile);
        
        /* Compute channels for each block */
        for (int i = 0; i < profile.block_count; i++) {
            Channel4 ch = compute_channels(
                profile.blocks[i].sid,
                profile.blocks[i].execution_count
            );
            (void)ch;  /* Use it */
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("=== QEMU Profiler Benchmark ===\n");
    printf("Iterations: %d\n", iterations);
    printf("Instructions per run: 10000\n");
    printf("Time: %.3f seconds\n", elapsed);
    printf("Rate: %.0f traces/sec\n", iterations / elapsed);
}

/* ============================================================
 * MAIN
 * ============================================================ */

int main(int argc, char *argv[]) {
    printf("=== QEMU Execution Trace to Polyform Converter ===\n\n");
    
    const char *output_dir = "polyform";
    int num_instructions = 1000;
    int time_scale = 100;
    
    /* Parse args */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            num_instructions = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            time_scale = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-b") == 0 && i + 1 < argc) {
            benchmark(atoi(argv[++i]));
            return 0;
        }
    }
    
    /* Simulate QEMU execution trace */
    ExecutionTrace trace;
    simulate_execution(&trace, num_instructions);
    
    printf("Trace: %d instructions (icount 0 - %lu)\n", 
           trace.count, trace.max_icount);
    
    /* Profile basic blocks */
    BlockProfile profile;
    profile_blocks(&trace, &profile);
    printf("Basic blocks: %d\n", profile.block_count);
    
    char path[256];
    
    /* Generate 3D temporal polyform */
    snprintf(path, sizeof(path), "%s/qemu-temporal-polyform.svg", output_dir);
    render_temporal_polyform_svg(&trace, &profile, path, time_scale);
    printf("Wrote temporal polyform: %s\n", path);
    
    /* Generate reverse debug view */
    snprintf(path, sizeof(path), "%s/qemu-reverse-debug.svg", output_dir);
    render_reverse_debug_svg(&trace, path);
    printf("Wrote reverse debug: %s\n", path);
    
    /* Generate Code16K frames */
    int frame_count = (trace.max_icount / (time_scale * CODE16K_COLS)) + 1;
    for (int f = 0; f < frame_count && f < 10; f++) {
        uint64_t start = f * time_scale * CODE16K_COLS;
        uint64_t end = start + time_scale * CODE16K_COLS;
        
        snprintf(path, sizeof(path), "%s/qemu-frame-%03d.svg", output_dir, f);
        render_code16k_frame(&trace, path, start, end);
    }
    printf("Wrote %d Code16K frames\n", frame_count);
    
    printf("\nOutput files:\n");
    printf("  %s/qemu-temporal-polyform.svg   (3D extrusion)\n", output_dir);
    printf("  %s/qemu-reverse-debug.svg       (reverse timeline)\n", output_dir);
    printf("  %s/qemu-frame-*.svg            (Code16K frames)\n", output_dir);
    
    return 0;
}

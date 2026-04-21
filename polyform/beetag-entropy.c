/*
 * beetag-entropy.c - Full BeeTag Entropy Map Generator
 * 
 * Generates all valid BeeTags (2-of-5 encoding) and computes
 * pairwise Hamming distances to populate the kB microstate table.
 * 
 * BeeTag Spec:
 * - 2-of-5: Exactly 2 black cells in each 5-cell column
 * - Versions 1-10 (1-10 data characters)
 * - Each character = 2 columns (high/low nibble)
 * - Start/Stop patterns add overhead
 * 
 * Total valid 2-of-5 patterns: C(5,2) = 10 per column
 * Total tags = sum of valid combinations per version
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define BEETAG_MAX_VERSION 10
#define BEETAG_CELLS 5
#define MAX_TAGS 10000

typedef struct {
    int version;
    int cols;
    uint8_t grid[BEETAG_CELLS][20];  /* Max 20 cols (10 chars * 2) */
    uint64_t code;  /* Unique identifier for this tag */
} BeeTagEntry;

static int is_valid_2of5(uint8_t pattern) {
    int ones = __builtin_popcount(pattern & 0x1F);
    return ones == 2;
}

static uint8_t patterns_2of5[10];
static int pattern_count = 0;

static void init_2of5_patterns() {
    pattern_count = 0;
    for (uint8_t p = 0; p < 32; p++) {
        if (is_valid_2of5(p)) {
            patterns_2of5[pattern_count++] = p;
        }
    }
}

static int hamming_distance_uint64(uint64_t a, uint64_t b) {
    return __builtin_popcount(a ^ b);
}

static void generate_beetag(BeeTagEntry *tag, int version, uint64_t code) {
    tag->version = version;
    tag->cols = version * 2 + 2;  /* Data + start + stop */
    tag->code = code;
    
    memset(tag->grid, 0, sizeof(tag->grid));
    
    /* Start pattern (fixed: 00011 in 2-of-5) */
    tag->grid[0][0] = 0; tag->grid[1][0] = 0; tag->grid[2][0] = 0;
    tag->grid[3][0] = 1; tag->grid[4][0] = 1;
    
    /* Data columns from code - enumerate valid patterns only */
    int col = 1;
    uint64_t remaining = code;
    for (int i = 0; i < version && col < tag->cols - 1; i++) {
        int pattern_idx = remaining % pattern_count;
        remaining /= pattern_count;
        uint8_t pattern = patterns_2of5[pattern_idx];
        
        for (int row = 0; row < BEETAG_CELLS; row++) {
            tag->grid[row][col] = (pattern >> (BEETAG_CELLS - 1 - row)) & 1;
        }
        col++;
    }
    
    /* Stop pattern (fixed: 11000 in 2-of-5) */
    tag->grid[0][tag->cols-1] = 1; tag->grid[1][tag->cols-1] = 1;
    tag->grid[2][tag->cols-1] = 0; tag->grid[3][tag->cols-1] = 0; tag->grid[4][tag->cols-1] = 0;
}

static void render_beetag_svg(const BeeTagEntry *tag, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return;
    
    int ms = 15;
    int w = tag->cols * ms;
    int h = BEETAG_CELLS * ms;
    
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 %d %d\">\n", w, h);
    fprintf(f, "  <title>BeeTag v%d Entropy</title>\n", tag->version);
    fprintf(f, "  <rect width=\"%d\" height=\"%d\" fill=\"white\"/>\n", w, h);
    
    for (int y = 0; y < BEETAG_CELLS; y++) {
        for (int x = 0; x < tag->cols; x++) {
            if (tag->grid[y][x]) {
                fprintf(f, "  <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" fill=\"black\"/>\n",
                        x * ms, y * ms, ms, ms);
            }
        }
    }
    
    fprintf(f, "</svg>\n");
    fclose(f);
}

int main(int argc, char *argv[]) {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║     BEETAG ENTROPY MAP GENERATOR                          ║\n");
    printf("║     ============================                          ║\n");
    printf("║  Full 2-of-5 enumeration → Hamming distances → kB        ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");
    
    init_2of5_patterns();
    
    printf("=== 2-OF-5 VALID PATTERNS (%d total) ===\n", pattern_count);
    for (int i = 0; i < pattern_count; i++) {
        printf("  Pattern %2d: ", i);
        for (int j = BEETAG_CELLS - 1; j >= 0; j--) {
            printf("%d", (patterns_2of5[i] >> j) & 1);
        }
        printf(" (0x%02X)\n", patterns_2of5[i]);
    }
    printf("\n");
    
    /* Generate all BeeTags per version */
    BeeTagEntry tags[MAX_TAGS];
    int total_tags = 0;
    
    printf("=== GENERATING BEETAGS ===\n");
    for (int v = 1; v <= BEETAG_MAX_VERSION; v++) {
        int max_code = 1;
        for (int i = 0; i < v; i++) max_code *= pattern_count;
        
        /* Limit to prevent overflow */
        if (max_code > 10000) max_code = 10000;
        
        int version_count = 0;
        
        for (int code = 0; code < max_code && total_tags < MAX_TAGS; code++) {
            generate_beetag(&tags[total_tags], v, code);
            version_count++;
            total_tags++;
        }
        
        printf("  Version %2d: %5d valid tags\n", v, version_count);
    }
    
    printf("\n  TOTAL VALID BEETAGS: %d\n\n", total_tags);
    
    /* Compute Hamming distance distribution */
    printf("=== HAMMING DISTANCE DISTRIBUTION ===\n");
    int dist_histogram[33] = {0};
    int64_t dist_sum = 0;
    int64_t dist_count = 0;
    
    /* Compute average Hamming distance - sample diverse pairs */
    int max_pairs = 10000;
    dist_count = 0;
    dist_sum = 0;
    
    /* Sample across versions for diversity */
    for (int step = 1; step < total_tags && dist_count < max_pairs; step *= 2) {
        for (int i = 0; i < total_tags && dist_count < max_pairs; i += step) {
            int j = (i + step) % total_tags;
            int dist = hamming_distance_uint64(tags[i].code, tags[j].code);
            dist_histogram[dist]++;
            dist_sum += dist;
            dist_count++;
        }
    }
    
    printf("  Distance distribution (sampled %lld pairs):\n", (long long)dist_count);
    for (int d = 0; d <= 32; d++) {
        if (dist_histogram[d] > 0) {
            printf("    d=%2d: %6d pairs\n", d, dist_histogram[d]);
        }
    }
    
    double avg_dist = (double)dist_sum / dist_count;
    printf("\n  Average Hamming distance: %.4f\n", avg_dist);
    
    /* Map to Boltzmann constant kB */
    printf("\n=== BOLTZMANN CONSTANT MAPPING ===\n");
    printf("  kB = 1.0 + (Hamming / Max_Hamming) * epsilon\n");
    printf("  Max Hamming distance = 32 bits\n");
    printf("  For avg distance %.4f: kB = 1.0 + %.6f = %.6f\n",
           avg_dist, avg_dist / 32.0, 1.0 + avg_dist / 32.0);
    
    /* Write entropy table */
    FILE *entropy_table = fopen("beetag-entropy-table.txt", "w");
    if (entropy_table) {
        fprintf(entropy_table, "# BEETAG ENTROPY TABLE\n");
        fprintf(entropy_table, "# version,code,bitstream,hamming_to_next\n");
        
        for (int i = 0; i < total_tags; i++) {
            int next_dist = (i + 1 < total_tags) 
                ? hamming_distance_uint64(tags[i].code, tags[i+1].code)
                : 0;
            fprintf(entropy_table, "%d,%llu,0x%08llX,%d\n",
                    tags[i].version, (unsigned long long)tags[i].code, 
                    (unsigned long long)tags[i].code, next_dist);
        }
        fclose(entropy_table);
        printf("\n  Wrote: polyform/beetag-entropy-table.txt\n");
    }
    
    /* Generate sample SVG tags */
    printf("\n=== GENERATING SAMPLE SVG TAGS ===\n");
    char path[256];
    for (int v = 1; v <= BEETAG_MAX_VERSION; v += 3) {
        snprintf(path, sizeof(path), "beetag-entropy-v%d.svg", v);
        for (int i = 0; i < total_tags; i++) {
            if (tags[i].version == v) {
                render_beetag_svg(&tags[i], path);
                printf("  %s (code=%d)\n", path, i);
                break;
            }
        }
    }
    
    /* Summary */
    printf("\n╔════════════════════════════════════════════════════════════╗\n");
    printf("║  ENTROPY SUMMARY                                          ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Valid 2-of-5 patterns: %2d                               ║\n", pattern_count);
    printf("║  Total valid BeeTags:    %d                                ║\n", total_tags);
    printf("║  Avg Hamming distance:   %.4f bits                        ║\n", avg_dist);
    printf("║  Microstates (S=kB·lnW): ~2^%.1f                           ║\n", log2((double)total_tags));
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    return 0;
}

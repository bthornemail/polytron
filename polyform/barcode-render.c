/*
 * barcode-render.c - Aztec, MaxiCode, and BeeTag barcode generator
 *
 * Generates barcodes from logical proof/Polyform data
 * Outputs: SVG barcodes for visual proof representation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define MAX_DATA 256

typedef enum {
    BARCODE_AZTEC,
    BARCODE_MAXI,
    BARCODE_BEETAG
} BarcodeType;

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
    return K(v, 0x1D);
}

typedef struct {
    uint8_t modules[64][64];
    int rows, cols;
} AztecGrid;

static void aztec_init(AztecGrid *az, int layers) {
    az->rows = layers * 2 + 1;
    az->cols = layers * 2 + 1;
    memset(az->modules, 0, sizeof(az->modules));
}

static void aztec_set_finder(AztecGrid *az, int cx, int cy, int size) {
    for (int y = cy - size; y <= cy + size; y++) {
        for (int x = cx - size; x <= cx + size; x++) {
            if (y >= 0 && y < az->rows && x >= 0 && x < az->cols) {
                int dx = abs(x - cx);
                int dy = abs(y - cy);
                if (dx == size || dy == size || (dx < 2 && dy < 2)) {
                    az->modules[y][x] = 1;
                } else if (dx <= 1 && dy <= 1) {
                    az->modules[y][x] = 0;
                } else if ((dx + dy) % 2 == 0) {
                    az->modules[y][x] = 1;
                }
            }
        }
    }
}

static void aztec_encode_data(AztecGrid *az, const uint8_t *data, int len) {
    int bit_idx = 0;
    int offset = 3;
    
    for (int i = 0; i < len && offset < az->rows - 3; i++) {
        for (int j = 0; j < 8 && offset + j < az->cols - 3; j++) {
            int bit = (data[i] >> (7 - j)) & 1;
            az->modules[offset + j/4][offset + j%4] = bit;
        }
        offset++;
    }
}

static void aztec_render_svg(AztecGrid *az, const char *filename, int module_size) {
    FILE *f = fopen(filename, "w");
    if (!f) return;
    
    int w = az->cols * module_size;
    int h = az->rows * module_size;
    
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 %d %d\">\n", w, h);
    fprintf(f, "  <rect width=\"%d\" height=\"%d\" fill=\"white\"/>\n", w, h);
    
    for (int y = 0; y < az->rows; y++) {
        for (int x = 0; x < az->cols; x++) {
            if (az->modules[y][x]) {
                fprintf(f, "  <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" fill=\"black\"/>\n",
                        x * module_size, y * module_size, module_size, module_size);
            }
        }
    }
    
    fprintf(f, "</svg>\n");
    fclose(f);
    printf("Wrote Aztec barcode to %s\n", filename);
}

typedef struct {
    uint8_t grid[33][33];
} MaxiCodeGrid;

static void maxi_init(MaxiCodeGrid *mc) {
    memset(mc->grid, 0, sizeof(mc->grid));
}

static void maxi_set_bullseye(MaxiCodeGrid *mc, int cx, int cy) {
    for (int y = cy - 5; y <= cy + 5; y++) {
        for (int x = cx - 5; x <= cx + 5; x++) {
            if (y >= 0 && y < 33 && x >= 0 && x < 33) {
                int d = abs(x - cx) + abs(y - cy);
                if (d <= 5) {
                    mc->grid[y][x] = (d % 2 == 0) ? 1 : 0;
                }
            }
        }
    }
}

static void maxi_set定位(MaxiCodeGrid *mc, int x, int y, int val) {
    if (x >= 0 && x < 33 && y >= 0 && y < 33) {
        mc->grid[y][x] = val;
    }
}

static void maxi_render_svg(MaxiCodeGrid *mc, const char *filename, int module_size) {
    FILE *f = fopen(filename, "w");
    if (!f) return;
    
    int w = 33 * module_size;
    int h = 33 * module_size;
    
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 %d %d\">\n", w, h);
    fprintf(f, "  <rect width=\"%d\" height=\"%d\" fill=\"white\"/>\n", w, h);
    
    for (int y = 0; y < 33; y++) {
        for (int x = 0; x < 33; x++) {
            if (mc->grid[y][x]) {
                fprintf(f, "  <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" fill=\"black\"/>\n",
                        x * module_size, y * module_size, module_size, module_size);
            }
        }
    }
    
    fprintf(f, "</svg>\n");
    fclose(f);
    printf("Wrote MaxiCode to %s\n", filename);
}

typedef struct {
    uint8_t grid[10][10];
    int version;
} BeeTagGrid;

static void beetag_init(BeeTagGrid *bt, int version) {
    bt->version = version;
    memset(bt->grid, 0, sizeof(bt->grid));
    
    int size = version + 1;
    bt->grid[0][0] = 1;
    bt->grid[0][size-1] = 1;
    bt->grid[size-1][0] = 1;
    bt->grid[size-1][size-1] = 1;
    
    for (int i = 1; i < size - 1; i++) {
        bt->grid[0][i] = 1;
        bt->grid[size-1][i] = 1;
        bt->grid[i][0] = 1;
        bt->grid[i][size-1] = 1;
    }
}

static void beetag_encode(BeeTagGrid *bt, const uint8_t *data, int len) {
    int size = bt->version + 1;
    int offset = 1;
    
    for (int i = 0; i < len && offset < size - 1; i++) {
        for (int j = 0; j < 8 && offset + j < size - 1; j++) {
            int bit = (data[i] >> (7 - j)) & 1;
            bt->grid[offset][offset + j] = bit;
        }
        offset++;
    }
}

static void beetag_render_svg(BeeTagGrid *bt, const char *filename, int module_size) {
    FILE *f = fopen(filename, "w");
    if (!f) return;
    
    int size = bt->version + 1;
    int w = size * module_size;
    int h = size * module_size;
    
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 %d %d\">\n", w, h);
    fprintf(f, "  <rect width=\"%d\" height=\"%d\" fill=\"white\"/>\n", w, h);
    fprintf(f, "  <text x=\"%d\" y=\"%d\" font-size=\"%d\" text-anchor=\"middle\">BeeTag %dx%d</text>\n",
            w/2, module_size, module_size*2/3, size, size);
    
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            if (bt->grid[y][x]) {
                fprintf(f, "  <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" fill=\"black\"/>\n",
                        x * module_size, y * module_size, module_size, module_size);
            }
        }
    }
    
    fprintf(f, "</svg>\n");
    fclose(f);
    printf("Wrote BeeTag to %s\n", filename);
}

void generate_from_wordnet_sid(uint32_t synset_id, const char *output_dir) {
    uint32_t sid = compute_sid(synset_id);
    uint8_t data[4] = {
        (sid >> 24) & 0xFF,
        (sid >> 16) & 0xFF,
        (sid >> 8) & 0xFF,
        sid & 0xFF
    };
    
    AztecGrid az;
    aztec_init(&az, 5);
    aztec_set_finder(&az, 5, 5, 4);
    aztec_set_finder(&az, az.cols-6, 5, 4);
    aztec_set_finder(&az, 5, az.rows-6, 4);
    aztec_encode_data(&az, data, 4);
    
    char path[256];
    snprintf(path, sizeof(path), "%s/aztec-%u.svg", output_dir, synset_id);
    aztec_render_svg(&az, path, 8);
    
    MaxiCodeGrid mc;
    maxi_init(&mc);
    maxi_set_bullseye(&mc, 16, 16);
    maxi_set定位(&mc, 3, 16, (sid >> 0) & 1);
    maxi_set定位(&mc, 5, 16, (sid >> 1) & 1);
    maxi_set定位(&mc, 7, 16, (sid >> 2) & 1);
    maxi_set定位(&mc, 9, 16, (sid >> 3) & 1);
    maxi_set定位(&mc, 11, 16, (sid >> 4) & 1);
    maxi_set定位(&mc, 13, 16, (sid >> 5) & 1);
    maxi_set定位(&mc, 15, 16, (sid >> 6) & 1);
    maxi_set定位(&mc, 17, 16, (sid >> 7) & 1);
    
    snprintf(path, sizeof(path), "%s/maxicode-%u.svg", output_dir, synset_id);
    maxi_render_svg(&mc, path, 10);
    
    BeeTagGrid bt;
    beetag_init(&bt, 5);
    beetag_encode(&bt, data, 4);
    
    snprintf(path, sizeof(path), "%s/beetag-%u.svg", output_dir, synset_id);
    beetag_render_svg(&bt, path, 20);
}

int main(int argc, char *argv[]) {
    printf("=== Barcode Generator (AZTEC, MaxiCode, BeeTag) ===\n\n");
    
    const char *output_dir = "polyform";
    
    printf("Generating barcodes from WordNet synset IDs...\n\n");
    
    uint32_t test_synsets[] = {
        100001740, 100001930, 100002137, 100002452,
        100002684, 100007846, 100017480, 100020890
    };
    
    for (int i = 0; i < sizeof(test_synsets)/sizeof(test_synsets[0]); i++) {
        uint32_t sid = compute_sid(test_synsets[i]);
        printf("Synset %u -> SID 0x%08X\n", test_synsets[i], sid);
        generate_from_wordnet_sid(test_synsets[i], output_dir);
    }
    
    printf("\nGenerated barcode files in %s/\n", output_dir);
    
    return 0;
}

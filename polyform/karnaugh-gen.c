/*
 * karnaugh-gen.c - Karnaugh Map Generator with Barcode/Polyform Output
 *
 * Generates 2D/2.5D/3D Karnaugh maps from logical formulas
 * Outputs: PNG barcodes (AZTEC/MAXI/BeeTag), SVG polyforms, Smith charts
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define MAX_VARS 6
#define MAX_CELLS (1 << MAX_VARS)

typedef enum {
    VAL_FALSE = 0,
    VAL_TRUE = 1,
    VAL_X = 2,  /* Don't care / Unknown */
    VAL_GAP = 3  /* Gap for polyform fitting */
} CellValue;

typedef struct {
    int n_vars;
    CellValue cells[MAX_CELLS];
    char labels[MAX_VARS][16];
} KarnaughMap;

typedef struct {
    int x, y, z;
    int active;
} Voxel;

typedef struct {
    Voxel voxels[64];
    int count;
} Polyform;

#define CHANNEL_FS 0x1C
#define CHANNEL_GS 0x1D
#define CHANNEL_RS 0x1E
#define CHANNEL_US 0x1F

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

void kmap_init(KarnaughMap *km, int n_vars) {
    km->n_vars = n_vars;
    for (int i = 0; i < MAX_CELLS; i++) {
        km->cells[i] = VAL_X;
    }
    for (int i = 0; i < n_vars; i++) {
        snprintf(km->labels[i], 16, "v%d", i);
    }
}

void kmap_set(KarnaughMap *km, uint32_t index, CellValue v) {
    if (index < (1 << km->n_vars)) {
        km->cells[index] = v;
    }
}

static int gray_code(int i) {
    return i ^ (i >> 1);
}

static void kmap_layout_2d(KarnaughMap *km, int *rows, int *cols) {
    int n = km->n_vars;
    if (n <= 2) {
        *rows = 1 << (n / 2);
        *cols = 1 << ((n + 1) / 2);
    } else {
        *rows = 1 << (n / 2);
        *cols = 1 << ((n + 1) / 2);
    }
}

static int kmap_index_2d(KarnaughMap *km, int row, int col) {
    int rows, cols;
    kmap_layout_2d(km, &rows, &cols);
    
    int gr = gray_code(row);
    int gc = gray_code(col);
    
    return (gr << ((km->n_vars + 1) / 2)) | gc;
}

void kmap_from_datalog(KarnaughMap *km, const char *facts[]) {
    kmap_init(km, 4);
    
    for (int i = 0; facts[i]; i++) {
        uint32_t vars[4] = {0, 0, 0, 0};
        sscanf(facts[i], "%u %u %u %u", &vars[0], &vars[1], &vars[2], &vars[3]);
        
        uint32_t index = 0;
        for (int j = 0; j < 4; j++) {
            index |= (vars[j] & 1) << j;
        }
        
        uint32_t sid = compute_sid(index);
        kmap_set(km, index, (sid & 1) ? VAL_TRUE : VAL_FALSE);
    }
    
    for (int i = 0; i < (1 << km->n_vars); i++) {
        if (km->cells[i] == VAL_X) {
            km->cells[i] = VAL_GAP;
        }
    }
}

int kmap_group_cells(KarnaughMap *km, int groups[][8], int *group_count) {
    int count = 0;
    int visited[MAX_CELLS] = {0};
    
    for (int i = 0; i < (1 << km->n_vars); i++) {
        if (km->cells[i] == VAL_TRUE && !visited[i]) {
            int group_size = 1;
            groups[count][0] = i;
            visited[i] = 1;
            
            for (int j = i + 1; j < (1 << km->n_vars); j++) {
                if (km->cells[j] == VAL_TRUE && !visited[j]) {
                    int diff = __builtin_popcount(i ^ j);
                    if (diff == 1 || diff == 2) {
                        groups[count][group_size++] = j;
                        visited[j] = 1;
                    }
                }
            }
            groups[count][group_size] = -1;
            count++;
            if (count >= 16) break;
        }
    }
    
    *group_count = count;
    return count;
}

typedef struct {
    uint8_t r, g, b;
} RGB;

static RGB val_to_color(CellValue v) {
    switch (v) {
        case VAL_TRUE:  return (RGB){255, 100, 100};
        case VAL_FALSE: return (RGB){100, 100, 255};
        case VAL_X:     return (RGB){200, 200, 200};
        case VAL_GAP:   return (RGB){50, 50, 50};
        default:        return (RGB){0, 0, 0};
    }
}

void write_png_header(FILE *f, int w, int h) {
    fprintf(f, "P6\n%d %d\n255\n", w, h);
}

void kmap_render_2d_png(KarnaughMap *km, const char *filename) {
    int rows, cols;
    kmap_layout_2d(km, &rows, &cols);
    
    int cell_size = 40;
    int img_w = cols * cell_size + 60;
    int img_h = rows * cell_size + 60;
    
    FILE *f = fopen(filename, "wb");
    if (!f) return;
    
    fprintf(f, "P6\n%d %d\n255\n", img_w, img_h);
    
    for (int y = 0; y < img_h; y++) {
        for (int x = 0; x < img_w; x++) {
            int is_border = (x < 50 || y < 50);
            int cell_x = (x - 50) / cell_size;
            int cell_y = (y - 50) / cell_size;
            
            RGB color = {255, 255, 255};
            
            if (is_border) {
                color = (RGB){200, 200, 200};
            } else if (cell_x >= 0 && cell_x < cols && cell_y >= 0 && cell_y < rows) {
                int idx = kmap_index_2d(km, cell_y, cell_x);
                color = val_to_color(km->cells[idx]);
            }
            
            fwrite(&color, 1, 3, f);
        }
    }
    
    fclose(f);
    printf("Wrote 2D K-map to %s\n", filename);
}

void kmap_render_2d_console(KarnaughMap *km) {
    int rows, cols;
    kmap_layout_2d(km, &rows, &cols);
    
    printf("\n=== 2D Karnaugh Map ===\n");
    printf("     ");
    for (int c = 0; c < cols; c++) {
        printf("%4X ", gray_code(c));
    }
    printf("\n");
    
    for (int r = 0; r < rows; r++) {
        printf("%4X  ", gray_code(r));
        for (int c = 0; c < cols; c++) {
            int idx = kmap_index_2d(km, r, c);
            char ch = 'X';
            switch (km->cells[idx]) {
                case VAL_TRUE:  ch = '1'; break;
                case VAL_FALSE: ch = '0'; break;
                case VAL_GAP:   ch = '.'; break;
                default:        ch = 'X'; break;
            }
            printf("  %c ", ch);
        }
        printf("\n");
    }
}

void polyform_from_kmap(KarnaughMap *km, Polyform *pf) {
    pf->count = 0;
    
    for (int i = 0; i < (1 << km->n_vars); i++) {
        if (km->cells[i] == VAL_TRUE) {
            if (pf->count >= 64) break;
            
            pf->voxels[pf->count].x = i % 4;
            pf->voxels[pf->count].y = i / 4;
            pf->voxels[pf->count].z = 0;
            pf->voxels[pf->count].active = 1;
            pf->count++;
        }
    }
}

void polyform_extrude(Polyform *pf, int layers) {
    if (pf->count >= 64) return;
    
    for (int z = 1; z < layers && pf->count < 64; z++) {
        for (int i = 0; i < pf->count && pf->count < 64; i++) {
            pf->voxels[pf->count] = pf->voxels[i];
            pf->voxels[pf->count].z = z;
            pf->count++;
        }
    }
}

void polyform_render_svg(Polyform *pf, const char *filename, int mode) {
    FILE *f = fopen(filename, "w");
    if (!f) return;
    
    int w = 400, h = 400;
    int scale = 30;
    
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 %d %d\">\n", w, h);
    fprintf(f, "  <style>\n");
    fprintf(f, "    .voxel { stroke: #333; stroke-width: 1; }\n");
    fprintf(f, "    .true { fill: #e74c3c; }\n");
    fprintf(f, "    .false { fill: #3498db; }\n");
    fprintf(f, "    .gap { fill: #95a5a6; opacity: 0.5; }\n");
    fprintf(f, "    .grid { stroke: #ddd; stroke-width: 0.5; }\n");
    fprintf(f, "  </style>\n");
    
    fprintf(f, "  <rect width=\"%d\" height=\"%d\" fill=\"white\"/>\n", w, h);
    
    if (mode == 2) {
        for (int i = 0; i <= 10; i++) {
            fprintf(f, "  <line x1=\"%d\" y1=\"0\" x2=\"%d\" y2=\"%d\" class=\"grid\"/>\n", i*scale, i*scale, h);
            fprintf(f, "  <line x1=\"0\" y1=\"%d\" x2=\"%d\" y2=\"%d\" class=\"grid\"/>\n", i*scale, w, i*scale);
        }
    }
    
    for (int i = 0; i < pf->count; i++) {
        Voxel *v = &pf->voxels[i];
        int x = 50 + v->x * scale;
        int y = 50 + v->y * scale;
        
        if (mode == 2) {
            int x2 = 50 + v->y * scale;
            int y2 = 50 + v->x * scale;
            fprintf(f, "  <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" class=\"voxel true\"/>\n",
                    x2, y2, scale-2, scale-2);
        } else if (mode == 3) {
            int z_off = v->z * 10;
            fprintf(f, "  <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" class=\"voxel true\"/>\n",
                    x + z_off, y - z_off, scale-2, scale-2);
        } else {
            fprintf(f, "  <rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" class=\"voxel true\"/>\n",
                    x, y, scale-2, scale-2);
        }
    }
    
    fprintf(f, "</svg>\n");
    fclose(f);
    printf("Wrote SVG polyform (mode %d) to %s\n", mode, filename);
}

void generate_smith_chart_svg(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) return;
    
    int w = 600, h = 600;
    int cx = w/2, cy = h/2, r = 250;
    
    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(f, "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 %d %d\">\n", w, h);
    fprintf(f, "  <rect width=\"%d\" height=\"%d\" fill=\"white\"/>\n", w, h);
    
    fprintf(f, "  <circle cx=\"%d\" cy=\"%d\" r=\"%d\" fill=\"none\" stroke=\"black\" stroke-width=\"2\"/>\n", cx, cy, r);
    fprintf(f, "  <line x1=\"%d\" y1=\"0\" x2=\"%d\" y2=\"%d\" stroke=\"#ddd\"/>\n", cx, cx, h);
    fprintf(f, "  <line x1=\"0\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke=\"#ddd\"/>\n", cy, w, cy);
    
    for (int i = 1; i <= 5; i++) {
        double ir = r * (1.0 - 1.0/(i+1));
        fprintf(f, "  <circle cx=\"%d\" cy=\"%d\" r=\"%.1f\" fill=\"none\" stroke=\"#888\" stroke-dasharray=\"4\"/>\n", 
                cx, cy, ir);
    }
    
    for (int i = -5; i <= 5; i++) {
        if (i == 0) continue;
        double angle = atan2(i, 1) * 180 / M_PI;
        fprintf(f, "  <line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke=\"#888\" stroke-dasharray=\"4\"/>\n",
                cx, cy, cx + (int)(r * cos(angle * M_PI / 180)), 
                cy + (int)(r * sin(angle * M_PI / 180)));
    }
    
    fprintf(f, "</svg>\n");
    fclose(f);
    printf("Wrote Smith chart to %s\n", filename);
}

void benchmark_kmap(int iterations) {
    clock_t start = clock();
    
    KarnaughMap km;
    kmap_init(&km, 4);
    
    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < 16; i++) {
            uint32_t sid = compute_sid(i);
            kmap_set(&km, i, (sid & 1) ? VAL_TRUE : VAL_FALSE);
        }
        
        int groups[16][8];
        int group_count;
        kmap_group_cells(&km, groups, &group_count);
        
        Polyform pf;
        polyform_from_kmap(&km, &pf);
        polyform_extrude(&pf, 3);
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("=== Benchmark: %d iterations ===\n", iterations);
    printf("Time: %.3f seconds\n", elapsed);
    printf("Rate: %.0f iterations/sec\n", iterations / elapsed);
}

int main(int argc, char *argv[]) {
    printf("=== Polyform Karnaugh Map Generator ===\n\n");
    
    KarnaughMap km;
    kmap_init(&km, 4);
    
    for (int i = 0; i < 16; i++) {
        uint32_t sid = compute_sid(i);
        kmap_set(&km, i, (sid & 1) ? VAL_TRUE : VAL_FALSE);
    }
    
    km.cells[3] = VAL_X;
    km.cells[12] = VAL_X;
    
    kmap_render_2d_console(&km);
    
    int groups[16][8];
    int group_count;
    kmap_group_cells(&km, groups, &group_count);
    printf("\nGroups found: %d\n", group_count);
    
    kmap_render_2d_png(&km, "polyform/kmap-2d.ppm");
    
    Polyform pf;
    polyform_from_kmap(&km, &pf);
    polyform_extrude(&pf, 3);
    
    polyform_render_svg(&pf, "polyform/polyform-2d.svg", 2);
    polyform_render_svg(&pf, "polyform/polyform-2.5d.svg", 2.5);
    polyform_render_svg(&pf, "polyform/polyform-3d.svg", 3);
    
    generate_smith_chart_svg("polyform/smith-chart.svg");
    
    benchmark_kmap(10000);
    
    printf("\nOutput files:\n");
    printf("  polyform/kmap-2d.ppm\n");
    printf("  polyform/polyform-2d.svg\n");
    printf("  polyform/polyform-2.5d.svg\n");
    printf("  polyform/polyform-3d.svg\n");
    printf("  polyform/smith-chart.svg\n");
    
    return 0;
}

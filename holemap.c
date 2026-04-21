/*
 * HOLE MAP VISUALIZER WITH VOID-SIDs
 * 
 * Renders topological singularities with their deterministic Void-SIDs.
 * Each hole gets a unique 16-bit identifier computed from its lattice position.
 *
 * Compile: gcc -o holemap holemap.c -lm
 * Run: ./holemap > holemap.svg
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef uint16_t Pair;

#define cons(a,d) ((((a) & 0xFF) << 8) | ((d) & 0xFF))
#define car(p)    (((p) >> 8) & 0xFF)
#define cdr(p)   ((p) & 0xFF)

#define CONSTITUTIONAL_C  0x1D
#define LATTICE_W 16
#define LATTICE_H 16
#define CELL 48

static uint16_t rotl(uint16_t x, int n) {
    n &= 15;
    return (x << n) | (x >> (16 - n));
}

static uint16_t rotr(uint16_t x, int n) {
    n &= 15;
    return (x >> n) | (x << (16 - n));
}

static Pair K(Pair p, Pair C) {
    return rotl(p, 1) ^ rotl(p, 3) ^ rotr(p, 2) ^ C;
}

static Pair hash7(Pair seed) {
    Pair h = seed;
    for (int i = 0; i < 7; i++) h = K(h, CONSTITUTIONAL_C);
    return h;
}

enum { PLANE_U, PLANE_B, PLANE_D, PLANE_H };
const char *plane_n[] = {"U", "B", "D", "H"};

typedef struct {
    int plane;
    int orient;
    int s_bit;
    int frame;
    int parity;
} LPoint;

static LPoint apply_cp(LPoint p, unsigned char cp) {
    LPoint r = p;
    switch (cp) {
        case 0x0C: case 0x0E: case 0x0F: case 0x28: case 0x29:
        case 0x5B: case 0x5D: case 0x7B: case 0x7D:
            r.s_bit ^= 1; r.parity ^= 1; break;
        case 0x08: case 0x15: case 0x1A: case 0x21: case 0x3A:
            r.parity ^= 1; break;
        case 0x09: case 0x31: case 0x5E: case 0x7C:
            r.orient = (r.orient + 1) & 3; break;
        case 0x0A: case 0x34:
            r.orient = (r.orient + 2) & 3; break;
        case 0x0B: case 0x25: case 0x36:
            r.orient = (r.orient + 3) & 3; break;
        case 0x1B: case 0x2B: case 0x2D: case 0x5C: case 0x7E:
            r.s_bit ^= 1; break;
        case 0x1F: case 0x23:
            r.orient = (r.orient + 1) & 3; r.s_bit ^= 1; break;
        case 0x24: r.s_bit ^= 1; r.parity ^= 1; break;
        case 0x26: r.orient = (r.orient + 1) & 3; break;
        case 0x2A: r.orient = (r.orient + 3) & 3; r.s_bit ^= 1; break;
        case 0x3C: case 0x3E: r.s_bit ^= 1; r.orient = (r.orient + 1) & 3; break;
        case 0x3F: r.parity ^= 1; r.s_bit ^= 1; break;
        case 0x40: r.s_bit ^= 1; r.orient = (r.orient + 3) & 3; break;
        default: break;
    }
    return r;
}

static Pair encode_point(LPoint p) {
    return ((p.plane & 3) << 6) | ((p.orient & 3) << 4) | ((p.s_bit & 1) << 3) | (p.frame & 7);
}

static uint8_t hash8(Pair p) {
    uint8_t h = ((p ^ (p >> 8) ^ (p >> 4)) * 17) & 0xFF;
    return (h ^ (h >> 3) ^ 0x55);
}

static int holonomy(LPoint p, char *ft) {
    Pair start = encode_point(p);
    uint8_t h = hash8(start);
    int is_hole = (h > 248);
    
    if (is_hole) {
        strcpy(ft, "HOLE");
        return 7;
    }
    if (h > 240) { strcpy(ft, "SING"); return 1; }
    if (h > 224) { strcpy(ft, "2CYC"); return 2; }
    if (h > 192) { strcpy(ft, "4CYC"); return 3; }
    if (h > 128) { strcpy(ft, "8CYC"); return 4; }
    strcpy(ft, "VALID"); return 0;
}

static Pair void_sid(LPoint p, int h) {
    Pair seed = cons((p.plane << 6) | (p.s_bit << 5) | (p.frame << 1) | (p.parity & 1),
                     (p.orient << 6) | (h & 0x3F));
    return hash7(seed);
}

int main(void) {
    typedef struct { LPoint p; int hole; int hol; Pair sid; char ft[16]; } Cell;
    Cell lat[LATTICE_H][LATTICE_W];
    int holes = 0, hdist[8] = {0};
    
    for (int y = 0; y < LATTICE_H; y++) {
        for (int x = 0; x < LATTICE_W; x++) {
            LPoint p = { .plane = (x / 4) & 3, .orient = x & 3,
                        .s_bit = (y / 8) & 1, .frame = y & 7,
                        .parity = (x + y + ((x/4)&1)) & 1 };
            lat[y][x].p = p;
            lat[y][x].hol = holonomy(p, lat[y][x].ft);
            lat[y][x].hole = (lat[y][x].hol != 0);
            if (lat[y][x].hole) {
                lat[y][x].sid = void_sid(p, lat[y][x].hol);
                holes++;
                hdist[lat[y][x].hol]++;
            } else {
                lat[y][x].sid = 0;
            }
        }
    }
    
    /* SVG */
    printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    printf("<svg width=\"%d\" height=\"%d\" viewBox=\"0 0 %d %d\" xmlns=\"http://www.w3.org/2000/svg\">\n",
           LATTICE_W * CELL + 280, LATTICE_H * CELL + 200,
           LATTICE_W * CELL + 280, LATTICE_H * CELL + 200);
    
    printf("  <defs>\n");
    printf("    <radialGradient id=\"g\"><stop offset=\"0%%\" stop-color=\"#00FF88\"/><stop offset=\"100%%\" stop-color=\"#004422\"/></radialGradient>\n");
    const char *hc[] = {"#FF4444","#FF6644","#FF8844","#FFAA44","#FFCC44","#FFEE44","#CCFF44","#88FF44"};
    for (int i = 0; i < 8; i++) {
        printf("    <radialGradient id=\"h%d\"><stop offset=\"0%%\" stop-color=\"%s\"/><stop offset=\"100%%\" stop-color=\"#220000\"/></radialGradient>\n", i, hc[i]);
    }
    printf("  </defs>\n");
    printf("  <rect width=\"100%%\" height=\"100%%\" fill=\"#0A0A12\"/>\n");
    printf("  <text x=\"%d\" y=\"35\" fill=\"#FFF\" font-size=\"20\" text-anchor=\"middle\" font-family=\"monospace\">VOID-SID HOLE MAP</text>\n",
           (LATTICE_W * CELL + 200) / 2);
    printf("  <text x=\"%d\" y=\"60\" fill=\"#888\" font-size=\"12\" text-anchor=\"middle\" font-family=\"monospace\">Lattice Gauge Theory</text>\n",
           (LATTICE_W * CELL + 200) / 2);
    
    int lx = LATTICE_W * CELL + 20;
    printf("  <text x=\"%d\" y=\"100\" fill=\"#FFF\" font-size=\"14\" font-family=\"monospace\">LEGEND</text>\n", lx);
    printf("  <rect x=\"%d\" y=\"115\" w=\"20\" h=\"20\" fill=\"url(#g)\" stroke=\"#0F8\"/>\n", lx);
    printf("  <text x=\"%d\" y=\"130\" fill=\"#0F8\" font-size=\"12\">VALID</text>\n", lx + 30);
    for (int i = 1; i < 8; i++) if (hdist[i]) {
        printf("  <rect x=\"%d\" y=\"%d\" w=\"20\" h=\"20\" fill=\"url(#h%d)\" stroke=\"#F64\"/>\n", lx, 145 + i * 22, i);
        printf("  <text x=\"%d\" y=\"%d\" fill=\"#FAA\" font-size=\"12\">HOLO=%d (%d)</text>\n", lx + 30, 160 + i * 22, i, hdist[i]);
    }
    
    /* Cells */
    for (int y = 0; y < LATTICE_H; y++) {
        for (int x = 0; x < LATTICE_W; x++) {
            int px = 50 + x * CELL, py = 90 + y * CELL;
            if (lat[y][x].hole) {
                int h = lat[y][x].hol & 7;
                if (h == 0) h = 1;
                printf("  <rect x=\"%d\" y=\"%d\" w=\"%d\" h=\"%d\" fill=\"url(#h%d)\" stroke=\"#F66\" stroke-width=\"2\"/>\n",
                       px, py, CELL-2, CELL-2, h);
                printf("  <text x=\"%d\" y=\"%d\" fill=\"#FFF\" font-size=\"12\" text-anchor=\"middle\" font-weight=\"bold\">∅</text>\n",
                       px + CELL/2 - 1, py + 16);
                printf("  <text x=\"%d\" y=\"%d\" fill=\"#FAA\" font-size=\"9\" text-anchor=\"middle\">0x%04X</text>\n",
                       px + CELL/2 - 1, py + 30, lat[y][x].sid);
                printf("  <text x=\"%d\" y=\"%d\" fill=\"#D88\" font-size=\"7\" text-anchor=\"middle\">%s</text>\n",
                       px + CELL/2 - 1, py + 42, lat[y][x].ft);
            } else {
                printf("  <rect x=\"%d\" y=\"%d\" w=\"%d\" h=\"%d\" fill=\"url(#g)\" stroke=\"#6FA\" stroke-width=\"1\"/>\n",
                       px, py, CELL-2, CELL-2);
                printf("  <text x=\"%d\" y=\"%d\" fill=\"#DFC\" font-size=\"10\" text-anchor=\"middle\">%s%d</text>\n",
                       px + CELL/2 - 1, py + 14, plane_n[lat[y][x].p.plane], lat[y][x].p.s_bit);
                printf("  <text x=\"%d\" y=\"%d\" fill=\"#BAC\" font-size=\"8\" text-anchor=\"middle\">o=%d f=%d</text>\n",
                       px + CELL/2 - 1, py + 26, lat[y][x].p.orient, lat[y][x].p.frame);
            }
        }
    }
    
    printf("  <text x=\"25\" y=\"%d\" fill=\"#AAA\" font-size=\"11\" transform=\"rotate(-90,25,%d)\">FRAME×S</text>\n",
           (LATTICE_H * CELL + 180) / 2, (LATTICE_H * CELL + 180) / 2);
    printf("  <text x=\"%d\" y=\"%d\" fill=\"#AAA\" font-size=\"11\" text-anchor=\"middle\">PLANE×ORIENT</text>\n",
           (LATTICE_W * CELL + 200) / 2, LATTICE_H * CELL + 175);
    printf("</svg>\n");
    
    fprintf(stderr, "VOID-SID HOLE MAP: %d×%d lattice, %d holes (%.1f%%)\n",
            LATTICE_W, LATTICE_H, holes, 100.0 * holes / (LATTICE_W * LATTICE_H));
    for (int i = 1; i < 8; i++) if (hdist[i])
        fprintf(stderr, "  Holonomy %d: %d points\n", i, hdist[i]);
    
    return 0;
}
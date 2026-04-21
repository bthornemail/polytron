/*
 * TETRA-WRITER — Guest writer for IVSHMEM framebuffer
 * 
 * Opens /dev/tetra_fb and writes constitutional palette colors.
 * Each VM writes to its own tile in the shared framebuffer.
 * 
 * COMPILE:
 *   gcc -o tetra-writer tetra-writer.c
 * 
 * CROSS-COMPILE for RISC-V:
 *   riscv64-linux-gnu-gcc -o tetra-writer tetra-writer.c
 * 
 * RUN:
 *   ./tetra-writer [vm_id] [color_index]
 *   ./tetra-writer 0 1   # VM0: constitutional gray
 *   ./tetra-writer 1 2   # VM1: red
 *   ./tetra-writer 2 4   # VM2: blue
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>

/* -------------------------------------------------------------------------- */
/* CONFIG                                                                      */
/* -------------------------------------------------------------------------- */

#define FB_WIDTH     512
#define FB_HEIGHT    512
#define FB_SIZE      (FB_WIDTH * FB_HEIGHT * 4)
#define TILE_SIZE    128
#define DEV_PATH     "/dev/tetra_fb"

/* -------------------------------------------------------------------------- */
/* EMBEDDED KERNEL                                                            */
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

static Pair compute_sid(Pair nf) {
    return K(nf, CONSTITUTIONAL_C);
}

/* -------------------------------------------------------------------------- */
/* CONSTITUTIONAL PALETTE (BGRA for little-endian)                           */
/* -------------------------------------------------------------------------- */

static const uint32_t PALETTE[16] = {
    0xFF000000,  /* 0: FS - black */
    0xFF1D1D1D,  /* 1: GS - constitutional gray */
    0xFF0000FF,  /* 2: RS - red */
    0xFF00FF00,  /* 3: US - green */
    0xFFFF0000,  /* 4: p - blue */
    0xFF00FFFF,  /* 5: q - yellow */
    0xFFFF00FF,  /* 6: r - magenta */
    0xFFFFFF00,  /* 7: s - cyan */
    0xFF808080,  /* 8: gray */
    0xFF0000C0,  /* 9: dark red */
    0xFF00C000,  /* 10: dark green */
    0xFFC00000,  /* 11: dark blue */
    0xFF000080,  /* 12: maroon */
    0xFF008000,  /* 13: dark green */
    0xFF800000,  /* 14: navy */
    0xFF008080   /* 15: olive */
};

/* -------------------------------------------------------------------------- */
/* MAIN                                                                       */
/* -------------------------------------------------------------------------- */

int main(int argc, char** argv) {
    int vm_id = 0;
    int color_idx = 1;  /* Default: constitutional gray */
    
    if (argc > 1) vm_id = atoi(argv[1]);
    if (argc > 2) color_idx = atoi(argv[2]);
    
    printf("═══ TETRA-WRITER ═══\n");
    printf("VM ID: %d\n", vm_id);
    printf("Color index: %d\n", color_idx);
    
    /* Open the framebuffer device */
    int fd = open(DEV_PATH, O_RDWR);
    if (fd < 0) {
        /* Try /dev/mem as fallback */
        fd = open("/dev/mem", O_RDWR);
        if (fd < 0) {
            perror("open " DEV_PATH);
            return 1;
        }
    }
    
    /* Map the framebuffer */
    uint32_t* fb = mmap(NULL, FB_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (fb == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }
    
    printf("Framebuffer mapped at %p\n", (void*)fb);
    
    /* Calculate tile position */
    int tiles_per_row = FB_WIDTH / TILE_SIZE;
    int tile_x = (vm_id % tiles_per_row) * TILE_SIZE;
    int tile_y = (vm_id / tiles_per_row) * TILE_SIZE;
    
    /* Clamp to framebuffer bounds */
    if (tile_x >= FB_WIDTH) tile_x = 0;
    if (tile_y >= FB_HEIGHT) tile_y = 0;
    
    printf("Tile position: (%d, %d)\n", tile_x, tile_y);
    
    /* Get color from palette */
    uint32_t color = PALETTE[color_idx & 0xF];
    printf("Color: 0x%08X\n", color);
    
    /* Compute SID for this VM */
    Pair sid = compute_sid(cons((uint8_t)vm_id, (uint8_t)color_idx));
    printf("SID: 0x%04X\n", sid);
    
    /* Fill the tile with the constitutional color */
    for (int y = 0; y < TILE_SIZE; y++) {
        for (int x = 0; x < TILE_SIZE; x++) {
            int idx = (tile_y + y) * FB_WIDTH + (tile_x + x);
            fb[idx] = color;
        }
    }
    
    /* Write SID in the first pixel of the tile */
    fb[tile_y * FB_WIDTH + tile_x] = 0xFF000000 | (sid << 8) | (sid >> 8);
    
    /* Write VM metadata in second row */
    fb[(tile_y + 1) * FB_WIDTH + tile_x] = (uint32_t)vm_id << 24 | (uint32_t)color_idx << 16 | sid;
    
    printf("Wrote tile with SID 0x%04X\n", sid);
    
    /* Sync to ensure writes are visible */
    msync(fb, FB_SIZE, MS_SYNC);
    
    munmap(fb, FB_SIZE);
    close(fd);
    
    printf("═══ DONE ═══\n");
    return 0;
}

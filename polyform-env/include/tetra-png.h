/*
 * tetra-png.h — PNG as Constitutional Base Object
 * 
 * PNG is the primitive substrate for polyforms.
 * Each pixel is a continuation brick.
 * Each row is a GNOMON step.
 * Each pass is an Omicron mode (Adam7).
 */

#ifndef TETRA_PNG_H
#define TETRA_PNG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include "polyform.h"

/* --------------------------------------------------------------------------
 * PNG SIGNATURE AS CONSTITUTIONAL CONSTANT
 * -------------------------------------------------------------------------- */

#define PNG_SIGNATURE_BYTES 8
static const png_byte PNG_SIG[PNG_SIGNATURE_BYTES] = 
    {137, 80, 78, 71, 13, 10, 26, 10};

/* The 0x1D (GS) appears at offset 4 of the PNG signature! */
#define PNG_SIG_GS_OFFSET 4
#define PNG_SIG_GS_VALUE 0x1DU

/* --------------------------------------------------------------------------
 * HEX CONVERSION
 * -------------------------------------------------------------------------- */

static void sid_to_hex(Pair sid, char* out) {
    out[0] = "0123456789ABCDEF"[(sid >> 12) & 0xF];
    out[1] = "0123456789ABCDEF"[(sid >> 8) & 0xF];
    out[2] = "0123456789ABCDEF"[(sid >> 4) & 0xF];
    out[3] = "0123456789ABCDEF"[sid & 0xF];
    out[4] = '\0';
}

static Pair hex_to_sid(const char* hex) {
    Pair sid = 0;
    for (int i = 0; i < 4 && hex[i]; i++) {
        char c = hex[i];
        if (c >= '0' && c <= '9') sid = (sid << 4) | (c - '0');
        else if (c >= 'A' && c <= 'F') sid = (sid << 4) | (c - 'A' + 10);
        else if (c >= 'a' && c <= 'f') sid = (sid << 4) | (c - 'a' + 10);
    }
    return sid;
}

/* --------------------------------------------------------------------------
 * CONSTITUTIONAL COLOR PALETTE
 * -------------------------------------------------------------------------- */

#define CONSTITUTIONAL_COLORS 16

static const png_color constitutional_palette[CONSTITUTIONAL_COLORS] = {
    {0x00, 0x00, 0x00},  /* 0: FS - black */
    {0x1D, 0x1D, 0x1D},  /* 1: GS - constitutional gray */
    {0xFF, 0x00, 0x00},  /* 2: RS - red */
    {0x00, 0xFF, 0x00},  /* 3: US - green */
    {0x00, 0x00, 0xFF},  /* 4: p - blue */
    {0xFF, 0xFF, 0x00},  /* 5: q - yellow */
    {0xFF, 0x00, 0xFF},  /* 6: r - magenta */
    {0x00, 0xFF, 0xFF},  /* 7: s - cyan */
    {0x80, 0x80, 0x80},  /* 8: gray */
    {0xC0, 0x00, 0x00},  /* 9: dark red */
    {0x00, 0xC0, 0x00},  /* 10: dark green */
    {0x00, 0x00, 0xC0},  /* 11: dark blue */
    {0x80, 0x00, 0x00},  /* 12: maroon */
    {0x00, 0x80, 0x00},  /* 13: dark green */
    {0x00, 0x00, 0x80},  /* 14: navy */
    {0x80, 0x80, 0x00}   /* 15: olive */
};

static const png_byte constitutional_alpha[CONSTITUTIONAL_COLORS] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/* --------------------------------------------------------------------------
 * WRITE PNG
 * -------------------------------------------------------------------------- */

static int polyform_write_png(const Polyform2D* poly, const char* filename) {
    FILE* fp = fopen(filename, "wb");
    if (!fp) return -1;
    
    png_structp png_ptr = png_create_write_struct(
        PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fclose(fp);
        return -1;
    }
    
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_write_struct(&png_ptr, NULL);
        fclose(fp);
        return -1;
    }
    
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_write_struct(&png_ptr, &info_ptr);
        fclose(fp);
        return -1;
    }
    
    png_init_io(png_ptr, fp);
    
    /* IHDR: width = degree * 2, height = degree */
    int width = poly->degree * 2;
    int height = poly->degree;
    
    png_set_IHDR(png_ptr, info_ptr, width, height,
                 8,                      /* bit depth */
                 PNG_COLOR_TYPE_PALETTE, /* indexed color */
                 PNG_INTERLACE_ADAM7,   /* Omicron mode! */
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
    
    /* PLTE: constitutional palette */
    png_set_PLTE(png_ptr, info_ptr, constitutional_palette, CONSTITUTIONAL_COLORS);
    
    /* tRNS: alpha channel (s-bit) */
    png_set_tRNS(png_ptr, info_ptr, constitutional_alpha, CONSTITUTIONAL_COLORS, NULL);
    
    /* tEXt: constitutional metadata */
    png_text texts[4];
    char sid_hex[8];
    sid_to_hex(poly->polyform_sid, sid_hex);
    
    texts[0].compression = PNG_TEXT_COMPRESSION_NONE;
    texts[0].key = "Tetragrammatron";
    texts[0].text = "Constitutional Polyform";
    
    texts[1].compression = PNG_TEXT_COMPRESSION_NONE;
    texts[1].key = "Dimension";
    texts[1].text = "2D";
    
    texts[2].compression = PNG_TEXT_COMPRESSION_NONE;
    texts[2].key = "Degree";
    char deg_str[8];
    sprintf(deg_str, "%d", poly->degree);
    texts[2].text = deg_str;
    
    texts[3].compression = PNG_TEXT_COMPRESSION_NONE;
    texts[3].key = "SID";
    texts[3].text = sid_hex;
    
    png_set_text(png_ptr, info_ptr, texts, 4);
    
    png_write_info(png_ptr, info_ptr);
    
    /* Write rows — each row is a GNOMON step */
    png_bytep row = (png_bytep)png_malloc(png_ptr, width);
    
    for (int y = 0; y < height; y++) {
        row[0] = PNG_FILTER_NONE;  /* Filter byte */
        memset(row + 1, 0, width - 1);
        
        for (int x = 1; x < width; x++) {
            int cell_idx = y;
            if (cell_idx < poly->count) {
                /* Color index from channel extraction */
                uint8_t fs = extract_fs_channel(poly);
                uint8_t gs = extract_gs_channel(poly);
                uint8_t rs = extract_rs_channel(poly);
                uint8_t us = y & 1;
                
                row[x] = ((fs & 1) << 0) | ((gs & 1) << 1) | 
                         ((rs & 1) << 2) | ((y & 1) << 3);
                if (row[x] > 7) row[x] = 1;
            } else {
                row[x] = 1;  /* Pad with IDAT marker */
            }
        }
        png_write_row(png_ptr, row);
    }
    
    png_free(png_ptr, row);
    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);
    
    return 0;
}

/* --------------------------------------------------------------------------
 * READ PNG
 * -------------------------------------------------------------------------- */

static Polyform2D* polyform_read_png(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) return NULL;
    
    /* Verify PNG signature */
    png_byte header[8];
    fread(header, 1, 8, fp);
    
    /* Check for constitutional constant 0x1D at offset 4 */
    if (header[PNG_SIG_GS_OFFSET] != PNG_SIG_GS_VALUE) {
        fclose(fp);
        return NULL;
    }
    
    png_structp png_ptr = png_create_read_struct(
        PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    
    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return NULL;
    }
    
    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);
    
    int width = png_get_image_width(png_ptr, info_ptr);
    int height = png_get_image_height(png_ptr, info_ptr);
    
    /* Allocate polyform */
    Polyform2D* poly = (Polyform2D*)malloc(sizeof(Polyform2D));
    *poly = make_polyomino(height, 0x4242);
    
    /* Read rows */
    png_bytep row = (png_bytep)png_malloc(png_ptr, width);
    
    for (int y = 0; y < height; y++) {
        png_read_row(png_ptr, row, NULL);
        
        for (int x = 0; x < width && y < poly->count; x++) {
            if (x < poly->count) {
                uint8_t color_idx = row[x];
                poly->cells[y].cell_sid = cons(color_idx, y);
            }
        }
    }
    
    png_free(png_ptr, row);
    
    /* Extract SID from text */
    png_textp text_ptr;
    int num_text = png_get_text(png_ptr, info_ptr, &text_ptr, NULL);
    for (int i = 0; i < num_text; i++) {
        if (strcmp(text_ptr[i].key, "SID") == 0) {
            poly->polyform_sid = hex_to_sid(text_ptr[i].text);
        }
    }
    
    png_read_end(png_ptr, info_ptr);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
    
    return poly;
}

/* --------------------------------------------------------------------------
 * DALÍ CROSS (Octacube - GNOMON fixed point in 3D)
 * -------------------------------------------------------------------------- */

static Polycube3D make_dali_cross(void) {
    /* Dalí cross = 8 cubes arranged as a cross */
    Polycube3D cross;
    cross.voxels = (Voxel3D*)malloc(8 * sizeof(Voxel3D));
    cross.count = 0;
    cross.degree = 8;
    cross.polycube_sid = compute_sid(0x1D1D);
    
    /* 4 vertical cubes */
    Voxel3D v[8] = {
        {0, 0, 0, 0, 0},  /* center */
        {0, 1, 0, 0, 0},  /* top */
        {0, -1, 0, 0, 0}, /* bottom */
        {1, 0, 0, 0, 0},   /* right */
        {-1, 0, 0, 0, 0},  /* left */
        {0, 0, 1, 0, 0},   /* front */
        {0, 0, -1, 0, 0},  /* back */
        {1, 1, 1, 0, 0}    /* corner */
    };
    
    for (int i = 0; i < 8; i++) {
        cross.voxels[i] = v[i];
        cross.voxels[i].voxel_sid = K(cross.polycube_sid, cons(i, 0));
        cross.count++;
        cross.polycube_sid = K(cross.polycube_sid, cross.voxels[i].voxel_sid);
    }
    
    return cross;
}

/* --------------------------------------------------------------------------
 * WRITE DALÍ CROSS AS PNG LAYERS
 * -------------------------------------------------------------------------- */

static int dali_cross_write_png(const Polycube3D* cross, const char* basename) {
    /* Write one PNG per Z-layer (4 layers for 3D) */
    for (int z = -1; z <= 2; z++) {
        char filename[256];
        sprintf(filename, "%s_z%d.png", basename, z + 1);
        
        /* Extract 2D slice at this Z */
        Polyform2D slice = {0};
        slice.cells = NULL;
        slice.count = 0;
        
        for (int i = 0; i < cross->count; i++) {
            if (cross->voxels[i].z == z) {
                slice.cells = (Cell2D*)realloc(slice.cells, (slice.count + 1) * sizeof(Cell2D));
                slice.cells[slice.count].x = cross->voxels[i].x + 2;
                slice.cells[slice.count].y = cross->voxels[i].y + 2;
                slice.cells[slice.count].cell_sid = cross->voxels[i].voxel_sid;
                slice.count++;
            }
        }
        
        slice.degree = slice.count;
        slice.basis = BASIS_CUBE;
        slice.polyform_sid = cross->polycube_sid;
        
        if (polyform_write_png(&slice, filename) != 0) {
            free(slice.cells);
            return -1;
        }
        
        free(slice.cells);
    }
    
    return 0;
}

#endif /* TETRA_PNG_H */
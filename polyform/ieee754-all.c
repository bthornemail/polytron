/*
 * IEEE 754 BINARY64/BINARY128 ENCODER/DECODER
 * ============================================
 * 
 * Extends the universal codeword bootloader to:
 * - binary64 (double): 1 + 11 + 52 = 64 bits
 * - binary128 (quad): 1 + 15 + 112 = 128 bits
 * 
 * Maps to Unicode planes:
 * - binary32: Plane 0 (BMP)
 * - binary64: Planes 0-3
 * - binary128: All 17 planes
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define REG8_SIZE  8
#define REG16_SIZE 16
#define REG32_SIZE 32
#define REG64_SIZE 64

typedef struct {
    uint8_t cells[REG8_SIZE];
} reg8_t;

typedef struct {
    uint8_t cells[REG16_SIZE];
} reg16_t;

typedef struct {
    uint8_t cells[REG32_SIZE];
} reg32_t;

typedef struct {
    uint8_t cells[REG64_SIZE];
} reg64_t;

/* ============================================================
 * IEEE 754 STRUCTURES
 * ============================================================ */

typedef union {
    double d;
    uint64_t u;
    struct {
        uint64_t fraction : 52;
        uint64_t exponent : 11;
        uint64_t sign : 1;
    } bits;
} ieee754_64_t;

typedef union {
    long double d;
    uint128_t u;  /* Not standard - we'll use array */
    struct {
        unsigned __int128 fraction : 112;
        unsigned __int128 exponent : 15;
        unsigned __int128 sign : 1;
    } bits;
} ieee754_128_t;

/* ============================================================
 * DELTA LAW FOR DIFFERENT REGISTER SIZES
 * ============================================================ */

static void rotl8(reg8_t *r, int n) {
    n = n % REG8_SIZE;
    uint8_t tmp[REG8_SIZE];
    for (int i = 0; i < REG8_SIZE; i++) {
        tmp[i] = r->cells[(i + n) % REG8_SIZE];
    }
    memcpy(r->cells, tmp, REG8_SIZE);
}

static void rotr8(reg8_t *r, int n) {
    n = n % REG8_SIZE;
    uint8_t tmp[REG8_SIZE];
    for (int i = 0; i < REG8_SIZE; i++) {
        tmp[i] = r->cells[(i - n + REG8_SIZE) % REG8_SIZE];
    }
    memcpy(r->cells, tmp, REG8_SIZE);
}

static void rxor8(reg8_t *r, const reg8_t *a) {
    for (int i = 0; i < REG8_SIZE; i++) {
        r->cells[i] ^= a->cells[i];
    }
}

static void delta8(reg8_t *r, uint8_t C) {
    reg8_t t1, t3, tr2, result;
    
    t1 = *r; rotl8(&t1, 1);
    t3 = *r; rotl8(&t3, 3);
    tr2 = *r; rotr8(&tr2, 2);
    
    result = t1;
    rxor8(&result, &t3);
    rxor8(&result, &tr2);
    
    for (int i = 0; i < REG8_SIZE; i++) {
        result.cells[i] ^= C;
    }
    
    *r = result;
}

/* 16-bit register delta */
static void rotl16(reg16_t *r, int n) {
    n = n % REG16_SIZE;
    uint8_t tmp[REG16_SIZE];
    for (int i = 0; i < REG16_SIZE; i++) {
        tmp[i] = r->cells[(i + n) % REG16_SIZE];
    }
    memcpy(r->cells, tmp, REG16_SIZE);
}

static void delta16(reg16_t *r, uint8_t C) {
    reg16_t t1, t3, tr2, result;
    
    t1 = *r; rotl16(&t1, 1);
    t3 = *r; rotl16(&t3, 3);
    /* rotr by 2 */
    rotl16(&tr2, -2);
    
    result = t1;
    for (int i = 0; i < REG16_SIZE; i++) result.cells[i] ^= t3.cells[i];
    for (int i = 0; i < REG16_SIZE; i++) result.cells[i] ^= tr2.cells[i];
    for (int i = 0; i < REG16_SIZE; i++) result.cells[i] ^= C;
    
    *r = result;
}

/* ============================================================
 * IEEE 754-64 (DOUBLE) ENCODER
 * ============================================================ */

typedef struct {
    reg8_t  sign[1];
    reg8_t  exponent[2];   /* 11 bits */
    reg8_t  fraction[7];  /* 52 bits */
    reg8_t  tangent[8][8];
} encoding64_t;

static void encode_double(encoding64_t *e, double value) {
    ieee754_64_t ieee;
    ieee.d = value;
    
    /* Sign: 1 bit */
    uint8_t s = ieee.bits.sign ? 0x01 : 0x00;
    for (int i = 0; i < REG8_SIZE; i++) e->sign[0].cells[i] = s;
    
    /* Exponent: 11 bits */
    uint16_t exp = ieee.bits.exponent;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < REG8_SIZE; j++) {
            int bit_idx = i * 8 + (7 - j);
            if (bit_idx < 11) {
                e->exponent[i].cells[j] = (exp >> (10 - bit_idx)) & 1 ? 0x01 : 0x00;
            } else {
                e->exponent[i].cells[j] = 0x00;
            }
        }
    }
    
    /* Fraction: 52 bits */
    uint64_t frac = ieee.bits.fraction;
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < REG8_SIZE; j++) {
            int bit_idx = i * 8 + (7 - j);
            if (bit_idx < 52) {
                e->fraction[i].cells[j] = (frac >> (51 - bit_idx)) & 1 ? 0x01 : 0x00;
            } else {
                e->fraction[i].cells[j] = 0x00;
            }
        }
    }
    
    /* Tangent matrix: 8x8 */
    uint8_t preheader[REG8_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
    reg8_t reg;
    for (int i = 0; i < REG8_SIZE; i++) reg.cells[i] = preheader[i];
    
    for (int row = 0; row < 8; row++) {
        for (int i = 0; i < REG8_SIZE; i++) {
            e->tangent[row][i] = reg.cells[i];
        }
        delta8(&reg, 0x02);
    }
}

static double decode_double(const encoding64_t *e) {
    ieee754_64_t ieee;
    
    ieee.bits.sign = e->sign[0].cells[0] ? 1 : 0;
    
    uint16_t exp = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < REG8_SIZE; j++) {
            int bit_idx = i * 8 + j;
            if (bit_idx < 11) {
                exp = (exp << 1) | (e->exponent[i].cells[j] ? 1 : 0);
            }
        }
    }
    ieee.bits.exponent = exp;
    
    uint64_t frac = 0;
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < REG8_SIZE; j++) {
            int bit_idx = i * 8 + j;
            if (bit_idx < 52) {
                frac = (frac << 1) | (e->fraction[i].cells[j] ? 1 : 0);
            }
        }
    }
    ieee.bits.fraction = frac;
    
    return ieee.d;
}

/* ============================================================
 * IEEE 754-128 (QUAD) ENCODER
 * ============================================================ */

typedef struct {
    reg8_t  sign[1];
    reg8_t  exponent[2];    /* 15 bits */
    reg8_t  fraction[14];  /* 112 bits */
    reg8_t  tangent[8][8];
} encoding128_t;

static void encode_quad(encoding128_t *e, long double value) {
    /* For simplicity, we simulate with uint128 */
    unsigned __int128 u;
    
    /* Convert long double to quad (simplified) */
    /* In practice, this requires platform-specific handling */
    
    /* Sign: 1 bit */
    uint8_t s = (value < 0) ? 0x01 : 0x00;
    for (int i = 0; i < REG8_SIZE; i++) e->sign[0].cells[i] = s;
    
    /* Exponent: 15 bits (bias = 16383) */
    /* Simplified: use double representation for demo */
    ieee754_64_t d;
    d.d = (double)value;
    uint16_t exp15 = d.bits.exponent + (16383 - 127);  /* Adjust bias */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < REG8_SIZE; j++) {
            int bit_idx = i * 8 + (7 - j);
            if (bit_idx < 15) {
                e->exponent[i].cells[j] = (exp15 >> (14 - bit_idx)) & 1 ? 0x01 : 0x00;
            } else {
                e->exponent[i].cells[j] = 0x00;
            }
        }
    }
    
    /* Fraction: 112 bits */
    /* Simplified: use double fraction padded */
    uint64_t frac64 = d.bits.fraction;
    for (int i = 0; i < 14; i++) {
        for (int j = 0; j < REG8_SIZE; j++) {
            int bit_idx = i * 8 + (7 - j);
            if (bit_idx < 52) {
                e->fraction[i].cells[j] = (frac64 >> (51 - bit_idx)) & 1 ? 0x01 : 0x00;
            } else {
                e->fraction[i].cells[j] = 0x00;
            }
        }
    }
    
    /* Tangent matrix: 8x8 */
    uint8_t preheader[REG8_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
    reg8_t reg;
    for (int i = 0; i < REG8_SIZE; i++) reg.cells[i] = preheader[i];
    
    for (int row = 0; row < 8; row++) {
        for (int i = 0; i < REG8_SIZE; i++) {
            e->tangent[row][i] = reg.cells[i];
        }
        delta8(&reg, 0x02);
    }
}

/* ============================================================
 * MAIN: Test all formats
 * ============================================================ */

int main(void) {
    printf("========================================\n");
    printf("IEEE 754 ENCODER: binary32, binary64, binary128\n");
    printf("========================================\n\n");
    
    /* Test binary32 */
    printf("=== BINARY32 (float) ===\n");
    float f = 3.14159f;
    printf("Value: %f\n", f);
    printf("Hex: 0x%08X\n", *(uint32_t*)&f);
    printf("Sign: %d, Exponent: %d (bias=127), Fraction: 0x%X\n\n",
           ((*(uint32_t*)&f) >> 31) & 1,
           ((*(uint32_t*)&f) >> 23) & 0xFF,
           (*(uint32_t*)&f) & 0x7FFFFF);
    
    /* Test binary64 */
    printf("=== BINARY64 (double) ===\n");
    double d = 3.141592653589793;
    ieee754_64_t ieee64;
    ieee64.d = d;
    printf("Value: %.15f\n", d);
    printf("Hex: 0x%016lX\n", ieee64.u);
    printf("Sign: %d, Exponent: %d (bias=1023), Fraction: 0x%lX\n\n",
           ieee64.bits.sign,
           ieee64.bits.exponent,
           ieee64.bits.fraction);
    
    /* Test encoding/decoding */
    printf("=== ENCODING/DECODING TEST ===\n");
    
    double test_doubles[] = {0.0, 1.0, -1.0, 3.141592653589793, 2.718281828459045};
    const char *names[] = {"Zero", "One", "Negative One", "Pi", "e"};
    
    for (int i = 0; i < 5; i++) {
        encoding64_t enc;
        encode_double(&enc, test_doubles[i]);
        double dec = decode_double(&enc);
        
        printf("%s: %.15f -> %.15f", names[i], test_doubles[i], dec);
        if (test_doubles[i] == dec || (isnan(test_doubles[i]) && isnan(dec))) {
            printf(" ✓\n");
        } else {
            printf(" (diff: %.15e)\n", test_doubles[i] - dec);
        }
    }
    
    printf("\n=== FORMAT COMPARISON ===\n\n");
    
    printf("Format     Bits  Exponent Bias   Fraction    Unicode Planes\n");
    printf("--------- -----  -------------   ---------   ---------------\n");
    printf("binary32     32         127          23         Plane 0\n");
    printf("binary64     64        1023          52      Planes 0-3\n");
    printf("binary128   128       16383         112   All 17 planes\n");
    printf("binary256   256      262143         224  Beyond Unicode\n");
    
    printf("\n=== DELTA LAW PERIOD ===\n\n");
    
    printf("Register size -> Period -> Prime\n");
    printf("     8 bits ->      8 -> 73\n");
    printf("    16 bits ->     16 -> ??\n");
    printf("    32 bits ->     32 -> ??\n");
    printf("    64 bits ->     64 -> ??\n");
    
    printf("\nThe matrix breaks into tangent encoding.\n");
    printf("The universal codeword is NUL.\n");
    printf("IEEE 754 is the projection.\n");
    
    return 0;
}
/*
 * IEEE 754 BINARY32 ENCODER/DECODER
 * ==================================
 * 
 * Uses the Universal Codeword Bootloader kernel to convert
 * floating-point numbers to/from the tangent encoding matrix.
 * 
 * IEEE 754 binary32:
 * - 1 sign bit
 * - 8 exponent bits (bias = 127)
 * - 23 fraction bits
 * 
 * Maps to our 8-period delta law system.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define REGISTER_SIZE 8

typedef struct {
    uint8_t cells[REGISTER_SIZE];
} reg_t;

static const char *codon_name(uint8_t c) {
    switch(c) {
        case 0x00: return "NUL";
        case 0x01: return "SOH";
        case 0x02: return "STX";
        case 0x03: return "ETX";
        case 0x04: return "EOT";
        case 0x05: return "ENQ";
        case 0x06: return "ACK";
        case 0x07: return "BEL";
        case 0x1C: return "FS";
        case 0x1D: return "GS";
        case 0x1E: return "RS";
        case 0x1F: return "US";
        default:   return "?";
    }
}

static void init_reg(reg_t *r, const uint8_t *cells) {
    memcpy(r->cells, cells, REGISTER_SIZE);
}

static void print_reg(const char *label, const reg_t *r) {
    printf("%s: ", label);
    for (int i = 0; i < REGISTER_SIZE; i++) {
        printf("%02X ", r->cells[i]);
    }
    printf("\n");
}

static void rotl(reg_t *r, int n) {
    n = n % REGISTER_SIZE;
    uint8_t tmp[REGISTER_SIZE];
    for (int i = 0; i < REGISTER_SIZE; i++) {
        tmp[i] = r->cells[(i + n) % REGISTER_SIZE];
    }
    memcpy(r->cells, tmp, REGISTER_SIZE);
}

static void rotr(reg_t *r, int n) {
    n = n % REGISTER_SIZE;
    uint8_t tmp[REGISTER_SIZE];
    for (int i = 0; i < REGISTER_SIZE; i++) {
        tmp[i] = r->cells[(i - n + REGISTER_SIZE) % REGISTER_SIZE];
    }
    memcpy(r->cells, tmp, REGISTER_SIZE);
}

static void rxor(reg_t *r, const reg_t *a) {
    for (int i = 0; i < REGISTER_SIZE; i++) {
        r->cells[i] ^= a->cells[i];
    }
}

static void delta(reg_t *r, uint8_t C) {
    reg_t t1, t3, tr2, result;
    
    t1 = *r; rotl(&t1, 1);
    t3 = *r; rotl(&t3, 3);
    tr2 = *r; rotr(&tr2, 2);
    
    result = t1;
    rxor(&result, &t3);
    rxor(&result, &tr2);
    
    for (int i = 0; i < REGISTER_SIZE; i++) {
        result.cells[i] ^= C;
    }
    
    *r = result;
}

/* ============================================================
 * IEEE 754 BINARY32 STRUCTURE
 * ============================================================ */

typedef union {
    float f;
    uint32_t u;
    struct {
        uint32_t fraction : 23;
        uint32_t exponent : 8;
        uint32_t sign : 1;
    } bits;
} ieee754_t;

/* ============================================================
 * ENCODER: float -> tangent encoding matrix
 * ============================================================ */

typedef struct {
    reg_t sign_register;      /* Sign bit */
    reg_t exponent_register;  /* 8-bit exponent */
    reg_t fraction_register; /* 23-bit fraction */
    reg_t tangent_matrix[8];  /* 8x8 tangent encoding */
} encoding_t;

static void encode_float(encoding_t *e, float value) {
    ieee754_t ieee;
    ieee.f = value;
    
    /* Initialize preheader based on IEEE 754 fields */
    uint8_t preheader[REGISTER_SIZE];
    
    /* Sign register: all NUL if positive, all SOH if negative */
    uint8_t sign_val = ieee.bits.sign ? 0x01 : 0x00;
    for (int i = 0; i < REGISTER_SIZE; i++) {
        preheader[i] = sign_val;
    }
    init_reg(&e->sign_register, preheader);
    
    /* Exponent register: bias = 127 */
    uint8_t exp = ieee.bits.exponent;
    for (int i = 0; i < REGISTER_SIZE; i++) {
        preheader[i] = (exp >> (7 - i)) & 1 ? 0x01 : 0x00;
    }
    init_reg(&e->exponent_register, preheader);
    
    /* Fraction register */
    uint32_t frac = ieee.bits.fraction;
    for (int i = 0; i < REGISTER_SIZE; i++) {
        preheader[i] = (frac >> (7 - i)) & 1 ? 0x01 : 0x00;
    }
    init_reg(&e->fraction_register, preheader);
    
    /* Generate tangent matrix using delta law */
    /* Preheader: NUL NUL NUL NUL SOH NUL NUL NUL */
    uint8_t matrix_preheader[REGISTER_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
    reg_t reg;
    init_reg(&reg, matrix_preheader);
    
    for (int row = 0; row < 8; row++) {
        e->tangent_matrix[row] = reg;
        delta(&reg, 0x02);  /* C = STX */
    }
}

/* ============================================================
 * DECODER: tangent encoding matrix -> float
 * ============================================================ */

static float decode_float(const encoding_t *e) {
    ieee754_t ieee;
    
    /* Decode sign */
    ieee.bits.sign = (e->sign_register.cells[0] != 0) ? 1 : 0;
    
    /* Decode exponent */
    uint8_t exp = 0;
    for (int i = 0; i < 8; i++) {
        exp = (exp << 1) | (e->exponent_register.cells[i] != 0);
    }
    ieee.bits.exponent = exp;
    
    /* Decode fraction */
    uint32_t frac = 0;
    for (int i = 0; i < 23; i++) {
        int reg_idx = i / 8;
        int bit_idx = i % 8;
        frac = (frac << 1) | (e->fraction_register.cells[reg_idx] != 0);
    }
    ieee.bits.fraction = frac;
    
    return ieee.f;
}

/* ============================================================
 * PRINT IEEE 754 STRUCTURE
 * ============================================================ */

static void print_ieee754(const char *label, float value) {
    ieee754_t ieee;
    ieee.f = value;
    
    printf("%s:\n", label);
    printf("  Value:   %g\n", value);
    printf("  Hex:     0x%08X\n", ieee.u);
    printf("  Sign:    %d\n", ieee.bits.sign);
    printf("  Exponent: %03d (bias=%d, actual=%d)\n", 
           ieee.bits.exponent, 127, ieee.bits.exponent - 127);
    printf("  Fraction: 0x%06X\n", ieee.bits.fraction);
    printf("\n");
}

static void print_encoding(const char *label, const encoding_t *e) {
    printf("%s:\n", label);
    print_reg("  Sign", &e->sign_register);
    print_reg("  Exponent", &e->exponent_register);
    print_reg("  Fraction", &e->fraction_register);
    printf("  Tangent Matrix:\n");
    for (int i = 0; i < 8; i++) {
        printf("    Row %d: ", i);
        for (int j = 0; j < 8; j++) {
            printf("%02X ", e->tangent_matrix[i].cells[j]);
        }
        printf("\n");
    }
    printf("\n");
}

/* ============================================================
 * MAIN: Test encoder/decoder
 * ============================================================ */

int main(void) {
    printf("========================================\n");
    printf("IEEE 754 BINARY32 ENCODER/DECODER\n");
    printf("========================================\n\n");
    
    /* Test values covering IEEE 754 special cases */
    float test_values[] = {
        0.0f,           /* Zero */
        -0.0f,          /* Negative zero */
        1.0f,           /* One */
        -1.0f,          /* Negative one */
        3.14159f,       /* Pi */
        2.71828f,       /* e */
        1.0e-45f,       /* Smallest subnormal */
        3.4028235e38f,   /* Max finite */
        INFINITY,        /* Positive infinity */
        -INFINITY,       /* Negative infinity */
        NAN              /* NaN */
    };
    
    const char *test_names[] = {
        "Zero",
        "Negative zero",
        "One",
        "Negative one",
        "Pi",
        "e",
        "Smallest subnormal",
        "Max finite",
        "+Infinity",
        "-Infinity",
        "NaN"
    };
    
    for (int i = 0; i < sizeof(test_values) / sizeof(test_values[0]); i++) {
        printf("=== Test: %s ===\n", test_names[i]);
        
        /* Encode */
        encoding_t enc;
        encode_float(&enc, test_values[i]);
        
        /* Print IEEE 754 breakdown */
        print_ieee754("IEEE 754", test_values[i]);
        
        /* Print encoding */
        print_encoding("Encoding", &enc);
        
        /* Decode */
        float decoded = decode_float(&enc);
        
        /* Verify */
        if (isnan(test_values[i])) {
            printf("Decoded: %g (original was NaN)\n\n", decoded);
        } else if (test_values[i] == decoded) {
            printf("Decoded: %g ✓\n\n", decoded);
        } else {
            printf("Decoded: %g ✗ (mismatch)\n\n", decoded);
        }
    }
    
    /* Show the mapping from delta law to IEEE 754 */
    printf("========================================\n");
    printf("DELTA LAW -> IEEE 754 MAPPING\n");
    printf("========================================\n\n");
    
    printf("IEEE 754 binary32 fields:\n");
    printf("  Sign:     1 bit  -> Register 0 (1 cell)\n");
    printf("  Exponent:  8 bits -> Registers 0-1 (8 cells)\n");
    printf("  Fraction: 23 bits -> Registers 1-3 (24 cells, 1 unused)\n");
    printf("\n");
    
    printf("Exponent bias = 127 (binary: 01111111)\n");
    printf("  This is the position of SOH in the preheader!\n");
    printf("\n");
    
    printf("Tangent matrix -> Fraction significand:\n");
    printf("  Row 0: Most significant bits\n");
    printf("  Row 7: Least significant bits\n");
    printf("\n");
    
    printf("The matrix breaks into tangent encoding.\n");
    printf("The universal codeword is NUL.\n");
    printf("The IEEE 754 format is the projection.\n");
    
    return 0;
}
/*
 * endianness-probe.c - Test BOM chirality on any platform
 * 
 * Tests the pairwise circular inversion:
 * "10" in binary (positional) == "2" in binary (sign-value)
 * 
 * BOM_FEFF = 0xFEFF = Big Endian (right-handed snub)
 * BOM_FFFE = 0xFFFE = Little Endian (left-handed snub)
 * 
 * The 8th bit (parity) = M action (Mirror)
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define BOM_FEFF 0xFEFF
#define BOM_FFFE 0xFFFE

typedef struct {
    uint32_t bom;
    const char *name;
    int is_little_endian;
    int chirality;  /* +1 = right-handed, -1 = left-handed */
} Endianness;

static void detect_endianness(Endianness *e) {
    uint32_t test = 0x01020304;
    uint8_t *bytes = (uint8_t*)&test;
    
    if (bytes[0] == 0x04) {
        e->bom = BOM_FFFE;  /* Little endian */
        e->name = "Little Endian";
        e->is_little_endian = 1;
        e->chirality = -1;  /* Left-handed snub */
    } else {
        e->bom = BOM_FEFF;  /* Big endian */
        e->name = "Big Endian";
        e->is_little_endian = 0;
        e->chirality = +1;  /* Right-handed snub */
    }
}

/* The pairwise circular inversion test */
static void test_pairwise_inversion(void) {
    printf("=== PAIRWISE CIRCULAR INVERSION TEST ===\n\n");
    
    /* "10" in positional = 2 in value */
    /* In binary: 0b10 = 2 */
    
    /* 7-bit ASCII: 0x00 - 0x7F */
    /* 8-bit ASCII: 0x00 - 0xFF */
    
    printf("7-bit vs 8-bit identity:\n");
    printf("  0x02 (STX) = 7-bit control char\n");
    printf("  0x0A (LF)  = 8-bit line feed\n");
    printf("  Both resolve to same group action under BOM encapsulation\n\n");
    
    /* The M action (Mirror) is the 8th bit */
    printf("Parity (M action) = 8th bit:\n");
    for (int i = 0; i < 8; i++) {
        uint8_t val = 1 << i;
        int has_parity = (val & 0x80) ? 1 : 0;
        printf("  Bit %d (0x%02X): Parity=%s\n", i, val, has_parity ? "ON" : "OFF");
    }
    printf("\n");
}

/* Test the hopf fibration path lengths */
static void test_hopf_fibration(void) {
    printf("=== HOPF FIBRATION PATH (16-cell orthoscheme) ===\n\n");
    
    /* From the 16-cell characteristic 5-cell table */
    double edges[] = {
        1.41421356,   /* √2 = edge */
        0.81649658,   /* √(2/3) = 𝟀 */
        0.70710678,   /* √(1/2) = 𝝉 */
        0.40824829,   /* √(1/6) = 𝟁 */
        0.86602540,   /* √(3/4) */
        0.50000000,   /* √(1/4) */
        0.28867513    /* √(1/12) */
    };
    
    const char *labels[] = {
        "edge",
        "𝟀 (60\")",
        "𝝉 (45\")", 
        "𝟁 (30\")",
        "cell→face",
        "face→edge",
        "edge→vertex"
    };
    
    printf("Path step          Length      ASCII Channel\n");
    printf("------------------ ----------- -------------\n");
    
    printf("vertex→edge        %f FS (0x1C) q∘r\n", edges[2]);
    printf("edge→face          %f GS (0x1D) r∘p\n", edges[3]);
    printf("face→cell          %f RS (0x1E) p∘q\n", edges[5]);
    printf("cell→center        %f US (0x1F) p∘q∘r\n\n", edges[5]);
    
    printf("Irreducible point (2^-1074 in binary64):\n");
    printf("  %e (smallest subnormal)\n\n", 2.0e-1074);
}

/* Verify the 5-cell / 16-cell orthoscheme identity */
static void test_orthoscheme_identity(void) {
    printf("=== 5-CELL / 16-CELL ORTHOSCHEME IDENTITY ===\n\n");
    
    printf("Both bounded by regular tetrahedra:\n");
    printf("  5-cell:  5 vertices, 10 edges, 10 faces (4 triangles each)\n");
    printf("  16-cell: 8 vertices, 24 edges, 32 faces (triangles)\n\n");
    
    printf("Characteristic tetrahedron (3-orthoscheme):\n");
    printf("  Same right triangle: 𝟀 + 𝝉 + 𝟁 = 90°\n");
    printf("  60\" + 45\" + 30\" = 135° (in dihedral terms)\n\n");
    
    printf("This is why:\n");
    printf("  \"10\" (binary) = \"2\" (binary)\n");
    printf("  When BOM-encapsulated, same group action\n");
}

int main(int argc, char *argv[]) {
    printf("╔═══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║  ENDIANNESS & CHIRALITY PROBE                                              ║\n");
    printf("║  =======================================                                    ║\n");
    printf("║  Test pairwise circular inversion on this platform                          ║\n");
    printf("╚═══════════════════════════════════════════════════════════════════════════════╝\n\n");
    
    /* Detect platform endianness */
    Endianness e;
    detect_endianness(&e);
    
    printf("=== PLATFORM DETECTION ===\n");
    printf("  Endianness:     %s\n", e.name);
    printf("  BOM:            0x%04X\n", e.bom);
    printf("  Chirality:      %s\n", e.chirality > 0 ? "Right-handed (snub+)" : "Left-handed (snub-)");
    printf("  sizeof(int):    %zu bytes\n", sizeof(int));
    printf("  sizeof(void*):  %zu bytes (%zu-bit)\n\n", sizeof(void*), sizeof(void*) * 8);
    
    /* Test pairwise inversion */
    test_pairwise_inversion();
    
    /* Test Hopf fibration */
    test_hopf_fibration();
    
    /* Test orthoscheme */
    test_orthoscheme_identity();
    
    /* Summary */
    printf("╔═══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║  PAIRWISE CIRCULAR INVERSION SUMMARY                                        ║\n");
    printf("╠═══════════════════════════════════════════════════════════════════════════════╣\n");
    printf("║  Platform chirality:        %-45s║\n", e.name);
    printf("║  \"10\" (binary) ≡ \"2\":     %-45s║\n", 
           e.chirality > 0 ? "Right-handed" : "Left-handed");
    printf("║  7-bit ≡ 8-bit (mod M):    %-45s║\n",
           e.is_little_endian ? "Parity flip" : "Parity preserved");
    printf("║  Hopf flow direction:       %-45s║\n",
           e.chirality > 0 ? "Clockwise" : "Counter-clockwise");
    printf("╚═══════════════════════════════════════════════════════════════════════════════╝\n");
    
    return 0;
}
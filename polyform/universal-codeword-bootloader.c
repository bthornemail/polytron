/*
 * UNIVERSAL CODEWORD BOOTLOADER
 * ============================
 * 
 * NULNULNULNULSOHNULNULNUL  → preheader
 * STXSOHSTXSOHSTXSOHSTXSTX  → tangent encoding
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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
        printf("%s ", codon_name(r->cells[i]));
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

static uint64_t fibonacci(int n) {
    if (n <= 1) return n;
    uint64_t a = 0, b = 1;
    for (int i = 2; i <= n; i++) {
        uint64_t c = a + b;
        a = b; b = c;
    }
    return b;
}

int main(void) {
    printf("UNIVERSAL CODEWORD BOOTLOADER\n");
    printf("===========================\n\n");
    
    /* Preheader: NUL NUL NUL NUL SOH NUL NUL NUL */
    uint8_t preheader[REGISTER_SIZE] = {0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};
    reg_t reg;
    init_reg(&reg, preheader);
    print_reg("Preheader", &reg);
    
    /* Delta law evolution */
    printf("\nDelta evolution:\n");
    for (int step = 0; step < 8; step++) {
        printf("Step %d: ", step);
        for (int i = 0; i < REGISTER_SIZE; i++) {
            printf("%s ", codon_name(reg.cells[i]));
        }
        printf("\n");
        delta(&reg, 0x02);  /* C = STX */
    }
    
    /* Fibonacci */
    printf("\nFibonacci: ");
    for (int i = 0; i < 8; i++) {
        printf("%lu ", fibonacci(i));
    }
    printf("\n");
    
    printf("\nThe matrix breaks into tangent encoding.\n");
    printf("The universal codeword is NUL.\n");
    
    return 0;
}
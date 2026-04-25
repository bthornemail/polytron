/*
 * OMICRON-GNOMON STREAM TO JSON EXPORTER
 * ====================================
 * 
 * This runs the transformer (delta law) and exports the stream as JSON
 * that can be consumed by the WebGL engine.
 * 
 * Compile: gcc -o stream-to-json stream-to-json.c -lm
 * Run: ./stream-to-json
 * Output: stdout (JSON)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define STATE_SIZE 8

/* Initial state */
uint8_t state[STATE_SIZE] = {0, 1, 2, 3, 4, 5, 6, 7};

/* Aegean codepoint range */
#define AEGEAN_BASE 0x10100

/* Geometry names */
const char* geometry_names[16] = {
    "point",        /* 0x10100 */
    "line",         /* 0x10101 */
    "triangle",     /* 0x10102 */
    "unknown",       /* 0x10103 */
    "unknown",       /* 0x10104 */
    "unknown",       /* 0x10105 */
    "unknown",       /* 0x10106 */
    "tetrahedron",  /* 0x10107 */
    "5-cell",       /* 0x10108 */
    "8-cell",       /* 0x10109 */
    "16-cell",      /* 0x1010A */
    "24-cell",      /* 0x1010B */
    "120-cell",     /* 0x1010C */
    "600-cell",     /* 0x1010D */
    "hopf-fiber",   /* 0x1010E */
    "s7"            /* 0x1010F */
};

/* Delta law: rotl(x,1) XOR rotl(x,3) XOR rotr(x,2) XOR C */
void rotl(uint8_t arr[], int n, uint8_t result[]) {
    n = n % STATE_SIZE;
    for (int i = 0; i < STATE_SIZE; i++) {
        result[i] = arr[(i + n) % STATE_SIZE];
    }
}

void rotr(uint8_t arr[], int n, uint8_t result[]) {
    n = n % STATE_SIZE;
    for (int i = 0; i < STATE_SIZE; i++) {
        result[i] = arr[(i - n + STATE_SIZE) % STATE_SIZE];
    }
}

void xor_arr(uint8_t a[], uint8_t b[], uint8_t result[]) {
    for (int i = 0; i < STATE_SIZE; i++) {
        result[i] = a[i] ^ b[i];
    }
}

void delta(uint8_t current[], uint8_t C, uint8_t next[]) {
    uint8_t t1[STATE_SIZE], t3[STATE_SIZE], tr2[STATE_SIZE];
    uint8_t x1[STATE_SIZE], x2[STATE_SIZE];
    
    rotl(current, 1, t1);
    rotl(current, 3, t3);
    rotr(current, 2, tr2);
    
    xor_arr(t1, t3, x1);
    xor_arr(x1, tr2, x2);
    
    for (int i = 0; i < STATE_SIZE; i++) {
        next[i] = x2[i] ^ C;
    }
}

/* Export stream as JSON */
void export_json(int steps, int initial_state[STATE_SIZE]) {
    printf("{\n");
    printf("  \"axiom\": \"NULL != 10\",\n");
    printf("  \"delta_law\": \"rotl(x,1) XOR rotl(x,3) XOR rotr(x,2) XOR C\",\n");
    printf("  \"period\": %d,\n", 8);
    printf("  \"prime\": %d,\n", 73);
    printf("  \"steps\": %d,\n", steps);
    printf("  \"initial_state\": [");
    for (int i = 0; i < STATE_SIZE; i++) {
        printf("%d%s", initial_state[i], i < STATE_SIZE - 1 ? ", " : "");
    }
    printf("],\n");
    printf("  \"stream\": [\n");
    
    /* Reset state */
    for (int i = 0; i < STATE_SIZE; i++) state[i] = initial_state[i];
    
    for (int step = 0; step < steps; step++) {
        /* Compute next state */
        uint8_t next[STATE_SIZE];
        delta(state, 0x02, next); /* C = STX */
        
        /* Map state[0] to geometry */
        int geom_idx = state[0] % 16;
        
        printf("    {\n");
        printf("      \"step\": %d,\n", step);
        printf("      \"state\": [");
        for (int i = 0; i < STATE_SIZE; i++) {
            printf("%d%s", state[i], i < STATE_SIZE - 1 ? ", " : "");
        }
        printf("],\n");
        printf("      \"codepoint\": \"U+%04X\",\n", AEGEAN_BASE + geom_idx);
        printf("      \"geometry\": \"%s\"\n", geometry_names[geom_idx]);
        printf("    }%s\n", step < steps - 1 ? "," : "");
        
        /* Advance state */
        for (int i = 0; i < STATE_SIZE; i++) state[i] = next[i];
    }
    
    printf("  ]\n");
    printf("}\n");
}

/* Export state as binary for WebGL */
void export_binary(int steps) {
    /* Reset state */
    for (int i = 0; i < STATE_SIZE; i++) state[i] = i;
    
    printf("# Binary stream for WebGL\n");
    
    for (int step = 0; step < steps; step++) {
        uint8_t next[STATE_SIZE];
        delta(state, 0x02, next);
        
        int geom_idx = state[0] % 16;
        printf("%02X ", AEGEAN_BASE + geom_idx);
        
        for (int i = 0; i < STATE_SIZE; i++) state[i] = next[i];
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    int steps = 64;
    int initial[STATE_SIZE];
    
    for (int i = 0; i < STATE_SIZE; i++) initial[i] = i;
    
    if (argc > 1) {
        steps = atoi(argv[1]);
    }
    
    printf("# Omicron-Gnomon Stream Exporter\n");
    printf("# Axiom: NULL != 10\n");
    printf("# Delta Law: rotl(x,1) XOR rotl(x,3) XOR rotr(x,2) XOR C\n");
    printf("# Period: 8, Prime: 73\n\n");
    
    export_json(steps, initial);
    
    return 0;
}
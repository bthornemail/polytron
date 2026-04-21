/*
 * TETRA-GNOMON-ANIM — GNOMON 8-Step Cycle Animation (Terminal)
 *
 * Renders the 8-step GNOMON cycle in ASCII art.
 *
 * COMPILE:
 *   gcc -o tetra-gnomon-anim tetra-gnomon-anim.c -lm
 * 
 * RUN:
 *   ./tetra-gnomon-anim
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <stdint.h>

/* -------------------------------------------------------------------------- */
/* EMBEDDED KERNEL                                                          */
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

/* -------------------------------------------------------------------------- */
/* GNOMON STATE COMPUTATION                                                 */
/* -------------------------------------------------------------------------- */

#define GNOMON_STEPS 8

typedef struct {
    Pair state;
    Pair sid;
    float r, g, b;
    char symbol;
} GnomonStep;

static void compute_gnomon(Pair seed, GnomonStep* steps) {
    Pair current = seed;
    const char symbols[] = "●◐○◑○◐●";
    
    for (int i = 0; i < GNOMON_STEPS; i++) {
        steps[i].state = current;
        steps[i].sid = K(current, CONSTITUTIONAL_C);
        
        uint8_t b0 = (current >> 8) & 0xFF;
        uint8_t b1 = current & 0xFF;
        steps[i].r = (float)((b0 >> 4) & 0xF) / 15.0f;
        steps[i].g = (float)(b0 & 0xF) / 15.0f;
        steps[i].b = (float)((b1 >> 4) & 0xF) / 15.0f;
        steps[i].symbol = symbols[i];
        
        current = K(current, CONSTITUTIONAL_C);
    }
}

/* -------------------------------------------------------------------------- */
/* ASCII ART RENDERING                                                       */
/* -------------------------------------------------------------------------- */

#define WIDTH 80
#define HEIGHT 24

static void clear_screen(void) {
    printf("\033[2J\033[H");
}

static void set_color(float r, float g, float b) {
    int ri = (int)(r * 5);
    int gi = (int)(g * 5);
    int bi = (int)(b * 5);
    int color = 16 + ri * 36 + gi * 6 + bi;
    printf("\033[38;5;%dm", color);
}

static void reset_color(void) {
    printf("\033[0m");
}

static void draw_gnomon_ascii(GnomonStep* steps, int current_step) {
    clear_screen();
    
    printf("═══ TETRA GNOMON ANIMATION ═══\n");
    printf("Period: %d steps\n\n", GNOMON_STEPS);
    
    /* Draw cycle positions */
    for (int i = 0; i < GNOMON_STEPS; i++) {
        float angle = (float)i / GNOMON_STEPS * 6.28318f;
        int x = (int)(40 + 30 * cosf(angle));
        int y = (int)(12 + 10 * sinf(angle));
        
        /* Position cursor and draw */
        printf("\033[%d;%dH", y, x);
        
        if (i == current_step) {
            set_color(steps[i].r, steps[i].g, steps[i].b);
            printf("●");
        } else {
            set_color(steps[i].r * 0.5f, steps[i].g * 0.5f, steps[i].b * 0.5f);
            printf("○");
        }
    }
    
    reset_color();
    
    /* Draw info */
    printf("\033[22;1H");
    printf("Current Step: %d\n", current_step);
    printf("State: 0x%04X\n", steps[current_step].state);
    printf("SID:  0x%04X\n", steps[current_step].sid);
    
    /* Draw state bar */
    printf("\nState evolution:\n");
    for (int i = 0; i < GNOMON_STEPS; i++) {
        if (i == current_step) {
            set_color(steps[i].r, steps[i].g, steps[i].b);
            printf("█");
        } else {
            printf("░");
        }
    }
    reset_color();
    printf("\n");
    
    /* Draw kernel formula */
    printf("\nKernel: K(p,C) = rotl(p,1) ^ rotl(p,3) ^ rotr(p,2) ^ C\n");
    printf("Constitutional constant: 0x%02X\n", CONSTITUTIONAL_C);
    
    fflush(stdout);
}

/* -------------------------------------------------------------------------- */
/* MAIN                                                                     */
/* -------------------------------------------------------------------------- */

int main(int argc, char** argv) {
    printf("═══ TETRA GNOMON ANIMATION ═══\n");
    
    /* Compute GNOMON states */
    GnomonStep steps[GNOMON_STEPS];
    compute_gnomon(0x4242, steps);
    
    printf("GNOMON cycle (period=%d):\n", GNOMON_STEPS);
    for (int i = 0; i < GNOMON_STEPS; i++) {
        printf("  step %d: state=0x%04X, SID=0x%04X\n",
               i, steps[i].state, steps[i].sid);
    }
    
    printf("\nRendering...\n");
    printf("Use terminal with 256 color support for best results.\n");
    printf("Press Ctrl+C to exit.\n");
    
    sleep(1);
    
    int step = 0;
    while (1) {
        draw_gnomon_ascii(steps, step);
        usleep(500000);  /* 0.5 seconds per frame */
        step = (step + 1) % GNOMON_STEPS;
    }
    
    return 0;
}
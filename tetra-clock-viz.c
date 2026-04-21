/*
 * TETRA-CLOCK-VIZ — Clock Tree Visualization
 *
 * Exports the constitutional clock tree as Graphviz DOT format.
 * 
 * COMPILE:
 *   gcc -o tetra-clock-viz tetra-clock-viz.c -lm
 * 
 * OUTPUT:
 *   ./tetra-clock-viz > clock-tree.dot
 *   dot -Tpng clock-tree.dot -o clock-tree.png
 */

#include <stdio.h>
#include <stdlib.h>
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
/* GNOMON STEPS                                                             */
/* -------------------------------------------------------------------------- */

#define GNOMON_STEPS 8

typedef struct {
    Pair state;
    Pair sid;
    int period;
} GnomonState;

static void compute_gnomon(Pair seed, GnomonState* states) {
    Pair current = seed;
    
    for (int i = 0; i < GNOMON_STEPS; i++) {
        states[i].state = current;
        states[i].sid = K(current, CONSTITUTIONAL_C);
        
        states[i].period = -1;
        for (int j = 0; j < i; j++) {
            if (states[j].state == current) {
                states[i].period = i - j;
                break;
            }
        }
        
        current = K(current, CONSTITUTIONAL_C);
    }
}

/* -------------------------------------------------------------------------- */
/* GRAPHVIZ OUTPUT                                                          */
/* -------------------------------------------------------------------------- */

int main(int argc, char** argv) {
    Pair seed = 0x4242;
    GnomonState states[GNOMON_STEPS];
    
    compute_gnomon(seed, states);
    
    /* Print Graphviz DOT */
    printf("digraph tetra_clock_tree {\n");
    printf("  rankdir=TB;\n");
    printf("  graph [splines=ortho, nodesep=0.8, ranksep=1.2];\n");
    printf("  node [shape=box, style=rounded, fontname=Helvetica, fontsize=12];\n");
    printf("  edge [color=\"#1D1D1D\", penwidth=2];\n");
    printf("\n");
    
    /* Constitutional clocks */
    printf("  subgraph cluster_const {\n");
    printf("    label=\"CONSTITUTION\";\n");
    printf("    bgcolor=\"#f5f5f5\";\n");
    printf("    style=dashed;\n");
    
    printf("    FS [label=\"FS\\n(File Separator)\\nbase clock\", fillcolor=\"#E0E0E0\", penwidth=2];\n");
    printf("    GS [label=\"GS\\n(Group Separator)\\nCONSTITUTIONAL\", fillcolor=\"#1D1D1D\", fontcolor=white, penwidth=2];\n");
    printf("    RS [label=\"RS\\n(Record Separator)\\nreference\", fillcolor=\"#404040\", fontcolor=white, penwidth=2];\n");
    printf("    US [label=\"US\\n(Unit Separator)\\nactive\", fillcolor=\"#000000\", fontcolor=white, penwidth=2];\n");
    printf("  }\n");
    printf("\n");
    
    /* GNOMON growth */
    printf("  subgraph cluster_gnomon {\n");
    printf("    label=\"GNOMON Growth (period=8)\";\n");
    printf("    bgcolor=\"#FFF8E0\";\n");
    printf("    style=dashed;\n");
    
    for (int i = 0; i < GNOMON_STEPS; i++) {
        printf("    g%d [label=\"step %d\\nstate=0x%04X\\nSID=0x%04X\", fillcolor=\"#FFFFF0\", penwidth=1];\n",
               i, i, states[i].state, states[i].sid);
    }
    
    for (int i = 0; i < GNOMON_STEPS - 1; i++) {
        printf("    g%d -> g%d [style=dashed, color=\"#808080\", penwidth=1];\n", i, i+1);
    }
    printf("  }\n");
    printf("\n");
    
    /* Clock connections */
    printf("  FS -> GS [label=\"K(p,0x1D)\", color=\"#1D1D1D\", penwidth=3];\n");
    printf("  GS -> RS [label=\"channel\", color=\"#404040\"];\n");
    printf("  RS -> US [label=\"propagate\", color=\"#606060\"];\n");
    printf("\n");
    
    /* Kernel */
    printf("  kernel [label=\"K(p,C)\\n= rotl(p,1) ^ rotl(p,3)\\n^ rotr(p,2) ^ C\", shape=ellipse, fillcolor=\"#E8E8E8\", style=bold, fontsize=14];\n");
    printf("  kernel -> FS [style=invis];\n");
    printf("\n");
    
    printf("}\n");
    
    return 0;
}
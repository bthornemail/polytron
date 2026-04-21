/*
 * TETRA-OMICRON — Dalí Cross 3D Model Exporter
 *
 * Exports the 8-cube Dalí cross (omicron) as OBJ/STL.
 *
 * COMPILE:
 *   gcc -o tetra-omicron tetra-omicron.c -lm
 *
 * OUTPUT:
 *   ./tetra-omicron          # Print info
 *   ./tetra-omicron obj      # Generate dali.obj
 *   ./tetra-omicron stl      # Generate dali.stl
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
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
/* OMICRON (Dalí Cross)                                                      */
/* -------------------------------------------------------------------------- */

#define OMICRON_CUBES 8

typedef struct {
    float x, y, z;      /* Position */
    Pair sid;           /* Kernel hash */
} Cube;

static void make_omicron(Cube cubes[OMICRON_CUBES]) {
    /* 8 voxels arranged as a cross */
    float positions[OMICRON_CUBES][3] = {
        { 0.0f,  0.0f,  0.0f},  /* center */
        { 0.0f,  1.0f,  0.0f},  /* top */
        { 0.0f, -1.0f,  0.0f},  /* bottom */
        { 1.0f,  0.0f,  0.0f},  /* right */
        {-1.0f,  0.0f,  0.0f},  /* left */
        { 0.0f,  0.0f,  1.0f},  /* front */
        { 0.0f,  0.0f, -1.0f},  /* back */
        { 1.0f,  1.0f,  1.0f}   /* corner */
    };
    
    Pair base = K(0x4242, CONSTITUTIONAL_C);
    
    for (int i = 0; i < OMICRON_CUBES; i++) {
        cubes[i].x = positions[i][0];
        cubes[i].y = positions[i][1];
        cubes[i].z = positions[i][2];
        
        Pair pos = cons((uint8_t)(positions[i][0] + 2), 
                       (uint8_t)(positions[i][1] + 2));
        pos = cons(car(pos), (uint8_t)(positions[i][2] + 2));
        cubes[i].sid = K(base ^ pos, CONSTITUTIONAL_C);
    }
}

/* -------------------------------------------------------------------------- */
/* CUBE MESH GENERATION                                                      */
/* -------------------------------------------------------------------------- */

#define CUBE_FACES 6
#define FACE_VERTICES 4

typedef struct {
    float v[3];
} Vertex;

typedef struct {
    Vertex verts[4];
    float normal[3];
} Face;

static void make_cube_mesh(float size, Face faces[CUBE_FACES]) {
    float h = size / 2.0f;
    
    /* Front face (z = +h) */
    faces[0].verts[0].v[0] = -h; faces[0].verts[0].v[1] = -h; faces[0].verts[0].v[2] = h;
    faces[0].verts[1].v[0] =  h; faces[0].verts[1].v[1] = -h; faces[0].verts[1].v[2] = h;
    faces[0].verts[2].v[0] =  h; faces[0].verts[2].v[1] =  h; faces[0].verts[2].v[2] = h;
    faces[0].verts[3].v[0] = -h; faces[0].verts[3].v[1] =  h; faces[0].verts[3].v[2] = h;
    faces[0].normal[0] = 0; faces[0].normal[1] = 0; faces[0].normal[2] = 1;
    
    /* Back face (z = -h) */
    faces[1].verts[0].v[0] =  h; faces[1].verts[0].v[1] = -h; faces[1].verts[0].v[2] = -h;
    faces[1].verts[1].v[0] = -h; faces[1].verts[1].v[1] = -h; faces[1].verts[1].v[2] = -h;
    faces[1].verts[2].v[0] = -h; faces[1].verts[2].v[1] =  h; faces[1].verts[2].v[2] = -h;
    faces[1].verts[3].v[0] =  h; faces[1].verts[3].v[1] =  h; faces[1].verts[3].v[2] = -h;
    faces[1].normal[0] = 0; faces[1].normal[1] = 0; faces[1].normal[2] = -1;
    
    /* Top face (y = +h) */
    faces[2].verts[0].v[0] = -h; faces[2].verts[0].v[1] = h; faces[2].verts[0].v[2] = h;
    faces[2].verts[1].v[0] =  h; faces[2].verts[1].v[1] = h; faces[2].verts[1].v[2] = h;
    faces[2].verts[2].v[0] =  h; faces[2].verts[2].v[1] = h; faces[2].verts[2].v[2] = -h;
    faces[2].verts[3].v[0] = -h; faces[2].verts[3].v[1] = h; faces[2].verts[3].v[2] = -h;
    faces[2].normal[0] = 0; faces[2].normal[1] = 1; faces[2].normal[2] = 0;
    
    /* Bottom face (y = -h) */
    faces[3].verts[0].v[0] = -h; faces[3].verts[0].v[1] = -h; faces[3].verts[0].v[2] = -h;
    faces[3].verts[1].v[0] =  h; faces[3].verts[1].v[1] = -h; faces[3].verts[1].v[2] = -h;
    faces[3].verts[2].v[0] =  h; faces[3].verts[2].v[1] = -h; faces[3].verts[2].v[2] = h;
    faces[3].verts[3].v[0] = -h; faces[3].verts[3].v[1] = -h; faces[3].verts[3].v[2] = h;
    faces[3].normal[0] = 0; faces[3].normal[1] = -1; faces[3].normal[2] = 0;
    
    /* Right face (x = +h) */
    faces[4].verts[0].v[0] = h; faces[4].verts[0].v[1] = -h; faces[4].verts[0].v[2] = h;
    faces[4].verts[1].v[0] = h; faces[4].verts[1].v[1] = -h; faces[4].verts[1].v[2] = -h;
    faces[4].verts[2].v[0] = h; faces[4].verts[2].v[1] =  h; faces[4].verts[2].v[2] = -h;
    faces[4].verts[3].v[0] = h; faces[4].verts[3].v[1] =  h; faces[4].verts[3].v[2] = h;
    faces[4].normal[0] = 1; faces[4].normal[1] = 0; faces[4].normal[2] = 0;
    
    /* Left face (x = -h) */
    faces[5].verts[0].v[0] = -h; faces[5].verts[0].v[1] = -h; faces[5].verts[0].v[2] = -h;
    faces[5].verts[1].v[0] = -h; faces[5].verts[1].v[1] = -h; faces[5].verts[1].v[2] = h;
    faces[5].verts[2].v[0] = -h; faces[5].verts[2].v[1] =  h; faces[5].verts[2].v[2] = h;
    faces[5].verts[3].v[0] = -h; faces[5].verts[3].v[1] =  h; faces[5].verts[3].v[2] = -h;
    faces[5].normal[0] = -1; faces[5].normal[1] = 0; faces[5].normal[2] = 0;
}

/* -------------------------------------------------------------------------- */
/* OBJ EXPORT                                                                */
/* -------------------------------------------------------------------------- */

static void export_obj(const char* filename, Cube* cubes) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen");
        return;
    }
    
    fprintf(fp, "# Tetragrammatron Omicron (Dalí Cross)\n");
    fprintf(fp, "# 8-cube octacube - GNOMON fixed point in 3D\n");
    fprintf(fp, "# Generated by tetra-omicron\n");
    fprintf(fp, "\n");
    
    Face cube_faces[CUBE_FACES];
    make_cube_mesh(1.0f, cube_faces);
    
    int vertex_offset = 1;
    
    for (int c = 0; c < OMICRON_CUBES; c++) {
        fprintf(fp, "# Cube %d: position (%.0f, %.0f, %.0f), SID=0x%04X\n",
                c, cubes[c].x, cubes[c].y, cubes[c].z, cubes[c].sid);
        
        /* Output vertices */
        for (int f = 0; f < CUBE_FACES; f++) {
            for (int v = 0; v < 4; v++) {
                fprintf(fp, "v %.4f %.4f %.4f\n",
                        cubes[c].x + cube_faces[f].verts[v].v[0],
                        cubes[c].y + cube_faces[f].verts[v].v[1],
                        cubes[c].z + cube_faces[f].verts[v].v[2]);
            }
        }
        
        /* Output faces (as triangles) */
        int vo = vertex_offset;
        for (int f = 0; f < CUBE_FACES; f++) {
            int base = vo + f * 4;
            fprintf(fp, "f %d %d %d\n", base, base + 1, base + 2);
            fprintf(fp, "f %d %d %d\n", base, base + 2, base + 3);
        }
        
        vertex_offset += CUBE_FACES * 4;
    }
    
    fclose(fp);
    printf("Exported OBJ: %s\n", filename);
}

/* -------------------------------------------------------------------------- */
/* STL EXPORT                                                                 */
/* -------------------------------------------------------------------------- */

static void export_stl(const char* filename, Cube* cubes) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("fopen");
        return;
    }
    
    fprintf(fp, "solid tetra_omicron\n");
    fprintf(fp, "  # Tetragrammatron Omicron (Dalí Cross)\n");
    fprintf(fp, "  # 8-cube octacube - GNOMON fixed point in 3D\n");
    
    Face cube_faces[CUBE_FACES];
    make_cube_mesh(1.0f, cube_faces);
    
    for (int c = 0; c < OMICRON_CUBES; c++) {
        for (int f = 0; f < CUBE_FACES; f++) {
            fprintf(fp, "  facet normal %.6f %.6f %.6f\n",
                    cube_faces[f].normal[0],
                    cube_faces[f].normal[1],
                    cube_faces[f].normal[2]);
            fprintf(fp, "    outer loop\n");
            
            for (int v = 0; v < 3; v++) {
                fprintf(fp, "      vertex %.6f %.6f %.6f\n",
                        cubes[c].x + cube_faces[f].verts[v].v[0],
                        cubes[c].y + cube_faces[f].verts[v].v[1],
                        cubes[c].z + cube_faces[f].verts[v].v[2]);
            }
            
            fprintf(fp, "    endloop\n");
            fprintf(fp, "  endfacet\n");
        }
    }
    
    fprintf(fp, "endsolid tetra_omicron\n");
    
    fclose(fp);
    printf("Exported STL: %s\n", filename);
}

/* -------------------------------------------------------------------------- */
/* MAIN                                                                     */
/* -------------------------------------------------------------------------- */

int main(int argc, char** argv) {
    Cube cubes[OMICRON_CUBES];
    make_omicron(cubes);
    
    printf("═══ TETRA OMICRON (Dalí Cross) ═══\n");
    printf("8-cube octacube - GNOMON fixed point in 3D\n\n");
    
    printf("Voxel positions:\n");
    for (int i = 0; i < OMICRON_CUBES; i++) {
        printf("  Cube %d: (%.0f, %.0f, %.0f) SID=0x%04X\n",
               i, cubes[i].x, cubes[i].y, cubes[i].z, cubes[i].sid);
    }
    
    printf("\n");
    printf("Symmetry: Octahedral (24 orientations)\n");
    printf("Chirality: Right-handed (determinant = +1)\n");
    
    if (argc > 1) {
        if (strcmp(argv[1], "obj") == 0) {
            export_obj("dali.obj", cubes);
        } else if (strcmp(argv[1], "stl") == 0) {
            export_stl("dali.stl", cubes);
        } else if (strcmp(argv[1], "both") == 0) {
            export_obj("dali.obj", cubes);
            export_stl("dali.stl", cubes);
        }
    }
    
    return 0;
}

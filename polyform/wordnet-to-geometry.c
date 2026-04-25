/*
 * WORDNET TO GEOMETRY STREAM CONVERTER
 * ====================================
 * 
 * Converts WordNet Prolog files to Aegean geometry streams
 * for visualization in the WebGL engine.
 * 
 * Compile: gcc -o wordnet-to-geometry wordnet-to-geometry.c -lm
 * Run: ./wordnet-to-geometry [wordnet_path]
 * Output: JSON stream for WebGL engine
 * 
 * Architecture:
 * 1. Parse wn_s.pl for synsets (words + POS)
 * 2. Parse wn_g.pl for glosses
 * 3. Parse wn_hyp.pl for hypernym relations
 * 4. Compute depth via BFS from roots
 * 5. Map depth+POS to Aegean geometry
 * 6. Output JSON stream
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#define MAX_LINE      4096
#define MAX_SYNSETS   120000
#define MAX_HYPERNYMS 200000
#define MAX_WORDS     500000
#define MAX_DEPTH     20
#define MAX_GEOM_STEP 64

/* Aegean Unicode block: U+10100 - U+1013F */
#define AEGEAN_BASE   0x10100

/* Geometry names for JSON output */
const char* geometry_names[] = {
    "point",        /* 0 */
    "line",         /* 1 */
    "triangle",     /* 2 */
    "tetrahedron",  /* 3 */
    "5-cell",       /* 4 */
    "8-cell",       /* 5 */
    "16-cell",      /* 6 */
    "24-cell",      /* 7 */
    "120-cell",     /* 8 */
    "600-cell",     /* 9 */
    "hopf-fiber",   /* 10 */
    "s7",          /* 11 */
    "s15",         /* 12 */
    "s31",         /* 13 */
    "s63",         /* 14 */
    "s127"         /* 15 */
};

/* Synset structure */
typedef struct {
    int synset_id;
    char pos;           /* n, v, a, r, s */
    int word_count;
    char words[32][64]; /* First few words */
    char gloss[1024];
    int depth;
    int geometry_idx;
    int hypernym_count;
    int hypernyms[16];
} Synset;

/* Hypernym relation */
typedef struct {
    int child;
    int parent;
} Hypernym;

/* Global data */
Synset synsets[MAX_SYNSETS];
int synset_count = 0;

Hypernym hypernyms[MAX_HYPERNYMS];
int hypernym_count = 0;

/* Synset lookup by ID */
int synset_index[MAX_SYNSETS * 10];  /* Simple hash */
int synset_lookup[MAX_SYNSETS * 10];

/* Statistics */
int depth_histogram[MAX_DEPTH] = {0};
int pos_histogram[256] = {0};

/* Parse integer from string */
int parse_int(char *s, int *out) {
    char *end;
    *out = (int)strtol(s, &end, 10);
    return end != s;
}

/* Extract number from s/5 predicate */
int extract_synset_id(char *line) {
    int id = 0;
    if (sscanf(line, "s(%d,", &id) == 1) return id;
    return -1;
}

/* Extract word from s/5 predicate */
void extract_word(char *line, char *word) {
    char *p = strchr(line, ',');
    if (!p) { strcpy(word, ""); return; }
    p++;
    p = strchr(p, ',');
    if (!p) { strcpy(word, ""); return; }
    p++;
    while (*p == ' ') p++;
    char *q = strchr(p, ',');
    if (!q) { strcpy(word, ""); return; }
    *q = '\0';
    strcpy(word, p);
    *q = ',';
}

/* Extract POS from s/5 predicate */
char extract_pos(char *line) {
    char pos = 'n';
    char *p = strrchr(line, ',');
    if (p) {
        p--;
        while (p > line && *p != ',') p--;
        if (*p == ',') p++;
        if (*p == '\'') p++;
        pos = *p;
    }
    return pos;
}

/* Extract gloss from g/2 predicate */
void extract_gloss(char *line, char *gloss) {
    char *p = strchr(line, ',');
    if (!p) { strcpy(gloss, ""); return; }
    p++;
    while (*p == ' ') p++;
    if (*p == '\'') p++;
    char *q = strrchr(p, '\'');
    if (q) *q = '\0';
    strcpy(gloss, p);
}

/* Extract hypernym relation */
int extract_hypernym(char *line, int *child, int *parent) {
    return sscanf(line, "hyp(%d,%d)", child, parent) == 2;
}

/* Hash function for synset ID */
int synset_hash(int id) {
    return (id / 1000) % (MAX_SYNSETS * 10);
}

/* Add synset to lookup */
void add_synset_lookup(int id, int idx) {
    int h = synset_hash(id);
    while (synset_lookup[h] != -1) {
        h = (h + 1) % (MAX_SYNSETS * 10);
    }
    synset_lookup[h] = idx;
    synset_index[h] = id;
}

/* Find synset by ID */
int find_synset(int id) {
    int h = synset_hash(id);
    int attempts = 0;
    while (synset_index[h] != id && synset_index[h] != -1 && attempts < MAX_SYNSETS * 10) {
        h = (h + 1) % (MAX_SYNSETS * 10);
        attempts++;
    }
    if (synset_index[h] == id) return synset_lookup[h];
    return -1;
}

/* Map depth to geometry index */
int depth_to_geometry(int depth, char pos) {
    int base;
    switch (pos) {
        case 'n': base = 0; break;  /* Noun: point starts */
        case 'v': base = 1; break;  /* Verb: line starts */
        case 'a': base = 2; break;  /* Adjective: triangle starts */
        case 'r': base = 3; break;  /* Adverb: tetrahedron starts */
        default:   base = 0;
    }
    int idx = base + depth;
    if (idx > 15) idx = 15;
    return idx;
}

/* Compute depth via BFS from roots */
void compute_depths() {
    int queue[MAX_SYNSETS];
    int q_head = 0, q_tail = 0;
    int visited[MAX_SYNSETS] = {0};
    
    /* Find root synsets (those with no hypernyms) */
    /* For each synset, check if it's a child in any hypernym */
    int is_child[MAX_SYNSETS] = {0};
    for (int i = 0; i < hypernym_count; i++) {
        if (hypernyms[i].child < MAX_SYNSETS * 10) {
            is_child[hypernyms[i].child] = 1;
        }
    }
    
    /* Find roots: synsets that are not children */
    for (int i = 0; i < synset_count; i++) {
        if (!is_child[synsets[i].synset_id]) {
            synsets[i].depth = 0;
            queue[q_tail++] = i;
            visited[i] = 1;
        }
    }
    
    /* BFS */
    while (q_head < q_tail) {
        int idx = queue[q_head++];
        Synset *s = &synsets[idx];
        
        /* Find all children of this synset */
        for (int i = 0; i < hypernym_count; i++) {
            if (hypernyms[i].parent == s->synset_id) {
                int child_idx = find_synset(hypernyms[i].child);
                if (child_idx >= 0 && !visited[child_idx]) {
                    synsets[child_idx].depth = s->depth + 1;
                    visited[child_idx] = 1;
                    queue[q_tail++] = child_idx;
                }
            }
        }
    }
    
    /* Count histogram */
    for (int i = 0; i < synset_count; i++) {
        int d = synsets[i].depth;
        if (d >= MAX_DEPTH) d = MAX_DEPTH - 1;
        depth_histogram[d]++;
        pos_histogram[(int)synsets[i].pos]++;
    }
}

/* Generate geometry vertices for a synset */
void generate_geometry(int geom_idx, float vertices[][3], int *vertex_count) {
    *vertex_count = 0;
    
    switch (geom_idx) {
        case 0: /* point */
            vertices[0][0] = 0; vertices[0][1] = 0; vertices[0][2] = 0;
            *vertex_count = 1;
            break;
            
        case 1: /* line */
            vertices[0][0] = -1; vertices[0][1] = 0; vertices[0][2] = 0;
            vertices[1][0] = 1; vertices[1][1] = 0; vertices[1][2] = 0;
            *vertex_count = 2;
            break;
            
        case 2: /* triangle */
            vertices[0][0] = 0; vertices[0][1] = 1; vertices[0][2] = 0;
            vertices[1][0] = -1; vertices[1][1] = -0.5; vertices[1][2] = 0;
            vertices[2][0] = 1; vertices[2][1] = -0.5; vertices[2][2] = 0;
            *vertex_count = 3;
            break;
            
        case 3: /* tetrahedron */
            vertices[0][0] = 1; vertices[0][1] = 1; vertices[0][2] = 1;
            vertices[1][0] = -1; vertices[1][1] = -1; vertices[1][2] = 1;
            vertices[2][0] = -1; vertices[2][1] = 1; vertices[2][2] = -1;
            vertices[3][0] = 1; vertices[3][1] = -1; vertices[3][2] = -1;
            *vertex_count = 4;
            break;
            
        case 4: /* 5-cell (simplex) */
            {
                float phi = (1.0f + sqrtf(5.0f)) / 4.0f;
                vertices[0][0] = 1; vertices[0][1] = phi; vertices[0][2] = 0;
                vertices[1][0] = 1; vertices[1][1] = -phi; vertices[1][2] = 0;
                vertices[2][0] = -1; vertices[2][1] = phi; vertices[2][2] = 0;
                vertices[3][0] = -1; vertices[3][1] = -phi; vertices[3][2] = 0;
                vertices[4][0] = 0; vertices[4][1] = 0; vertices[4][2] = 2 * phi;
                *vertex_count = 5;
            }
            break;
            
        case 5: /* 8-cell (tesseract) - project to 3D */
            {
                int idx = 0;
                for (int i = 0; i < 2; i++) {
                    for (int j = 0; j < 2; j++) {
                        for (int k = 0; k < 2; k++) {
                            vertices[idx][0] = (i * 2 - 1) * 0.7f;
                            vertices[idx][1] = (j * 2 - 1) * 0.7f;
                            vertices[idx][2] = (k * 2 - 1) * 0.7f;
                            idx++;
                        }
                    }
                }
                *vertex_count = 16;
            }
            break;
            
        default:
            /* Higher cells: generate sphere points */
            {
                int n = 8 + (geom_idx - 5) * 4;
                for (int i = 0; i < n && i < 32; i++) {
                    float theta = 2.0f * 3.14159f * i / n;
                    float phi = 3.14159f * i / (2 * n);
                    vertices[i][0] = sinf(phi) * cosf(theta) * 0.5f;
                    vertices[i][1] = sinf(phi) * sinf(theta) * 0.5f;
                    vertices[i][2] = cosf(phi) * 0.5f;
                }
                *vertex_count = n < 32 ? n : 32;
            }
    }
}

/* Output JSON */
void output_json(int limit) {
    printf("{\n");
    printf("  \"source\": \"WordNet Prolog\",\n");
    printf("  \"mapping\": \"hypernym_depth_to_aegean_geometry\",\n");
    printf("  \"synset_count\": %d,\n", synset_count);
    printf("  \"hypernym_count\": %d,\n", hypernym_count);
    
    printf("  \"depth_histogram\": [");
    for (int i = 0; i < MAX_DEPTH; i++) {
        printf("%d%s", depth_histogram[i], i < MAX_DEPTH - 1 ? ", " : "");
    }
    printf("],\n");
    
    printf("  \"pos_histogram\": {\"n\": %d, \"v\": %d, \"a\": %d, \"r\": %d},\n",
           pos_histogram['n'], pos_histogram['v'], pos_histogram['a'], pos_histogram['r']);
    
    printf("  \"synsets\": [\n");
    
    int out_count = limit > synset_count ? synset_count : limit;
    for (int i = 0; i < out_count; i++) {
        Synset *s = &synsets[i];
        int geom_idx = depth_to_geometry(s->depth, s->pos);
        
        printf("    {\n");
        printf("      \"synset_id\": %d,\n", s->synset_id);
        printf("      \"pos\": \"%c\",\n", s->pos);
        printf("      \"depth\": %d,\n", s->depth);
        printf("      \"geometry\": \"%s\",\n", geometry_names[geom_idx]);
        printf("      \"geometry_idx\": %d,\n", geom_idx);
        printf("      \"aegean\": \"U+%04X\",\n", AEGEAN_BASE + geom_idx);
        printf("      \"words\": [");
        for (int w = 0; w < s->word_count && w < 5; w++) {
            printf("\"%s\"%s", s->words[w], w < s->word_count - 1 && w < 4 ? ", " : "");
        }
        printf("],\n");
        
        /* Generate vertices */
        float verts[32][3];
        int vcount;
        generate_geometry(geom_idx, verts, &vcount);
        
        printf("      \"vertices\": [");
        for (int v = 0; v < vcount; v++) {
            printf("[%.3f, %.3f, %.3f]%s", 
                   verts[v][0], verts[v][1], verts[v][2],
                   v < vcount - 1 ? ", " : "");
        }
        printf("],\n");
        
        /* Gloss preview */
        char gloss_preview[256];
        int gloss_len = strlen(s->gloss);
        if (gloss_len > 200) {
            strncpy(gloss_preview, s->gloss, 197);
            gloss_preview[197] = '\0';
            strcat(gloss_preview, "...");
        } else {
            strcpy(gloss_preview, s->gloss);
        }
        
        printf("      \"gloss\": \"%s\"\n", gloss_preview);
        printf("    }%s\n", i < out_count - 1 ? "," : "");
    }
    
    printf("  ]\n");
    printf("}\n");
}

/* Output word search stream */
void output_word_stream(const char *search_word) {
    printf("# WordNet Geometry Stream for: %s\n\n", search_word);
    
    for (int i = 0; i < synset_count; i++) {
        Synset *s = &synsets[i];
        
        /* Check if this synset contains our word */
        int found = 0;
        for (int w = 0; w < s->word_count; w++) {
            if (strcasecmp(s->words[w], search_word) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) continue;
        
        int geom_idx = depth_to_geometry(s->depth, s->pos);
        
        printf("# Synset %d, Depth %d, POS %c, Geometry %s\n",
               s->synset_id, s->depth, s->pos, geometry_names[geom_idx]);
        
        /* Generate the geometry stream for hypernym path */
        printf("(FS ");
        
        /* Walk up the hypernym chain */
        int current = i;
        int path_len = 0;
        int path[MAX_DEPTH];
        
        while (current >= 0 && path_len < MAX_DEPTH) {
            path[path_len++] = current;
            
            /* Find first hypernym parent */
            int next = -1;
            for (int h = 0; h < hypernym_count; h++) {
                if (hypernyms[h].child == synsets[current].synset_id) {
                    next = find_synset(hypernyms[h].parent);
                    break;
                }
            }
            if (next < 0) break;
            current = next;
        }
        
        /* Output in order: root to leaf */
        for (int p = path_len - 1; p >= 0; p++) {
            Synset *ps = &synsets[p];
            int gi = depth_to_geometry(ps->depth, ps->pos);
            printf("U+%04X ", AEGEAN_BASE + gi);
        }
        
        printf("GS %s RS U+%04X ETX)\n", search_word, AEGEAN_BASE + geom_idx);
        
        /* Show hypernym path */
        printf("# Path: ");
        for (int p = path_len - 1; p >= 0; p--) {
            Synset *ps = &synsets[p];
            if (p < path_len - 1) printf(" -> ");
            printf("%s(%d)", ps->words[0], ps->depth);
        }
        printf("\n\n");
    }
}

int main(int argc, char *argv[]) {
    const char *wn_path = "/home/main/Downloads/WNprolog-3.0/prolog";
    const char *search_word = NULL;
    int limit = 100;
    
    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
            search_word = argv[++i];
        } else if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            limit = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            wn_path = argv[++i];
        } else if (strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s [-p path] [-w word] [-n count] [-h]\n", argv[0]);
            printf("  -p path   : WordNet prolog directory (default: /home/main/Downloads/WNprolog-3.0/prolog)\n");
            printf("  -w word   : Search for word and output its geometry stream\n");
            printf("  -n count  : Number of synsets to output (default: 100)\n");
            printf("  -h        : Show this help\n");
            return 0;
        }
    }
    
    printf("# WordNet to Geometry Stream Converter\n");
    printf("# WordNet path: %s\n", wn_path);
    printf("# Loading...\n\n");
    
    /* Initialize lookup tables */
    for (int i = 0; i < MAX_SYNSETS * 10; i++) {
        synset_lookup[i] = -1;
        synset_index[i] = -1;
    }
    
    /* Load synsets from wn_s.pl */
    char filename[512];
    char line[MAX_LINE];
    sprintf(filename, "%s/wn_s.pl", wn_path);
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Error: Cannot open %s\n", filename);
        return 1;
    }
    
    int lines_read = 0;
    while (fgets(line, sizeof(line), f) && synset_count < MAX_SYNSETS) {
        lines_read++;
        if (lines_read % 100000 == 0) {
            printf("# Processed %d lines...\n", lines_read);
            fflush(stdout);
        }
        
        if (strncmp(line, "s(", 2) != 0) continue;
        
        /* Quick parse: extract first word only */
        int id;
        char pos;
        char word[64] = "";
        
        if (sscanf(line, "s(%d,%*d,'%63[^']',%c", &id, word, &pos) >= 1) {
            Synset *s = &synsets[synset_count];
            s->synset_id = id;
            s->pos = (sscanf(line + strlen(line) - 3, "%c", &pos) > 0) ? pos : 'n';
            s->word_count = 1;
            strncpy(s->words[0], word, 63);
            s->words[0][63] = '\0';
            s->depth = -1;
            s->hypernym_count = 0;
            s->gloss[0] = '\0';
            
            add_synset_lookup(id, synset_count);
            synset_count++;
        }
    }
    fclose(f);
    printf("# Loaded %d synsets from wn_s.pl (%d lines read)\n", synset_count, lines_read);
    
    /* Load glosses from wn_g.pl */
    sprintf(filename, "%s/wn_g.pl", wn_path);
    f = fopen(filename, "r");
    if (f) {
        while (fgets(line, sizeof(line), f)) {
            if (strncmp(line, "g(", 2) == 0) {
                int id;
                char gloss[MAX_LINE];
                extract_gloss(line, gloss);
                if (sscanf(line, "g(%d,", &id) == 1) {
                    int idx = find_synset(id);
                    if (idx >= 0) {
                        strncpy(synsets[idx].gloss, gloss, 1023);
                    }
                }
            }
        }
        fclose(f);
    }
    printf("# Loaded glosses from wn_g.pl\n");
    
    /* Load hypernyms from wn_hyp.pl */
    sprintf(filename, "%s/wn_hyp.pl", wn_path);
    f = fopen(filename, "r");
    if (f) {
        while (fgets(line, sizeof(line), f) && hypernym_count < MAX_HYPERNYMS) {
            if (strncmp(line, "hyp(", 4) == 0) {
                int child, parent;
                if (extract_hypernym(line, &child, &parent)) {
                    hypernyms[hypernym_count].child = child;
                    hypernyms[hypernym_count].parent = parent;
                    hypernym_count++;
                }
            }
        }
        fclose(f);
    }
    printf("# Loaded %d hypernym relations from wn_hyp.pl\n", hypernym_count);
    
    /* Compute depths */
    printf("# Computing hypernym depths...\n");
    compute_depths();
    
    /* Output results */
    if (search_word) {
        output_word_stream(search_word);
    } else {
        output_json(limit);
    }
    
    return 0;
}

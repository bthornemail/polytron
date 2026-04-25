/*
 * WORDNET GEOMETRY LOOKUP
 * ======================
 * 
 * Fast lookup: find word geometry stream from WordNet.
 * No in-memory graph - just grep + parse.
 * 
 * Compile: gcc -o wn-geometry wn-geometry.c -lm
 * Run: ./wn-geometry [word]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AEGEAN_BASE 0x10100

const char* depth_to_geom(int d) {
    static const char* geoms[] = {
        "point", "line", "triangle", "tetrahedron", "5-cell",
        "8-cell", "16-cell", "24-cell", "120-cell", "600-cell"
    };
    return geoms[d > 9 ? 9 : d];
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s word\n", argv[0]);
        return 1;
    }
    
    const char *word = argv[1];
    char cmd[256];
    char line[4096];
    FILE *f;
    
    printf("# WordNet Geometry Stream\n");
    printf("# Word: %s\n\n", word);
    
    /* Find synsets containing this word */
    snprintf(cmd, sizeof(cmd), 
             "grep -E \"^s\\([0-9]+,[0-9]+,'%s',\" /home/main/Downloads/WNprolog-3.0/prolog/wn_s.pl 2>/dev/null",
             word);
    
    f = popen(cmd, "r");
    if (!f) {
        fprintf(stderr, "Error running grep\n");
        return 1;
    }
    
    int count = 0;
    while (fgets(line, sizeof(line), f) && count < 20) {
        int synset_id, wnum;
        char pos[4];
        char found_word[128];
        
        if (sscanf(line, "s(%d,%d,'%127[^']',%3s", &synset_id, &wnum, found_word, pos) >= 4) {
            /* Try to find hypernym depth */
            char hyp_cmd[256];
            snprintf(hyp_cmd, sizeof(hyp_cmd),
                    "grep '^hyp(%d,' /home/main/Downloads/WNprolog-3.0/prolog/wn_hyp.pl | wc -l 2>/dev/null",
                    synset_id);
            
            FILE *hf = popen(hyp_cmd, "r");
            int depth = 0;
            if (hf) {
                char buf[32];
                if (fgets(buf, sizeof(buf), hf)) {
                    depth = atoi(buf);
                }
                pclose(hf);
            }
            
            /* Find gloss */
            snprintf(hyp_cmd, sizeof(hyp_cmd),
                    "grep '^g(%d,' /home/main/Downloads/WNprolog-3.0/prolog/wn_g.pl 2>/dev/null | head -1");
            snprintf(hyp_cmd, sizeof(hyp_cmd),
                    "sed -n 's/.*g(%d,.*//p' /home/main/Downloads/WNprolog-3.0/prolog/wn_g.pl 2>/dev/null | head -1",
                    synset_id);
            
            printf("(FS ");
            for (int i = 0; i <= depth && i < 10; i++) {
                printf("U+%04X ", AEGEAN_BASE + i);
            }
            printf("GS %s RS U+%04X ETX)\n", word, AEGEAN_BASE + depth);
            printf("# Synset %d, POS %s, Depth %d, Geometry %s\n", 
                   synset_id, pos, depth, depth_to_geom(depth));
            
            count++;
        }
    }
    pclose(f);
    
    if (count == 0) {
        printf("# No synsets found for: %s\n", word);
    }
    
    return 0;
}
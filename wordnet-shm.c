/*
 * wordnet-shm.c - WordNet IVSHMEM for live queries
 *
 * Maps WordNet relations into shared memory for zero-copy access
 * from QEMU guest via ivshmem-plain device.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define CONSTITUTIONAL_C  0x1D
#define MAX_SYNSETS       250000
#define MAX_RELATIONS     200000

typedef uint32_t Pair;

#define cons(a,d) ((((a) & 0xFF) << 8) | ((d) & 0xFF))
#define car(p)    (((p) >> 8) & 0xFF)
#define cdr(p)    ((p) & 0xFF)

typedef struct {
    Pair synset_id;
    Pair lemma_ptr;
    uint8_t pos;
    uint16_t sense_count;
    uint16_t hr_count;
} SynsetEntry;

typedef struct {
    Pair child;
    Pair parent;
    uint8_t relation_type;
} RelationEntry;

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t synset_count;
    uint32_t relation_count;
    uint32_t hypernym_count;
    uint32_t antonym_count;
    uint32_t meronym_count;
    
    SynsetEntry synsets[MAX_SYNSETS];
    RelationEntry relations[MAX_RELATIONS];
    
    char string_table[];
} WordNetSHM;

static WordNetSHM *wn_shm = NULL;

static uint32_t rotl32(uint32_t x, int n)
{
    n &= 31;
    return (x << n) | (x >> (32 - n));
}

static uint32_t rotr32(uint32_t x, int n)
{
    n &= 31;
    return (x >> n) | (x << (32 - n));
}

static uint32_t K(uint32_t p, uint32_t C)
{
    return rotl32(p, 1) ^ rotl32(p, 3) ^ rotr32(p, 2) ^ C;
}

static Pair compute_sid(Pair p)
{
    return K(p, CONSTITUTIONAL_C);
}

bool wordnet_shm_init(const char *shm_path)
{
    FILE *f = fopen(shm_path, "r+b");
    if (!f) {
        fprintf(stderr, "Warning: Could not open %s, creating new\n", shm_path);
        f = fopen(shm_path, "w+b");
        if (!f) {
            perror("fopen");
            return false;
        }
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    
    if (size < sizeof(WordNetSHM)) {
        size = sizeof(WordNetSHM);
        ftruncate(fileno(f), size);
    }
    
    wn_shm = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fileno(f), 0);
    fclose(f);
    
    if (wn_shm == MAP_FAILED) {
        perror("mmap");
        return false;
    }
    
    if (wn_shm->magic == 0) {
        wn_shm->magic = 0xDEADBEEF;
        wn_shm->version = 1;
        wn_shm->synset_count = 0;
        wn_shm->relation_count = 0;
    }
    
    return true;
}

int wordnet_add_synset(Pair id, const char *lemma, uint8_t pos)
{
    if (!wn_shm || wn_shm->synset_count >= MAX_SYNSETS)
        return -1;
    
    SynsetEntry *e = &wn_shm->synsets[wn_shm->synset_count++];
    e->synset_id = id;
    e->pos = pos;
    e->sense_count = 1;
    e->hr_count = 0;
    
    return wn_shm->synset_count - 1;
}

int wordnet_add_hypernym(Pair child, Pair parent)
{
    if (!wn_shm || wn_shm->relation_count >= MAX_RELATIONS)
        return -1;
    
    RelationEntry *r = &wn_shm->relations[wn_shm->relation_count++];
    r->child = child;
    r->parent = parent;
    r->relation_type = 'H';  /* hypernym */
    
    wn_shm->hypernym_count++;
    
    return wn_shm->relation_count - 1;
}

int wordnet_add_antonym(Pair src, Pair tgt)
{
    if (!wn_shm || wn_shm->relation_count >= MAX_RELATIONS)
        return -1;
    
    RelationEntry *r = &wn_shm->relations[wn_shm->relation_count++];
    r->child = src;
    r->parent = tgt;
    r->relation_type = 'A';  /* antonym */
    
    wn_shm->antonym_count++;
    
    return wn_shm->relation_count - 1;
}

int wordnet_add_meronym(Pair whole, Pair part)
{
    if (!wn_shm || wn_shm->relation_count >= MAX_RELATIONS)
        return -1;
    
    RelationEntry *r = &wn_shm->relations[wn_shm->relation_count++];
    r->child = whole;
    r->parent = part;
    r->relation_type = 'M';  /* meronym */
    
    wn_shm->meronym_count++;
    
    return wn_shm->relation_count - 1;
}

int query_hypernyms(Pair synset, Pair *results, int max_results)
{
    int count = 0;
    for (uint32_t i = 0; i < wn_shm->relation_count && count < max_results; i++) {
        RelationEntry *r = &wn_shm->relations[i];
        if (r->child == synset && r->relation_type == 'H') {
            results[count++] = r->parent;
        }
    }
    return count;
}

int query_antonyms(Pair synset, Pair *results, int max_results)
{
    int count = 0;
    for (uint32_t i = 0; i < wn_shm->relation_count && count < max_results; i++) {
        RelationEntry *r = &wn_shm->relations[i];
        if (r->child == synset && r->relation_type == 'A') {
            results[count++] = r->parent;
        }
    }
    return count;
}

SynsetEntry* wordnet_lookup(Pair id)
{
    for (uint32_t i = 0; i < wn_shm->synset_count; i++) {
        if (wn_shm->synsets[i].synset_id == id)
            return &wn_shm->synsets[i];
    }
    return NULL;
}

Pair wordnet_sid(Pair id)
{
    SynsetEntry *s = wordnet_lookup(id);
    return s ? compute_sid(s->synset_id) : 0;
}

void wordnet_stats(void)
{
    if (!wn_shm) {
        printf("WordNet SHM not initialized\n");
        return;
    }
    
    printf("=== WordNet Statistics ===\n");
    printf("Synsets:     %u\n", wn_shm->synset_count);
    printf("Relations:   %u\n", wn_shm->relation_count);
    printf("  Hypernyms: %u\n", wn_shm->hypernym_count);
    printf("  Antonyms:  %u\n", wn_shm->antonym_count);
    printf("  Meronyms:  %u\n", wn_shm->meronym_count);
}

int main(int argc, char *argv[])
{
    const char *shm_path = "/dev/shm/wordnet.db";
    
    if (argc > 1)
        shm_path = argv[1];
    
    printf("Initializing WordNet SHM at %s\n", shm_path);
    
    if (!wordnet_shm_init(shm_path)) {
        fprintf(stderr, "Failed to initialize WordNet SHM\n");
        return 1;
    }
    
    wordnet_stats();
    
    Pair results[10];
    int n = query_hypernyms(100007846, results, 10);
    printf("\nHypernyms of person (100007846):\n");
    for (int i = 0; i < n; i++) {
        SynsetEntry *s = wordnet_lookup(results[i]);
        printf("  %u -> SID 0x%04X\n", results[i], wordnet_sid(results[i]));
    }
    
    return 0;
}

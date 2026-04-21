/*
 * TETRA-NRR — Non-Repudiation Receipt Append-Only Log
 *
 * An append-only log where every entry is a constitutional receipt.
 * Each receipt contains:
 *   - Entry SID (hash of content)
 *   - Previous SID (chain linkage)
 *   - Timestamp (cryptographic)
 *   - Signature SID (issuer verification)
 *
 * The log is immutable: you can only append, never modify or delete.
 * Verification is through SID chain computation.
 */

#ifndef TETRA_NRR_H
#define TETRA_NRR_H

#include <stdint.h>
#include "tetra-block.h"

/* -------------------------------------------------------------------------- */
/* NRR CONSTANTS                                                             */
/* -------------------------------------------------------------------------- */

#define NRR_MAGIC     0x4E525230  /* "NRR0" */
#define NRR_VERSION  1
#define MAX_ENTRIES  65536

/* -------------------------------------------------------------------------- */
/* NRR ENTRY                                                            */
/* -------------------------------------------------------------------------- */

typedef struct {
    Pair entry_sid;       /* Hash of (content + timestamp) */
    Pair prev_sid;       /* Previous entry's chain SID */
    Pair confirm_sid;    /* Confirmation SID */
    uint8_t flags;      /* Entry flags */
    uint8_t issuer;     /* Issuer ID (0-255) */
    uint32_t timestamp; /* Cryptographic timestamp */
    uint8_t content_len;
    uint8_t content[240];
} NRREntry;

/* Create an NRR entry */
static NRREntry make_entry(Pair content_sid, Pair prev, uint8_t issuer, 
                          const uint8_t *content, int len) {
    NRREntry e;
    e.entry_sid = content_sid;
    e.prev_sid = prev;
    e.confirm_sid = 0;
    e.flags = 0;
    e.issuer = issuer;
    e.timestamp = 0;
    e.content_len = (uint8_t)(len < 240 ? len : 240);
    for (int i = 0; i < len && i < 240; i++) {
        e.content[i] = content[i];
    }
    return e;
}

/* -------------------------------------------------------------------------- */
/* NRR LOG HEADER                                                          */
/* -------------------------------------------------------------------------- */

typedef struct {
    uint32_t magic;         /* NRR_MAGIC */
    uint8_t  version;      /* NRR_VERSION */
    uint8_t  entry_count;  /* Number of entries (mod 256) */
    Pair root_sid;         /* First entry's SID */
    Pair tip_sid;         /* Current tip SID */
    Pair genesis_sid;     /* Genesis SID */
    uint32_t crc;         /* Header CRC */
} NRRHeader;

/* Create genesis entry */
static Pair nrr_genesis(void) {
    /* Genesis is K(0, GS) - the constitutional constant */
    return K((Pair)0, 0x1D);
}

/* Initialize an NRR log header */
static void nrr_init(NRRHeader *hdr) {
    hdr->magic = NRR_MAGIC;
    hdr->version = NRR_VERSION;
    hdr->entry_count = 0;
    hdr->root_sid = nrr_genesis();
    hdr->tip_sid = hdr->root_sid;
    hdr->genesis_sid = hdr->root_sid;
    hdr->crc = 0;
}

/* Compute header CRC */
static uint32_t nrr_crc(const NRRHeader *hdr) {
    uint32_t crc = hdr->magic ^ hdr->version;
    crc ^= (hdr->root_sid << 8) ^ hdr->tip_sid;
    crc ^= (hdr->genesis_sid << 4) ^ (hdr->entry_count * 17);
    return crc;
}

/* -------------------------------------------------------------------------- */
/* NRR APPEND OPERATION                                                    */
/* -------------------------------------------------------------------------- */

typedef struct {
    NRRHeader header;
    NRREntry entries[MAX_ENTRIES];
    uint32_t count;
} NRRLog;

/* Initialize log */
static void nrr_log_init(NRRLog *log) {
    nrr_init(&log->header);
    log->count = 0;
}

/* Append an entry */
static Pair nrr_append(NRRLog *log, const uint8_t *content, int len, uint8_t issuer) {
    if (log->count >= MAX_ENTRIES) return 0;
    
    /* Compute entry SID from content */
    Pair content_sid = compute_sid(len >= 2 ? 
        ((Pair)content[0] << 8) | content[1] : 
        ((Pair)content[0] << 8));
    
    /* Create entry with previous tip as prev_sid */
    NRREntry entry = make_entry(content_sid, log->header.tip_sid, issuer, content, len);
    
    /* Generate timestamp from previous tip and new content */
    entry.timestamp = (uint32_t)(log->header.tip_sid ^ content_sid);
    
    /* Compute confirmation SID */
    entry.confirm_sid = K(entry.entry_sid, entry.timestamp);
    
    /* Update tip */
    log->header.tip_sid = K(log->header.tip_sid, entry.confirm_sid);
    
    log->entries[log->count++] = entry;
    log->header.entry_count = (uint8_t)log->count;
    
    return entry.confirm_sid;
}

/* Verify the entire log */
static int nrr_verify(NRRLog *log) {
    if (log->count == 0) return 1;
    
    Pair expected_tip = log->header.root_sid;
    
    for (uint32_t i = 0; i < log->count; i++) {
        NRREntry *e = &log->entries[i];
        
        /* Verify linkage */
        if (e->prev_sid != expected_tip) return 0;
        
        /* Verify confirmation */
        Pair confirm = K(e->entry_sid, e->timestamp);
        if (confirm != e->confirm_sid) return 0;
        
        /* Advance tip */
        expected_tip = K(expected_tip, e->confirm_sid);
    }
    
    return expected_tip == log->header.tip_sid;
}

/* Get entry by index */
static NRREntry *nrr_get(NRRLog *log, uint32_t index) {
    if (index >= log->count) return NULL;
    return &log->entries[index];
}

/* Find entry by confirmation SID */
static NRREntry *nrr_find(NRRLog *log, Pair confirm_sid) {
    for (uint32_t i = 0; i < log->count; i++) {
        if (log->entries[i].confirm_sid == confirm_sid) {
            return &log->entries[i];
        }
    }
    return NULL;
}

/* -------------------------------------------------------------------------- */
/* RECEIPT SERIALIZATION                                                     */
/* -------------------------------------------------------------------------- */

#define MAX_RECEIPT_SIZE (4 + 2 + 2 + 1 + 1 + 4 + 1 + 240)

/* Serialize log to bytes */
static int nrr_serialize(NRRLog *log, uint8_t *out) {
    int len = 0;
    
    /* Header */
    out[len++] = (log->header.magic >> 24) & 0xFF;
    out[len++] = (log->header.magic >> 16) & 0xFF;
    out[len++] = (log->header.magic >> 8) & 0xFF;
    out[len++] = log->header.magic & 0xFF;
    out[len++] = log->header.version;
    out[len++] = log->header.entry_count;
    out[len++] = (log->header.root_sid >> 8) & 0xFF;
    out[len++] = log->header.root_sid & 0xFF;
    out[len++] = (log->header.tip_sid >> 8) & 0xFF;
    out[len++] = log->header.tip_sid & 0xFF;
    out[len++] = (log->header.genesis_sid >> 8) & 0xFF;
    out[len++] = log->header.genesis_sid & 0xFF;
    
    /* Entries */
    for (uint32_t i = 0; i < log->count; i++) {
        NRREntry *e = &log->entries[i];
        out[len++] = (e->entry_sid >> 8) & 0xFF;
        out[len++] = e->entry_sid & 0xFF;
        out[len++] = (e->prev_sid >> 8) & 0xFF;
        out[len++] = e->prev_sid & 0xFF;
        out[len++] = e->flags;
        out[len++] = e->issuer;
        out[len++] = (e->timestamp >> 24) & 0xFF;
        out[len++] = (e->timestamp >> 16) & 0xFF;
        out[len++] = (e->timestamp >> 8) & 0xFF;
        out[len++] = e->timestamp & 0xFF;
        out[len++] = e->content_len;
        for (int j = 0; j < e->content_len && j < 240; j++) {
            out[len++] = e->content[j];
        }
    }
    
    return len;
}

/* -------------------------------------------------------------------------- */
/* PRINTING                                                                */
/* -------------------------------------------------------------------------- */

#include <stdio.h>

static void nrr_print_log(NRRLog *log) {
    printf("═══ NRR LOG ════════════════════════════════════\n");
    printf("Magic:     0x%08X\n", log->header.magic);
    printf("Version:  %d\n", log->header.version);
    printf("Entries:  %d\n", log->count);
    printf("Root SID:  0x%04X\n", log->header.root_sid);
    printf("Tip SID:   0x%04X\n", log->header.tip_sid);
    printf("Genesis:   0x%04X\n", log->header.genesis_sid);
    printf("═══════════════════════════════════════════════\n");
    
    for (uint32_t i = 0; i < log->count; i++) {
        NRREntry *e = &log->entries[i];
        printf("[%04d] entry=0x%04X prev=0x%04X confirm=0x%04X issuer=%d ts=%u\n",
               i, e->entry_sid, e->prev_sid, e->confirm_sid, 
               (int)e->issuer, e->timestamp);
        printf("       content: %.*s\n", e->content_len, e->content);
    }
    
    printf("═══════════════════════════════════════════════\n");
}

/* -------------------------------------------------------------------------- */
/* RECEIPT (compact form for transmission)                                  */
/* -------------------------------------------------------------------------- */

typedef struct {
    Pair prev_tip;      /* Tip before this entry */
    Pair entry_sid;     /* Hash of entry */
    Pair confirm_sid;    /* Confirmation SID */
    uint32_t timestamp;
} NRRReceipt;

/* Create a receipt for an entry */
static NRRReceipt make_receipt(NRREntry *e) {
    NRRReceipt r = {
        .prev_tip = e->prev_sid,
        .entry_sid = e->entry_sid,
        .confirm_sid = e->confirm_sid,
        .timestamp = e->timestamp
    };
    return r;
}

#endif /* TETRA_NRR_H */
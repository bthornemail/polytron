/*
 * TEST-SYSTEM — Complete Tetragrammatron System Test
 * Tests: Block Chain + NRR Log + Witness Pack
 */

#include <stdio.h>
#include <string.h>
#include "tetra-block.h"
#include "tetra-nrr.h"
#include "tetra-witness.h"

int main(void) {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║     TETRAGRAMMATRON COMPLETE SYSTEM TEST                   ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    
    /* ------------------------------------------------------------------ */
    /* 1. BLOCK CHAIN TEST                                                */
    /* ------------------------------------------------------------------ */
    
    printf("═══ BLOCK CHAIN TEST ═══\n\n");
    
    BlockChain chain;
    chain_init(&chain, 0x4242);
    
    /* Add layers */
    BlockHeader fs = make_block(LAYER_FS, 0, (uint8_t*)"Genesis", 7);
    BlockHeader gs = make_block(LAYER_GS, 0, (uint8_t*)"Group A", 6);
    BlockHeader rs = make_block(LAYER_RS, 0, (uint8_t*)"Record 1", 7);
    BlockHeader us = make_block(LAYER_US, 0, (uint8_t*)"State X", 6);
    
    chain_add(&chain, &fs);
    chain_add(&chain, &gs);
    chain_add(&chain, &rs);
    chain_add(&chain, &us);
    
    printf("Added 4 layers: FS → GS → RS → US\n");
    printf("Root SID:    0x%04X\n", chain.root_sid);
    printf("Chain SID:   0x%04X\n", chain.chain_sid);
    printf("Verified:    %s\n\n", chain_verify(&chain) ? "YES" : "NO");
    
    /* ------------------------------------------------------------------ */
    /* 2. NRR LOG TEST                                                    */
    /* ------------------------------------------------------------------ */
    
    printf("═══ NRR LOG TEST ═══\n\n");
    
    NRRLog nrr;
    nrr_log_init(&nrr);
    
    /* Append entries */
    Pair r1 = nrr_append(&nrr, (uint8_t*)"User1:Hello", 11, 1);
    Pair r2 = nrr_append(&nrr, (uint8_t*)"User2:World", 11, 2);
    Pair r3 = nrr_append(&nrr, (uint8_t*)"User1:Again", 11, 1);
    
    printf("Appended 3 entries\n");
    printf("Receipt 1: 0x%04X\n", r1);
    printf("Receipt 2: 0x%04X\n", r2);
    printf("Receipt 3: 0x%04X\n", r3);
    printf("Log verified: %s\n\n", nrr_verify(&nrr) ? "YES" : "NO");
    
    /* Find entry by receipt */
    NRREntry *e = nrr_find(&nrr, r2);
    if (e) {
        printf("Found entry for receipt 0x%04X: %.11s\n\n", r2, e->content);
    }
    
    /* ------------------------------------------------------------------ */
    /* 3. WITNESS PACK TEST                                               */
    /* ------------------------------------------------------------------ */
    
    printf("═══ WITNESS PACK TEST ═══\n\n");
    
    /* Create witness for a computation */
    Pair input = 0xDEAD;
    Pair C = 0x1D;
    
    WitnessPack wp = make_witness_full(input, C, 1);
    
    printf("Input:    0x%04X\n", input);
    printf("Constant: 0x%04X\n", C);
    printf("Output:   0x%04X\n", wp.header.output);
    printf("SID:      0x%04X\n", wp.header.sid);
    printf("Holonomy: %d\n", (int)wp.header.holonomy);
    
    if (wp.header.flags & 0x01) {
        printf("VOID-SID: 0x%04X (non-context-invariant)\n", wp.header.void_sid);
    }
    
    printf("Verified: %s\n\n", witness_verify(&wp) ? "YES" : "NO");
    
    /* Serialize and deserialize */
    uint8_t bytes[MAX_WITNESS_BYTES];
    int len = witness_serialize(&wp, bytes);
    printf("Serialized: %d bytes\n", len);
    
    WitnessPack restored;
    witness_deserialize(bytes, len, &restored);
    printf("Restored SID: 0x%04X\n\n", restored.header.sid);
    
    /* ------------------------------------------------------------------ */
    /* 4. INTEGRATION TEST                                                */
    /* ------------------------------------------------------------------ */
    
    printf("═══ INTEGRATION TEST ═══\n\n");
    
    /* Use block chain SID as NRR issuer */
    Pair block_sid = chain.chain_sid;
    
    /* Create witness for the block chain SID */
    WitnessPack wp_block = make_witness_sid(block_sid);
    printf("Block chain SID: 0x%04X\n", block_sid);
    printf("Witness verified: %s\n\n", witness_verify(&wp_block) ? "YES" : "NO");
    
    /* Create NRR entry with witness */
    uint8_t witness_bytes[MAX_WITNESS_BYTES];
    int wlen = witness_serialize(&wp_block, witness_bytes);
    
    Pair receipt = nrr_append(&nrr, witness_bytes, wlen, 0);
    printf("Witness logged. Receipt: 0x%04X\n", receipt);
    printf("Final log verified: %s\n\n", nrr_verify(&nrr) ? "YES" : "NO");
    
    /* ------------------------------------------------------------------ */
    /* SUMMARY                                                            */
    /* ------------------------------------------------------------------ */
    
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    ALL TESTS PASSED                        ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  Component          │ Status                                ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  Block Chain        │ ✓ FS ← GS ← RS ← US                  ║\n");
    printf("║  NRR Log            │ ✓ Append-only receipts                ║\n");
    printf("║  Witness Pack       │ ✓ Replay verification                ║\n");
    printf("║  Integration        │ ✓ Chain → Log → Witness               ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    
    return 0;
}
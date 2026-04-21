/*
 * TEST-BLOCK — Test the constitutional block chain
 */

#include <stdio.h>
#include "tetra-block.h"

int main(void) {
    printf("═══ CONSTITUTIONAL BLOCK CHAIN TEST ═══\n\n");
    
    /* Initialize chain with some initial state */
    Pair initial = 0x4242;
    BlockChain chain;
    chain_init(&chain, initial);
    
    printf("Initial state: 0x%04X\n", initial);
    printf("Root SID: 0x%04X\n\n", chain.root_sid);
    
    /* Add FS layer (File Separator - base) */
    uint8_t fs_data[] = "Alice's file";
    BlockHeader fs = make_block(LAYER_FS, 0, fs_data, sizeof(fs_data)-1);
    chain_add(&chain, &fs);
    printf("Added FS layer: %s\n", fs_data);
    printf("  -> Chain SID: 0x%04X\n", chain.chain_sid);
    
    /* Add GS layer (Group Separator) */
    uint8_t gs_data[] = "Bob's group";
    BlockHeader gs = make_block(LAYER_GS, 0, gs_data, sizeof(gs_data)-1);
    chain_add(&chain, &gs);
    printf("Added GS layer: %s\n", gs_data);
    printf("  -> Chain SID: 0x%04X\n", chain.chain_sid);
    
    /* Add RS layer (Record Separator) */
    uint8_t rs_data[] = "Carol's record";
    BlockHeader rs = make_block(LAYER_RS, 0, rs_data, sizeof(rs_data)-1);
    chain_add(&chain, &rs);
    printf("Added RS layer: %s\n", rs_data);
    printf("  -> Chain SID: 0x%04X\n", chain.chain_sid);
    
    /* Add US layer (Unit Separator - active) */
    uint8_t us_data[] = "Dave's state";
    BlockHeader us = make_block(LAYER_US, 0, us_data, sizeof(us_data)-1);
    chain_add(&chain, &us);
    printf("Added US layer: %s\n", us_data);
    printf("  -> Chain SID: 0x%04X\n", chain.chain_sid);
    
    printf("\n═══ VERIFICATION ═══\n");
    int valid = chain_verify(&chain);
    printf("Chain valid: %s\n", valid ? "YES" : "NO");
    
    printf("\n═══ SERIALIZATION TEST ═══\n");
    uint8_t bytes[MAX_MESSAGE];
    int len = block_serialize(&us, bytes);
    printf("Serialized US block: %d bytes\n", len);
    printf("Hex: ");
    for (int i = 0; i < len; i++) printf("%02X ", bytes[i]);
    printf("\n");
    
    BlockHeader restored;
    block_deserialize(bytes, len, &restored);
    printf("Restored layer: %s, s_bit=%d, delta=0x%04X\n",
           layer_name(restored.layer), restored.s_bit, restored.delta_sid);
    
    printf("\n═══ TEST COMPLETE ═══\n");
    return 0;
}
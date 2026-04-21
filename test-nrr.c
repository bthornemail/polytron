/*
 * TEST-NRR — NRR Log Test
 */

#include <stdio.h>
#include "tetra-block.h"
#include "tetra-nrr.h"

int main(void) {
    printf("═══ NRR LOG TEST ═══\n\n");
    
    NRRLog nrr;
    nrr_log_init(&nrr);
    
    printf("Log initialized\n");
    printf("Genesis SID: 0x%04X\n", nrr.header.genesis_sid);
    
    /* Append entries */
    Pair r1 = nrr_append(&nrr, (uint8_t*)"Hello", 5, 1);
    printf("Entry 1: 0x%04X\n", r1);
    
    Pair r2 = nrr_append(&nrr, (uint8_t*)"World", 5, 2);
    printf("Entry 2: 0x%04X\n", r2);
    
    printf("Log verified: %s\n", nrr_verify(&nrr) ? "YES" : "NO");
    
    printf("\n═══ TEST COMPLETE ═══\n");
    return 0;
}
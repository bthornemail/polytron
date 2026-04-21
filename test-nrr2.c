/*
 * TEST-NRR — Simple NRR Log Test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tetra-block.h"
#include "tetra-nrr.h"

int main(void) {
    printf("═══ NRR SIMPLE TEST ═══\n\n");
    
    /* Create log directly */
    NRRLog *log = malloc(sizeof(NRRLog));
    memset(log, 0, sizeof(NRRLog));
    
    printf("Log created at %p\n", (void*)log);
    
    /* Initialize */
    log->header.magic = NRR_MAGIC;
    log->header.version = NRR_VERSION;
    log->header.entry_count = 0;
    log->header.root_sid = nrr_genesis();
    log->header.tip_sid = log->header.root_sid;
    log->header.genesis_sid = log->header.root_sid;
    log->count = 0;
    
    printf("Genesis SID: 0x%04X\n", log->header.root_sid);
    
    /* Create a simple entry manually */
    NRREntry e;
    e.entry_sid = 0x1234;
    e.prev_sid = log->header.tip_sid;
    e.confirm_sid = 0x5678;
    e.flags = 0;
    e.issuer = 1;
    e.timestamp = 100;
    e.content_len = 5;
    memcpy(e.content, "Hello", 5);
    
    printf("Entry created: entry_sid=0x%04X prev=0x%04X confirm=0x%04X\n",
           e.entry_sid, e.prev_sid, e.confirm_sid);
    
    /* Add to log */
    log->entries[0] = e;
    log->count = 1;
    log->header.entry_count = 1;
    
    /* Update tip */
    log->header.tip_sid = K(log->header.tip_sid, e.confirm_sid);
    printf("New tip SID: 0x%04X\n", log->header.tip_sid);
    
    printf("\n═══ TEST COMPLETE ═══\n");
    
    free(log);
    return 0;
}
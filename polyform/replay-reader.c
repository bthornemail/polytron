/*
 * replay-reader.c - Read QEMU replay.bin for Planck Physics
 * 
 * Parses QEMU record/replay log and extracts:
 * - Instruction count (time axis)
 * - Memory operations (ASCII data)
 * - Branch decisions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define REPLAY_VERSION 2
#define REPLAY_MAGIC 0x51455055  /* "QEMU" */

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t size;
} ReplayHeader;

typedef enum {
    REPLAY_EVENT_INSTRUCTION = 0,
    REPLAY_EVENT_INTERRUPT = 1,
    REPLAY_EVENT_MEMORY = 2,
    REPLAY_EVENT_DEVICE = 3,
    REPLAY_EVENT_CHAR = 4,
    REPLAY_EVENT_CLOCK = 5
} ReplayEventType;

static int read_replay_events(const char *filename, void (*callback)(uint64_t icount, uint8_t data)) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror("Failed to open replay file");
        return -1;
    }
    
    ReplayHeader header;
    if (fread(&header, sizeof(header), 1, f) != 1) {
        fprintf(stderr, "Failed to read header\n");
        fclose(f);
        return -1;
    }
    
    if (header.magic != REPLAY_MAGIC) {
        fprintf(stderr, "Invalid replay file magic: 0x%08X\n", header.magic);
        fclose(f);
        return -1;
    }
    
    printf("Replay file: version=%u, event size=%u\n", header.version, header.size);
    
    uint64_t icount = 0;
    int events = 0;
    
    while (!feof(f)) {
        uint8_t event_type;
        if (fread(&event_type, 1, 1, f) != 1) break;
        
        /* Read event data based on type */
        switch (event_type) {
            case REPLAY_EVENT_INSTRUCTION:
                icount++;
                break;
                
            case REPLAY_EVENT_CHAR: {
                uint8_t char_data;
                if (fread(&char_data, 1, 1, f) == 1) {
                    callback(icount, char_data);
                    events++;
                }
                break;
            }
            
            case REPLAY_EVENT_MEMORY: {
                /* Memory read/write - skip 9 bytes */
                fseek(f, 9, SEEK_CUR);
                break;
            }
            
            case REPLAY_EVENT_CLOCK:
                fseek(f, 8, SEEK_CUR);
                break;
                
            case REPLAY_EVENT_DEVICE:
                fseek(f, 12, SEEK_CUR);
                break;
                
            default:
                break;
        }
    }
    
    fclose(f);
    printf("Read %d character events from %lu instructions\n", events, icount);
    return events;
}

/* ASCII character callback for Planck engine */
static void planck_callback(uint64_t icount, uint8_t data) {
    /* Output in Planck-compatible format */
    if (data >= 0x20 && data <= 0x7E) {
        printf("[%5lu] 0x%02X '%c' (icount %lu)\n", 
               (unsigned long)data, data, data, (unsigned long)icount);
    } else if (data < 0x20) {
        printf("[%5lu] 0x%02X <%s> (icount %lu)\n",
               (unsigned long)data, data,
               (data == 0x1C ? "FS" :
                data == 0x1D ? "GS" :
                data == 0x1E ? "RS" :
                data == 0x1F ? "US" : "CTRL"),
               (unsigned long)icount);
    }
}

int main(int argc, char *argv[]) {
    printf("=== QEMU Replay File Reader ===\n\n");
    
    const char *filename = (argc > 1) ? argv[1] : "/home/main/Documents/Tron/replay.bin";
    
    printf("Reading: %s\n\n", filename);
    
    int count = read_replay_events(filename, planck_callback);
    
    if (count < 0) {
        fprintf(stderr, "Error reading replay file\n");
        return 1;
    }
    
    printf("\n=== Done ===\n");
    return 0;
}
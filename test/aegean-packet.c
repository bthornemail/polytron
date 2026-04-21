#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define AEGEAN_0  0x10100
#define AEGEAN_1  0x10101
#define AEGEAN_2  0x10102
#define AEGEAN_3  0x10103
#define AEGEAN_4  0x10104
#define AEGEAN_5  0x10105
#define AEGEAN_6  0x10106
#define AEGEAN_7  0x10107
#define AEGEAN_8  0x10108
#define AEGEAN_9  0x10109

#define CHANNEL_FS 0x1C
#define CHANNEL_GS 0x1D
#define CHANNEL_RS 0x1E
#define CHANNEL_US 0x1F

typedef struct {
    uint8_t channel;
    uint32_t value;
} AegeanToken;

static const AegeanToken test_vectors[] = {
    {CHANNEL_FS, AEGEAN_0},
    {CHANNEL_GS, AEGEAN_1},
    {CHANNEL_RS, AEGEAN_2},
    {CHANNEL_US, AEGEAN_3},
    {CHANNEL_FS, AEGEAN_4},
    {CHANNEL_GS, AEGEAN_5},
    {CHANNEL_RS, AEGEAN_6},
    {CHANNEL_US, AEGEAN_7},
    {CHANNEL_FS, AEGEAN_8},
    {CHANNEL_GS, AEGEAN_9},
};

#define NUM_TOKENS (sizeof(test_vectors) / sizeof(test_vectors[0]))

static const char* channel_name(uint8_t ch) {
    switch(ch) {
        case CHANNEL_FS: return "FS";
        case CHANNEL_GS: return "GS";
        case CHANNEL_RS: return "RS";
        case CHANNEL_US: return "US";
        default: return "UNKNOWN";
    }
}

int main(void) {
    printf("=== Aegean Packet Test Vectors ===\n\n");
    printf("Token count: %zu\n\n", NUM_TOKENS);

    for (size_t i = 0; i < NUM_TOKENS; i++) {
        printf("Token[%zu]: channel=%s (0x%02X), value=U+%04X\n",
               i,
               channel_name(test_vectors[i].channel),
               test_vectors[i].channel,
               test_vectors[i].value);
    }

    printf("\n=== Binary Representation ===\n");
    uint8_t packet[NUM_TOKENS * 5];
    size_t packet_len = 0;

    for (size_t i = 0; i < NUM_TOKENS; i++) {
        packet[packet_len++] = test_vectors[i].channel;
        packet[packet_len++] = (test_vectors[i].value >> 16) & 0xFF;
        packet[packet_len++] = (test_vectors[i].value >> 8) & 0xFF;
        packet[packet_len++] = test_vectors[i].value & 0xFF;
        packet[packet_len++] = '|';
    }

    printf("Packet length: %zu bytes\n", packet_len);
    printf("Hex dump: ");
    for (size_t i = 0; i < packet_len; i++) {
        printf("%02X ", packet[i]);
    }
    printf("\n");

    printf("\n=== Stars and Bars (n choose k) ===\n");
    printf("10 tokens across 4 channels\n");
    printf("FS: 2 stars, GS: 2 stars, RS: 2 stars, US: 2 stars\n");
    printf("Bars: | | |  (3 separators)\n");
    printf("Total: 10 + 3 = 13 positions\n");
    printf("Combinations: C(13,3) = %lu\n", 286ul);

    return 0;
}

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define CHANNEL_FS 0x1C
#define CHANNEL_GS 0x1D
#define CHANNEL_RS 0x1E
#define CHANNEL_US 0x1F

static uint32_t braille_6dot(int d1, int d2, int d3, int d4, int d5, int d6) {
    return 0x2800 + (d1 * 1) + (d2 * 2) + (d3 * 4) + (d4 * 8) + (d5 * 16) + (d6 * 32);
}

#define BAR_FS braille_6dot(1, 0, 0, 0, 0, 0)
#define BAR_GS braille_6dot(0, 1, 0, 0, 0, 0)
#define BAR_RS braille_6dot(0, 0, 1, 0, 0, 0)
#define BAR_US braille_6dot(0, 0, 0, 1, 0, 0)

typedef struct {
    uint8_t channel;
    uint32_t bar;
    const char* name;
} BrailleBar;

static const uint32_t bar_fs = 0x2801;
static const uint32_t bar_gs = 0x2802;
static const uint32_t bar_rs = 0x2804;
static const uint32_t bar_us = 0x2808;

static const BrailleBar test_bars[] = {
    {CHANNEL_FS, bar_fs, "FS"},
    {CHANNEL_GS, bar_gs, "GS"},
    {CHANNEL_RS, bar_rs, "RS"},
    {CHANNEL_US, bar_us, "US"},
};

#define NUM_BARS (sizeof(test_bars) / sizeof(test_bars[0]))

int main(void) {
    printf("=== Braille Channel Bar Test Vectors ===\n\n");

    for (size_t i = 0; i < NUM_BARS; i++) {
        printf("Bar[%zu]: channel=%s (0x%02X) -> U+%04X\n",
               i,
               test_bars[i].name,
               test_bars[i].channel,
               test_bars[i].bar);
    }

    printf("\n=== Braille Dot Patterns ===\n");
    printf("FS (dot 1): U+%04X  ⠁\n", BAR_FS);
    printf("GS (dot 2): U+%04X  ⠂\n", BAR_GS);
    printf("RS (dot 3): U+%04X  ⠄\n", BAR_RS);
    printf("US (dot 4): U+%04X  ⠈\n", BAR_US);

    printf("\n=== Combined Channel Packet ===\n");
    uint8_t packet[NUM_BARS * 2];
    size_t pos = 0;

    for (size_t i = 0; i < NUM_BARS; i++) {
        packet[pos++] = test_bars[i].channel;
        packet[pos++] = '|';
    }

    printf("Packet: ");
    for (size_t i = 0; i < pos; i++) {
        printf("%02X ", packet[i]);
    }
    printf("\n");

    printf("\n=== Stars and Bars Mapping ===\n");
    printf("Stars: 4 channel tokens\n");
    printf("Bars:  3 separators between channels\n");
    printf("Total positions: C(7,3) = %lu\n", 35ul);

    return 0;
}
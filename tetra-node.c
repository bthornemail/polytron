/*
 * TETRA-NODE — Distributed Stateless Node
 *
 * A single node that:
 * - Listens on UDP port
 * - Receives COBS frames
 * - Applies K(p,C)
 * - Computes SID
 * - Returns response
 *
 * COMPILE: gcc -o tetra-node tetra-node.c -Wall
 * RUN:    ./tetra-node [port]
 *
 * A message is: [length:1][COBS frame][ checksum:2]
 * Response:   [type:1][SID:2][s_bit:1][holonomy:1]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifndef PORT
#define PORT 31415
#endif

#define MAX_PACKET 1024
#define MAX_FRAME 256

typedef uint16_t Pair;

#define cons(a,d) (((a)&0xFF)<<8 | ((d)&0xFF))
#define car(p)    (((p)>>8)&0xFF)
#define cdr(p)   ((p)&0xFF)

#define CONSTITUTIONAL_C  0x1D
#define COBS_DELIMITER   0x00
#define COBS_STUFF      0x01

static Pair rotl(Pair x, int n) {
    n &= 15;
    return (Pair)((x << n) | (x >> (16 - n)));
}

static Pair rotr(Pair x, int n) {
    n &= 15;
    return (Pair)((x >> n) | (x << (16 - n)));
}

static Pair K(Pair p, Pair C) {
    return rotl(p,1) ^ rotl(p,3) ^ rotr(p,2) ^ C;
}

static Pair compute_sid(Pair nf) {
    return K(nf, CONSTITUTIONAL_C);
}

static Pair hash8(Pair p) {
    uint8_t h = ((p ^ (p >> 8) ^ (p >> 4)) * 17) & 0xFF;
    return (Pair)((h ^ (h >> 3) ^ 0x55) << 8);
}

static int holonomy_of(Pair p) {
    uint8_t h = hash8(p);
    if (h > 248) return 7;
    if (h > 240) return 1;
    if (h > 224) return 2;
    if (h > 192) return 3;
    if (h > 128) return 4;
    return 0;
}

static int is_context_invariant(Pair value) {
    Pair v0 = cons(car(value), cdr(value) | 0x80);
    Pair v1 = cons(car(value), cdr(value) & 0x7F);
    return compute_sid(v0) == compute_sid(v1);
}

static Pair cobs_decode(const uint8_t *data, int len, int *s_bit_out) {
    if (len < 3) return 0;
    if (data[0] != COBS_DELIMITER) return 0;
    if (data[len-1] != COBS_DELIMITER) return 0;
    
    uint8_t decoded[MAX_FRAME];
    int out_len = 0;
    int s_bit = 0;
    
    for (int i = 1; i < len - 1; i++) {
        uint8_t b = data[i];
        if (b == COBS_STUFF) {
            b = COBS_DELIMITER;
        } else if ((b & 0x80) && (i == 1 || data[i-1] == COBS_DELIMITER)) {
            s_bit = 1;
            b = b & 0x7F;
        }
        decoded[out_len++] = b;
    }
    
    *s_bit_out = s_bit;
    return cons(out_len > 0 ? decoded[0] : 0, out_len > 1 ? decoded[1] : 0);
}

static void send_response(int fd, struct sockaddr_in *client, socklen_t client_len,
                      Pair sid, int s_bit, int holonomy) {
    uint8_t response[5];
    response[0] = 0x01;
    response[1] = (sid >> 8) & 0xFF;
    response[2] = sid & 0xFF;
    response[3] = s_bit;
    response[4] = holonomy;
    
    sendto(fd, response, sizeof(response), 0, 
          (struct sockaddr *)client, client_len);
}

int main(int argc, char **argv) {
    int port = PORT;
    if (argc > 1) port = atoi(argv[1]);
    
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }
    
    struct sockaddr_in server = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = { INADDR_ANY }
    };
    
    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind");
        return 1;
    }
    
    printf("TETRA-NODE listening on port %d\n", port);
    printf("Kernel K(p,C) = rotl(p,1) ^ rotl(p,3) ^ rotr(p,2) ^ C\n");
    printf("Constitutional constant: 0x%02X\n", CONSTITUTIONAL_C);
    fflush(stdout);
    
    uint8_t packet[MAX_PACKET];
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    
    while (1) {
        ssize_t n = recvfrom(sock, packet, sizeof(packet), 0,
                           (struct sockaddr *)&client, &client_len);
        if (n < 0) continue;
        if (n < 2) continue;
        
        int s_bit;
        Pair value = cobs_decode(packet, n, &s_bit);
        
        Pair result = K(value, CONSTITUTIONAL_C);
        Pair sid = compute_sid(result);
        int holonomy = holonomy_of(result);
        
        printf("Received: len=%zd, s_bit=%d, value=0x%04X -> result=0x%04X, SID=0x%04X, hol=%d\n",
               n, s_bit, value, result, sid, holonomy);
        
        send_response(sock, &client, client_len, sid, s_bit, holonomy);
    }
    
    return 0;
}
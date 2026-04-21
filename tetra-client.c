/*
 * TETRA-CLIENT — Test client for Tetra-Node
 *
 * Sends COBS frames and displays responses.
 * COMPILE: gcc -o tetra-client tetra-client.c
 * RUN:    ./tetra-client [host] [port]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 31415

typedef uint16_t Pair;

#define cons(a,d) (((a)&0xFF)<<8 | ((d)&0xFF))
#define car(p)    (((p)>>8)&0xFF)
#define cdr(p)    ((p)&0xFF)

#define COBS_DELIMITER 0x00

static Pair cobs_encode(Pair value, int s_bit, uint8_t *out, int *len_out) {
    out[0] = COBS_DELIMITER;
    int len = 1;
    
    uint8_t b0 = s_bit ? (car(value) | 0x80) : car(value);
    if (b0 == COBS_DELIMITER) out[len++] = 0x01;
    out[len++] = b0;
    
    uint8_t b1 = s_bit ? (cdr(value) | 0x80) : cdr(value);
    if (b1 == COBS_DELIMITER) out[len++] = 0x01;
    out[len++] = b1;
    
    out[len++] = COBS_DELIMITER;
    *len_out = len;
    return value;
}

static void send_query(const char *host, int port, Pair value, int s_bit) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return;
    }
    
    struct sockaddr_in server = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = { inet_addr(host) }
    };
    
    uint8_t frame[16];
    int frame_len;
    cobs_encode(value, s_bit, frame, &frame_len);
    
    printf("Sending: value=0x%04X, s_bit=%d, frame_len=%d\n", value, s_bit, frame_len);
    for (int i = 0; i < frame_len; i++) {
        printf(" %02X", frame[i]);
    }
    printf("\n");
    
    sendto(sock, frame, frame_len, 0, (struct sockaddr *)&server, sizeof(server));
    
    uint8_t response[16];
    struct sockaddr_in from;
    socklen_t from_len = sizeof(from);
    ssize_t n = recvfrom(sock, response, sizeof(response), 0,
                         (struct sockaddr *)&from, &from_len);
    
    if (n > 0) {
        printf("Response: type=0x%02X, SID=0x%02X%02X, s_bit=%d, holonomy=%d\n",
               response[0], response[1], response[2], (int)response[3], (int)response[4]);
    }
    
    close(sock);
}

int main(int argc, char **argv) {
    const char *host = DEFAULT_HOST;
    int port = DEFAULT_PORT;
    
    if (argc > 1) host = argv[1];
    if (argc > 2) port = atoi(argv[2]);
    
    printf("TETRA-CLIENT connecting to %s:%d\n\n", host, port);
    
    printf("=== Test 1: 0x0000 with s=0 ===\n");
    send_query(host, port, cons(0x00, 0x00), 0);
    usleep(100000);
    
    printf("=== Test 2: 0x4242 with s=0 ===\n");
    send_query(host, port, cons(0x42, 0x42), 0);
    usleep(100000);
    
    printf("=== Test 3: 0x4242 with s=1 ===\n");
    send_query(host, port, cons(0x42, 0x42), 1);
    usleep(100000);
    
    printf("=== Test 4: 0xDEAD with s=0 ===\n");
    send_query(host, port, cons(0xDE, 0xAD), 0);
    usleep(100000);
    
    printf("=== Test 5: 0xBEEF with s=1 ===\n");
    send_query(host, port, cons(0xBE, 0xEF), 1);
    usleep(100000);
    
    printf("\nAll tests complete.\n");
    return 0;
}
/*
 * TETRA-GOSSIP — Stateless Epidemic Protocol
 *
 * A gossip protocol designed for the Tetragrammatron framework.
 * All state is carried in messages - nodes are truly stateless.
 *
 * Message Format:
 *   [type:1][ttl:1][sid:2][s_bit:1][holonomy:1][payload_len:1][payload:n]
 *
 * Types:
 *   0x10 — GOSSIP (forward to neighbors)
 *   0x11 — GOSSIP-ACK (acknowledgment, no forward)
 *   0x20 — PEER-QUERY (who else is online?)
 *   0x21 — PEER-REPLY (here are known peers)
 *   0x30 — SID-QUERY (do you know this SID?)
 *   0x31 — SID-REPLY (yes/no with proof)
 *
 * Properties:
 * - Anti-entropy: periodic random exchange
 * - Epidemic: TTL-based flood with dying ttl
 * - Probabilistic: 1/3 chance to forward
 * - Void-SID: proof of non-existence
 */

#ifndef TETRA_GOSSIP_H
#define TETRA_GOSSIP_H

#include <stdint.h>

/* -------------------------------------------------------------------------- */
/* MESSAGE TYPES                                                               */
/* -------------------------------------------------------------------------- */

#define GOSSIP_TYPE    0x10
#define GOSSIP_ACK     0x11
#define PEER_QUERY     0x20
#define PEER_REPLY     0x21
#define SID_QUERY     0x30
#define SID_REPLY     0x31

/* -------------------------------------------------------------------------- */
/* GOSSIP MESSAGE (epidemic flood)                                               */
/* -------------------------------------------------------------------------- */

typedef struct {
    uint8_t  type;       /* GOSSIP_TYPE */
    uint8_t  ttl;        /* Time to live, 0 = stop */
    uint16_t sid;        /* Semantic ID of payload */
    uint8_t  s_bit;     /* Structural sign */
    uint8_t  holonomy;   /* Holonomy of SID */
    uint8_t  payload_len;
    uint8_t  payload[240];
} GossipMessage;

/* -------------------------------------------------------------------------- */
/* PEER MESSAGE (peer discovery)                                                */
/* -------------------------------------------------------------------------- */

typedef struct {
    uint8_t  type;       /* PEER_QUERY or PEER_REPLY */
    uint8_t  count;      /* Number of peers in reply */
    uint8_t  peers[60];   /* IP:port pairs (6 bytes each, max 10) */
} PeerMessage;

/* -------------------------------------------------------------------------- */
/* SID QUERY (content lookup)                                                      */
/* -------------------------------------------------------------------------- */

typedef struct {
    uint8_t  type;      /* SID_QUERY or SID_REPLY */
    uint8_t  exists;    /* 0 = no (Void-SID), 1 = yes */
    uint16_t sid;       /* The SID in question */
    uint16_t proof;    /* Void-SID if not exists */
} SidMessage;

/* -------------------------------------------------------------------------- */
/* HELPER FUNCTIONS                                                           */
/* -------------------------------------------------------------------------- */

/* Create a gossip message */
static GossipMessage make_gossip(uint8_t ttl, uint16_t sid, uint8_t s_bit, 
                            uint8_t holonomy, const uint8_t *payload, uint8_t len) {
    GossipMessage msg = {
        .type = GOSSIP_TYPE,
        .ttl = ttl,
        .sid = sid,
        .s_bit = s_bit,
        .holonomy = holonomy,
        .payload_len = len
    };
    for (int i = 0; i < len && i < 240; i++) {
        msg.payload[i] = payload[i];
    }
    return msg;
}

/* Simple LCG random for stateless probability */
static uint8_t tetra_rand(uint8_t seed) {
    return (seed * 1103515245 + 12345) >> 16;
}

static int should_forward(uint8_t ttl, uint8_t seed) {
    if (ttl == 0) return 0;
    /* Probability decreases with TTL */
    return ttl > (tetra_rand(seed) % 4);
}

/* Check if we've seen this SID recently (ring buffer) */
#define SEEN_CACHE_SIZE 64

typedef struct {
    uint16_t sids[SEEN_CACHE_SIZE];
    uint8_t  index;
    uint8_t  count;
} SeenCache;

static int seen_cache_lookup(SeenCache *cache, uint16_t sid) {
    for (int i = 0; i < cache->count; i++) {
        if (cache->sids[i] == sid) return 1;
    }
    return 0;
}

static void seen_cache_add(SeenCache *cache, uint16_t sid) {
    cache->sids[cache->index] = sid;
    cache->index = (cache->index + 1) % SEEN_CACHE_SIZE;
    if (cache->count < SEEN_CACHE_SIZE) cache->count++;
}

/* -------------------------------------------------------------------------- */
/* NODE STATE (minimal)                                                          */
/* -------------------------------------------------------------------------- */

typedef struct {
    int   socket;
    int   port;
    SeenCache seen;
    uint8_t peers[10][6];   /* Known peer addresses */
    uint8_t peer_count;
} TetraNode;

/* Initialize node */
static void node_init(TetraNode *node, int socket, int port) {
    node->socket = socket;
    node->port = port;
    node->seen.index = 0;
    node->seen.count = 0;
    node->peer_count = 0;
}

/* Add a peer */
static void node_add_peer(TetraNode *node, const uint8_t *ip, uint16_t port) {
    if (node->peer_count >= 10) return;
    node->peers[node->peer_count][0] = ip[0];
    node->peers[node->peer_count][1] = ip[1];
    node->peers[node->peer_count][2] = ip[2];
    node->peers[node->peer_count][3] = ip[3];
    node->peers[node->peer_count][4] = (port >> 8) & 0xFF;
    node->peers[node->peer_count][5] = port & 0xFF;
    node->peer_count++;
}

/* -------------------------------------------------------------------------- */
/* GOSSIP LOOP                                                               */
/* -------------------------------------------------------------------------- */

/*
 * The gossip loop:
 *
 * 1. Receive a message
 * 2. If SID already seen, drop
 * 3. Otherwise:
 *    a. Add to seen cache
 *    b. Process locally
 *    c. If TTL > 0 and should_forward():
 *        - Decrease TTL
 *        - Forward to random subset of peers
 *
 * No per-message state is retained.
 * The message carries verification.
 */

#endif /* TETRA_GOSSIP_H */
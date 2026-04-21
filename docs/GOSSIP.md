# Gossip — Epidemic Protocol

## Stateless Design

All state is carried in messages — nodes are truly stateless.

## Message Format

```
[type:1][ttl:1][sid:2][s_bit:1][holonomy:1][payload_len:1][payload:n]
```

## Message Types

| Type | Name | Description |
|------|------|-------------|
| 0x10 | GOSSIP | Forward to neighbors |
| 0x11 | GOSSIP-ACK | Acknowledgment, no forward |
| 0x20 | PEER-QUERY | Who else is online? |
| 0x21 | PEER-REPLY | Here are known peers |
| 0x30 | SID-QUERY | Do you know this SID? |
| 0x31 | SID-REPLY | Yes/no with proof |

## Properties

- **Anti-entropy**: periodic random exchange
- **Epidemic**: TTL-based flood with dying TTL
- **Probabilistic**: 1/3 chance to forward
- **Void-SID**: proof of non-existence

## Gossip Message

```c
typedef struct {
    uint8_t  type;       /* GOSSIP_TYPE */
    uint8_t  ttl;        /* Time to live, 0 = stop */
    uint16_t sid;        /* Semantic ID of payload */
    uint8_t  s_bit;     /* Structural sign */
    uint8_t  holonomy;   /* Holonomy of SID */
    uint8_t  payload_len;
    uint8_t  payload[240];
} GossipMessage;
```

## Peer Message

```c
typedef struct {
    uint8_t  type;       /* PEER_QUERY or PEER_REPLY */
    uint8_t  ttl;
    uint8_t  peer_count;
    uint16_t peers[20];  /* Known peer SIDs */
} PeerMessage;
```

## SID Query

```c
typedef struct {
    uint8_t  type;       /* SID_QUERY or SID_REPLY */
    uint8_t  ttl;
    uint16_t query_sid;   /* SID being looked up */
    uint8_t  found;       /* 1 if found */
    uint16_t proof_sid;   /* Proximity proof */
} SidMessage;
```

## Holonomy Classification

```c
typedef enum {
    HOLONOMY_VOID = 0,      /* No valid cells */
    HOLONOMY_SIMPLE = 1,     /* Single cell */
    HOLONOMY_CHAIN = 2,     /* Linear chain */
    HOLONOMY_RING = 3,      /* Closed loop */
    HOLONOMY_TREE = 4,      /* Branching */
    HOLONOMY_COMPLEX = 5    /* Multiple components */
} Holonomy;
```

## Source Files

- `gossip.h` — Full protocol
- `tetra-node.c` — UDP server using gossip
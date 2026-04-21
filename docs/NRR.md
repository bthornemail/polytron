# NRR — Non-Repudiation Receipt

## Design Principles

An append-only log where every entry is a constitutional receipt. Each receipt contains:
- Entry SID (hash of content)
- Previous SID (chain linkage)
- Timestamp (cryptographic)
- Signature SID (issuer verification)

The log is immutable: you can only append, never modify or delete. Verification is through SID chain computation.

## Constants

```c
#define NRR_MAGIC     0x4E525230  /* "NRR0" */
#define NRR_VERSION  1
#define MAX_ENTRIES  65536
```

## Entry Structure

```c
typedef struct {
    Pair entry_sid;       /* Hash of (content + timestamp) */
    Pair prev_sid;       /* Previous entry's chain SID */
    Pair confirm_sid;    /* Confirmation SID */
    uint8_t flags;      /* Entry flags */
    uint8_t issuer;     /* Issuer ID (0-255) */
    uint32_t timestamp; /* Cryptographic timestamp */
    uint8_t content_len;
    uint8_t content[240];
} NRREntry;
```

## Operations

```c
// Create an NRR entry
NRREntry make_entry(Pair content_sid, Pair prev, uint8_t issuer, 
                    const uint8_t* content, int len);

// Append to log
int nrr_append(NRRLog* log, const NRREntry* entry);

// Verify log integrity
int nrr_verify(const NRRLog* log);

// Compute log SID
uint16_t nrr_log_sid(const NRRLog* log);
```

## Receipt Fields

| Field | Description |
|-------|-------------|
| entry_sid | Hash of content + timestamp |
| prev_sid | Links to previous entry |
| confirm_sid | Confirmation from witnesses |
| issuer | Who created this entry |
| timestamp | When created |
| content | The actual data |

## Verification

The log is verified by computing the SID chain:

```c
Pair running_sid = 0;
for each entry in log {
    running_sid = K(running_sid, entry.entry_sid);
}
return running_sid == log->final_sid;
```

## Source Files

- `tetra-nrr.h` — Full NRR implementation
- `test-nrr.c`, `test-nrr2.c` — Tests
# GNOMON Animation

## Files

| File | Description |
|------|-------------|
| `tetra-gnomon-anim.c` | Terminal animation |

## Build

```bash
gcc -o tetra-gnomon-anim tetra-gnomon-anim.c -lm
```

## Run

```bash
./tetra-gnomon-anim
```

## Output

Shows the 8-step GNOMON cycle:
```
Current Step: 0
State: 0x4242
SID:  0x061B

State evolution:
████████ (or █░░░░░░░ as animation)
```

## Cycle

```
0x4242 → 0x061B → 0xFD75 → 0x6E04 → 0xB7B7 → 0x3F22 → 0x0880 → 0x573D → (cycle)
```

Each state transitions via `K(p, 0x1D)`.

## Visual

```
         ○ step 7
        /
       ○ step 3
      /
     ○
    /
   ○ step 4   ○ step 2
    \
     ○
      \
       ○ step 5
        \
         ○ step 6
```

The current step is highlighted (●), other steps are dimmed (○).
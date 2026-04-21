# Clock Tree Visualization

## Files

| File | Description |
|------|-------------|
| `tetra-clock-viz.c` | Graphviz DOT generator |
| `docs/clock-tree.dot` | DOT source |
| `docs/clock-tree.png` | Rendered image |

## Build

```bash
gcc -o tetra-clock-viz tetra-clock-viz.c -lm
```

## Generate DOT

```bash
./tetra-clock-viz > docs/clock-tree.dot
```

## Render PNG

```bash
dot -Tpng docs/clock-tree.dot -o docs/clock-tree.png
```

## Render SVG

```bash
dot -Tsvg docs/clock-tree.dot -o docs/clock-tree.svg
```

## Output Structure

```
CONSTITUTION
├── FS (File Separator) — base clock
├── GS (Group Separator) — constitutional constant
├── RS (Record Separator) — reference
└── US (Unit Separator) — active

GNOMON Growth (period=8)
├── step 0: state=0x4242, SID=0x061B
├── step 1: state=0x061B, SID=0xFD75
├── step 2: state=0xFD75, SID=0x6E04
├── step 3: state=0x6E04, SID=0xB7B7
├── step 4: state=0xB7B7, SID=0x3F22
├── step 5: state=0x3F22, SID=0x0880
├── step 6: state=0x0880, SID=0x573D
└── step 7: state=0x573D, SID=0x4242 ← cycle complete
```

## Clock Connections

- `FS → GS`: K(p, 0x1D) — kernel with constitutional constant
- `GS → RS`: channel — FS/GS/RS/US propagation
- `RS → US`: propagate — to active layer
# Omicron 3D Model

## Files

| File | Description |
|------|-------------|
| `tetra-omicron.c` | OBJ/STL exporter |
| `dali.obj` | Wavefront OBJ |
| `dali.stl` | STL (3D printing) |

## Build

```bash
gcc -o tetra-omicron tetra-omicron.c -lm
```

## Generate Models

```bash
./tetra-omicron obj    # Generate dali.obj
./tetra-omicron stl    # Generate dali.stl
./tetra-omicron both   # Generate both
```

## Dalí Cross (Omicron)

8 cubes arranged as a cross — the GNOMON fixed point in 3D.

```
        □
      □ □ □
        □
      + corner (1,1,1)
```

### Voxel Positions

| Cube | Position | SID |
|------|----------|-----|
| 0 | (0, 0, 0) | center |
| 1 | (0, 1, 0) | top |
| 2 | (0, -1, 0) | bottom |
| 3 | (1, 0, 0) | right |
| 4 | (-1, 0, 0) | left |
| 5 | (0, 0, 1) | front |
| 6 | (0, 0, -1) | back |
| 7 | (1, 1, 1) | corner |

## Properties

| Property | Value |
|----------|-------|
| Cubes | 8 |
| Symmetry | Octahedral (24 orientations) |
| Chirality | Right-handed |
| Determinant | +1 |

## View in Blender

```bash
blender dali.obj
```

## 3D Print

```bash
# Convert to 3MF for PrusaSlicer
meshlab -i dali.stl -o dali.3mf

# Or directly slice STL
prusa-slicer dali.stl
```

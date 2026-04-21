# Full GPU Pipeline

## Complete Stack

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    TETRAGRAMMATRON GPU RENDERING STACK                        │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│   APPLICATION                                                                │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │  tetra-viewer.c       —  PNG → OpenGL ES viewer                    │   │
│   │  tetra-host-viewer.c  —  IVSHMEM zero-copy viewer                  │   │
│   │  tetra-writer.c      —  IVSHMEM tile writer (guest)               │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│                                                                              │
│   POLYFORM LAYER                                                             │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │  tetra-png.h  —  PNG read/write with constitutional palette       │   │
│   │  polyform.h   —  1D/2D/2.5D/3D types                              │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│                                                                              │
│   KERNEL LAYER                                                               │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │  K(p,C)  —  rotl⊕rotl⊕rotr⊕C                                       │   │
│   │  SID     —  compute_sid(nf)                                        │   │
│   │  Pair    —  cons/car/cdr                                           │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│                                                                              │
│   GPU API LAYER                                                              │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │  OpenGL ES 2.0  —  tetra-gpu.h                                    │   │
│   │  EGL         —  context setup                                      │   │
│   │  IVSHMEM     —  shared memory framebuffer                          │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│                                                                              │
│   HARDWARE                                                                  │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │  QEMU RISC-V   —  virtio-gpu-gl / ivshmem-plain                  │   │
│   │  ESP32        —  SPI + GPU                                         │   │
│   │  Native       —  DRI /dev/dri/card0                                │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Pipeline 1: PNG → OpenGL ES

```
PNG File → libpng → polyform_read_png() 
         → constitutional palette extraction 
         → RGBA conversion 
         → glTexImage2D() 
         → OpenGL ES texture 
         → vertex shader 
         → fragment shader 
         → framebuffer 
         → display
```

## Pipeline 2: IVSHMEM Zero-Copy

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    IVSHMEM ZERO-COPY PIPELINE                                │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│   Host: /dev/shm/mesh_fb (16 MB shared framebuffer)                        │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │  Tile 0    │  Tile 1    │  Tile 2    │  Tile 3    │                 │   │
│   │  (VM0)     │  (VM1)     │  (VM2)     │  (VM3)     │                 │   │
│   │  128×128   │  128×128   │  128×128   │  128×128   │                 │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│                                                                              │
│   Host Viewer: shm_open() → mmap() → glTexImage2D() → display             │
│                                                                              │
│   Guest VMs: write to /dev/tetra_fb → memory appears on host instantly    │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Files

| File | Description |
|------|-------------|
| `tetra-viewer.c` | PNG → OpenGL ES viewer |
| `tetra-host-viewer.c` | IVSHMEM zero-copy viewer |
| `tetra-writer.c` | IVSHMEM tile writer (guest) |
| `tetra-gpu.h` | GPU utilities |
| `tetra-png.h` | PNG with constitutional palette |
| `polyform.h` | 1D/2D/2.5D/3D types |
| `launch-ivshmem.sh` | QEMU launch with IVSHMEM |

## QEMU RISC-V with VirGL (PNG Pipeline)

```bash
qemu-system-riscv64 \
    -machine virt \
    -cpu rv64 \
    -smp 4 \
    -m 2G \
    -device virtio-gpu-gl \
    -display sdl,gl=on \
    -drive file=us.qcow2,format=qcow2,if=virtio
```

## QEMU RISC-V with IVSHMEM (Zero-Copy Pipeline)

```bash
# Create shared memory
truncate -s 16M /dev/shm/mesh_fb
chmod 666 /dev/shm/mesh_fb

# Launch with IVSHMEM
qemu-system-riscv64 \
    -machine virt \
    -cpu rv64 \
    -smp 4 \
    -m 2G \
    -device ivshmem-plain,memdev=fb_mem \
    -object memory-backend-file,id=fb_mem,size=16M,mem-path=/dev/shm/mesh_fb,share=on \
    -drive file=us.qcow2,format=qcow2,if=virtio \
    -nographic
```

Or use the launch script:
```bash
chmod +x launch-ivshmem.sh
./launch-ivshmem.sh 0 2G
```

## Build

```bash
# Host viewer (IVSHMEM)
gcc -o tetra-host-viewer tetra-host-viewer.c -lGLESv2 -lEGL -lrt -lm

# PNG viewer
gcc -o tetra-viewer tetra-viewer.c -lGL -lEGL -lpng -lm -DUSE_GL=1

# Guest writer (cross-compile for RISC-V)
riscv64-linux-gnu-gcc -o tetra-writer tetra-writer.c
```

## Run

```bash
# Terminal 1: Launch host viewer
./tetra-host-viewer

# Terminal 2: Launch IVSHMEM VMs
./launch-ivshmem.sh 0 &
./launch-ivshmem.sh 1 &
./launch-ivshmem.sh 2 &

# Terminal 3: Guest writers (inside VMs)
./tetra-writer 0 1   # VM0: constitutional gray
./tetra-writer 1 2   # VM1: red
./tetra-writer 2 4   # VM2: blue
```

## Constitutional Constants

| Constant | Value | Description |
|----------|-------|-------------|
| CONSTITUTIONAL_C | 0x1D | GS - Group Separator |
| FS | 0x1C | File Separator |
| RS | 0x1E | Record Separator |
| US | 0x1F | Unit Separator |
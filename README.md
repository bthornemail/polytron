# seed-qemu

Bare-metal RISC-V kernel that runs in QEMU and exports images.

```
state -> serial text
state -> pixel buffer -> serial PGM
lisp expression -> pixel buffer -> serial PGM   (future)
```

## Prerequisites

```bash
# Ubuntu / Debian
sudo apt install gcc-riscv64-unknown-elf qemu-system-misc

# macOS (Homebrew)
brew install riscv-gnu-toolchain qemu
```

## Stages

The single source file `kernel.c` compiles to four stages.
Build any stage with `make STAGE=N`.

| Stage | What it does                            | Command         |
|-------|-----------------------------------------|-----------------|
| 1     | Boot + print `HELLO` over serial        | `make run1`     |
| 2     | Print 16 steps of the bit law as hex    | `make run2`     |
| 3     | Fill pixel buffer, print first row      | `make run3`     |
| 4     | Emit full 64×64 PGM over serial         | `make pgm`      |

Exit QEMU at any stage: `Ctrl-A` then `X`.

## Quick start

```bash
# build and capture 64x64 PGM
make pgm

# build, capture, and open image
make run
```

Output file: `out.pgm` (raw P5 PGM, 64×64 grayscale).

## Primitive law

```c
rotl(x,1) ^ rotl(x,3) ^ rotr(x,2) ^ C
```

Width is 16 bits. `C` is the constant injection.
Seed and `C` are set in `kernel.c` — change them freely.

## File map

```
start.S     assembly entry — sets stack, calls main, halts
kernel.c    all logic — law, pixel buffer, PGM export
linker.ld   memory map — code at 0x80000000, 16K stack
Makefile    build + run shortcuts
```

## Roadmap

```
1. QEMU boot          ← you are here (Stage 1)
2. serial output      ← Stage 2
3. pixel buffer       ← Stage 3
4. PGM export         ← Stage 4
5. tagged values
6. cons heap
7. Lisp primitives
8. Lisp evaluator
9. Lisp writes images
```

Steps 5–9 will be added to `kernel.c` once Stage 4 is stable.
# polytron

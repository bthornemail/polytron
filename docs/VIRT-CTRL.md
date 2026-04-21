# Tetragrammatron virt-ctrl Device

The virt-ctrl device is a QOM-based MMIO device that provides guest VM access to the constitutional clock tree and IVSHMEM framebuffer.

## Architecture

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                    TETRAGRAMMATRON VIRT-CTRL SYSTEM                           │
├─────────────────────────────────────────────────────────────────────────────┤
│                                                                              │
│   QEMU Host                                                                  │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │  tetra-ctrl (MMIO)                                                   │   │
│   │  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐            │   │
│   │  │ FEATURES │  │ COMMAND  │  │  STATE   │  │   SID    │            │   │
│   │  │   (RO)   │  │   (WO)   │  │   (RW)   │  │   (RO)   │            │   │
│   │  └──────────┘  └──────────┘  └──────────┘  └──────────┘            │   │
│   │       │              │              │              │                 │   │
│   │       ▼              ▼              ▼              ▼                 │   │
│   │  ┌──────────────────────────────────────────────────────────────┐  │   │
│   │  │                    GNOMON State Machine                        │  │   │
│   │  │   K(p,C) = rotl(p,1) ^ rotl(p,3) ^ rotr(p,2) ^ C             │  │   │
│   │  └──────────────────────────────────────────────────────────────┘  │   │
│   │                              │                                      │   │
│   │              ┌───────────────┼───────────────┐                     │   │
│   │              ▼               ▼               ▼                     │   │
│   │       ┌──────────┐   ┌──────────────┐   ┌──────────┐             │   │
│   │       │TetraClock│   │   IVSHMEM    │   │  Timer   │             │   │
│   │       │FS/GS/RS/US│   │ Framebuffer │   │  GNOMON  │             │   │
│   │       └──────────┘   └──────────────┘   └──────────┘             │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│                                                                              │
│   Guest VM                                                                   │
│   ┌─────────────────────────────────────────────────────────────────────┐   │
│   │  tetra-ctrl.ko (Kernel Module)                                       │   │
│   │  ┌──────────────────────────────────────────────────────────────┐  │   │
│   │  │  /sys/devices/platform/tetra-ctrl/                           │  │   │
│   │  │  ├── gnomon   (WO) — Trigger GNOMON step                     │  │   │
│   │  │  ├── omicron  (WO) — Toggle Omicron mode                      │  │   │
│   │  │  ├── state    (RO) — Current state                           │  │   │
│   │  │  ├── sid      (RO) — Current SID                             │  │   │
│   │  │  ├── period   (RO) — GNOMON period                            │  │   │
│   │  │  ├── cycle    (RO) — Cycle position (0-7)                    │  │   │
│   │  │  ├── channel  (RW) — FS/GS/RS/US select                      │  │   │
│   │  │  ├── constant (RW) — Constitutional C (default 0x1D)          │  │   │
│   │  │  ├── framebuf (RW) — IVSHMEM offset                          │  │   │
│   │  │  └── reset    (WO) — Reset to initial state                   │  │   │
│   │  └──────────────────────────────────────────────────────────────┘  │   │
│   └─────────────────────────────────────────────────────────────────────┘   │
│                                                                              │
└─────────────────────────────────────────────────────────────────────────────┘
```

## Register Map (256 bytes, 32-bit MMIO)

| Offset | Name      | Access | Description                              |
|--------|-----------|--------|------------------------------------------|
| 0x00   | FEATURES  | RO     | Supported features bitmask               |
| 0x04   | COMMAND   | WO     | Guest command register                  |
| 0x08   | STATE     | RW     | Constitutional state (pair)             |
| 0x0C   | SID       | RO     | Current SID (K(state, 0x1D))             |
| 0x10   | GNOMON    | WO     | Trigger GNOMON step                     |
| 0x14   | OMICRON   | WO     | Toggle Omicron mode                      |
| 0x18   | CHANNEL   | RW     | FS/GS/RS/US channel select              |
| 0x1C   | FRAMEBUF  | RW     | IVSHMEM framebuffer offset              |
| 0x20   | CONSTANT  | RW     | Constitutional C (default 0x1D)         |
| 0x24   | PERIOD    | RO     | Current GNOMON period                   |
| 0x28   | CYCLE     | RO     | Cycle position (0-7)                    |

## Feature Bits

| Bit   | Name                | Description                    |
|-------|---------------------|--------------------------------|
| 0     | POWER_CTRL          | Halt/Panic commands            |
| 1     | GNOMON              | GNOMON stepping                |
| 2     | OMICRON             | Omicron mode toggle            |
| 3     | CHANNEL             | Channel selection              |
| 4     | FRAMEBUF            | IVSHMEM framebuffer access     |
| 5     | CLOCK               | Clock tree integration         |
| 6     | CYCLE               | Cycle detection                |

## Commands (WO to 0x04)

| Value | Name      | Description                              |
|-------|-----------|------------------------------------------|
| 0     | NOOP      | No operation                             |
| 1     | RESET     | Reset to initial state (0x4242)         |
| 2     | HALT      | Request VM shutdown                      |
| 3     | PANIC     | Request VM panic                         |
| 4     | GNOMON    | Trigger single GNOMON step               |
| 5     | OMICRON   | Toggle Omicron mode                      |
| 6     | WRITE_FB  | Write SID to framebuffer                |
| 7     | SYNC      | Sync with clock tree                     |

## Channels

| Value | Name | Description              |
|-------|------|-------------------------|
| 0x1C  | FS   | File Separator (base)   |
| 0x1D  | GS   | Group Separator (constitutional) |
| 0x1E  | RS   | Record Separator (reference) |
| 0x1F  | US   | Unit Separator (active) |

## GNOMON Cycle

The GNOMON function K(p,C) has period 8:
```
0x4242 → 0x061B → 0xFD75 → 0x6E04 → 0xB7B7 → 0x3F22 → 0x0880 → 0x573D → (repeat)
```

The PERIOD register tracks detected cycle length, and CYCLE tracks position within cycle.

## Files

| File | Description |
|------|-------------|
| `qemu/hw/misc/tetra-ctrl.c` | QOM device implementation |
| `qemu/hw/misc/tetra-clock-qom.c` | Clock device implementation |
| `driver/tetra-ctrl.c` | Guest Linux kernel module |
| `driver/Makefile` | Driver build makefile |

## QEMU Command Line

```bash
qemu-system-riscv64 \
    -machine virt \
    -cpu rv64 \
    -smp 4 \
    -m 2G \
    -device tetra-clock-qom \
    -device tetra-ctrl,timer=on \
    -device ivshmem-plain,memdev=fb_mem \
    -object memory-backend-file,id=fb_mem,size=16M,mem-path=/dev/shm/mesh_fb,share=on \
    -display sdl,gl=on
```

## Guest Usage

```bash
# Trigger GNOMON step
echo 1 > /sys/devices/platform/tetra-ctrl/gnomon

# Toggle Omicron mode
echo 1 > /sys/devices/platform/tetra-ctrl/omicron

# Read current state
cat /sys/devices/platform/tetra-ctrl/state

# Read current SID
cat /sys/devices/platform/tetra-ctrl/sid

# Read GNOMON period
cat /sys/devices/platform/tetra-ctrl/period

# Read cycle position
cat /sys/devices/platform/tetra-ctrl/cycle

# Set channel (e.g., US = 0x1F)
echo 0x1F > /sys/devices/platform/tetra-ctrl/channel

# Reset to initial state
echo 1 > /sys/devices/platform/tetra-ctrl/reset
```

## Guest Driver Build

```bash
cd driver
make
sudo insmod tetra-ctrl.ko
```

## Integration Notes

The virt-ctrl device integrates with:

1. **TetraClock** - Provides FS/GS/RS/US clock outputs that can be modulated by constitutional state
2. **IVSHMEM** - Writes SID/state to shared framebuffer for host visualization
3. **Timer** - Optional autonomous GNOMON stepping (100ms interval when enabled)

The device uses COBS framing for any network communication and maintains constitutional state across resets via VMState.

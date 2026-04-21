# TETRA-ESP32 — Tetragrammatron on ESP32

Pure C implementation of the Tetragrammatron constitutional stack for ESP32. No Python.

## The Constitutional Stack

```
pair → kernel → COBS → SID → witness → NRR → block chain → mesh
```

- **Pair**: 16-bit word (car=high byte, cdr=low byte)
- **Kernel K(p,C)**: `rotl(p,1) ^ rotl(p,3) ^ rotr(p,2) ^ C`
- **COBS**: Self-contained switching unit with s-bit
- **SID**: Semantic Identifier = K(value, 0x1D)
- **Witness**: Replay proof pack
- **NRR**: Non-Repudiation Receipt append-only log
- **Block Chain**: FS ← GS ← RS ← US

## Project Structure

```
esp32/
├── CMakeLists.txt
├── sdkconfig.defaults
├── main/
│   ├── CMakeLists.txt
│   ├── tetra-kernel.h    # Core: pair, K, COBS, SID,Witness, NRR, chain
│   └── tetra-main.c     # ESP32 application
└── README.md
```

## Build

```bash
# Set up ESP-IDF (one-time)
. ~/esp/esp-idf/export.sh

# Build
cd esp32
idf.py build

# Run in QEMU (no hardware needed)
idf.py qemu monitor

# Or flash to real ESP32
idf.py -p /dev/ttyUSB0 flash monitor
```

## QEMU Output

```
╔══════════════════════════════════════════════════════════════╗
║     TETRAGRAMMATRON ESP32 NODE — CONSTITUTIONAL STACK    ║
╚══════════════════════════════════════════════════════════════╝

ESP32: 2 cores, Wi-Fi/BT/BLE, 4 MB flash
Constitutional: pair → kernel → COBS → SID → witness → NRR → chain

Wi-Fi connecting to SSID:YourWiFi...
═══════════════════════════════════════════════════════════════
  TETRAGRAMMATRON ESP32 NODE
═══════════════════════════════════════════════════════════════
Listening on port 31415
Kernel K(p,C) = rotl(p,1) ^ rotl(p,3) ^ rotr(p,2) ^ C
Constitutional constant: 0x1D (GS - Group Separator)
Block chain layers: FS ← GS ← RS ← US
═══════════════════════════════════════════════════════════════
Block chain initialized: root=0x061B
NRR log initialized: genesis=0x001D
Tetra-Node task started
```

## Test

From a separate terminal:

```bash
# Send a test packet
echo -en '\x00\x42\x42\x00' | nc -u 127.0.0.1 31415

# Or use the tetra-client
cd ..
./tetra-client 127.0.0.1 31415
```

## Constants

| Constant | Value | Description |
|----------|-------|------------|
| CONSTITUTIONAL_C | 0x1D | GS (Group Separator) |
| LAYER_FS | 0x1C | File Separator |
| LAYER_GS | 0x1D | Group Separator |
| LAYER_RS | 0x1E | Record Separator |
| LAYER_US | 0x1F | Unit Separator |
| COBS_DELIMITER | 0x00 | Frame delimiter |
| COBS_ESCAPE | 0x01 | Stuffing byte |

## Constitutional Separators Table

| Codepoint | Name | Plane | Role |
|----------|------|-------|------|
| 0x1C | FS | Hex×Decimal | File - base layer |
| 0x1D | GS | Hex×Binary | Group - overlay 1 |
| 0x1E | RS | Decimal×Binary | Record - overlay 2 |
| 0x1F | US | All three | Unit - active layer |
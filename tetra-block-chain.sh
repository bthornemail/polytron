#!/bin/bash
# tetra-block-chain.sh
# Constitutional Block Chain: FS ← GS ← RS ← US

set -e

TETRA_DIR="/home/main/Documents/Tron"
cd "$TETRA_DIR"

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║     TETRAGRAMMATRON BLOCK CHAIN — FS ← GS ← RS ← US       ║"
echo "║     Constitutional Separators as QEMU Block Layers          ║"
echo "╚══════════════════════════════════════════════════════════════╝"

# -------------------------------------------------------------------
# 1. Check for QEMU
# -------------------------------------------------------------------

if ! command -v qemu-img &> /dev/null; then
    echo "ERROR: qemu-img not found. Install qemu-utils."
    exit 1
fi

echo ""
echo "=== Creating Constitutional Block Images ==="

# -------------------------------------------------------------------
# 2. Create the constitutional block images
# -------------------------------------------------------------------

# FS: File Separator (base layer) — Hex×Decimal plane, s=0
qemu-img create -f qcow2 fs.qcow2 64M 2>/dev/null || true
echo "Created fs.qcow2 (FS layer — File Separator, Hex×Decimal plane, s=0)"

# GS: Group Separator (overlay 1) — Hex×Binary plane, s=0
qemu-img create -f qcow2 -b fs.qcow2 -F qcow2 gs.qcow2 2>/dev/null || true
echo "Created gs.qcow2 (GS layer — Group Separator, Hex×Binary plane, s=0)"

# RS: Record Separator (overlay 2) — Decimal×Binary plane, s=0
qemu-img create -f qcow2 -b gs.qcow2 -F qcow2 rs.qcow2 2>/dev/null || true
echo "Created rs.qcow2 (RS layer — Record Separator, Decimal×Binary plane, s=0)"

# US: Unit Separator (active layer) — All three planes, s=1
qemu-img create -f qcow2 -b rs.qcow2 -F qcow2 us.qcow2 2>/dev/null || true
echo "Created us.qcow2 (US layer — Unit Separator, All planes, s=1)"

echo ""
echo "Block chain: [FS] ← [GS] ← [RS] ← [US]"

# -------------------------------------------------------------------
# 3. Show the backing chain
# -------------------------------------------------------------------

echo ""
echo "=== Backing Chain ==="
qemu-img info us.qcow2 | grep -E "image|backing file|format|virtual size"

# -------------------------------------------------------------------
# 4. Create minimal Tetra-Node rootfs
# -------------------------------------------------------------------

echo ""
echo "=== Creating Tetra-Node Rootfs ==="

rm -rf tetra-root
mkdir -p tetra-root/{bin,dev,proc,sys,tmp,etc}

# Copy the tetra-node binary
cp tetra-node tetra-root/bin/ 2>/dev/null || echo "Note: tetra-node not found, using stub"
cp tetra-client tetra-root/bin/ 2>/dev/null || echo "Note: tetra-client not found"

# Create a simple init
cat > tetra-root/init << 'EOF'
#!/bin/sh

echo "═══════════════════════════════════════════════════════════════"
echo "  TETRAGRAMMATRON BLOCK CHAIN NODE"
echo "═══════════════════════════════════════════════════════════════"
echo ""
echo "Active layer: US (Unit Separator, s=1)"
echo "Backing chain: FS ← GS ← RS ← US"
echo ""
echo "Constitutional constants:"
echo "  FS (0x1C): File Separator — Base layer"
echo "  GS (0x1D): Group Separator — Constitutional C"
echo "  RS (0x1E): Record Separator — Deltas"
echo "  US (0x1F): Unit Separator — Active state"
echo ""

mount -t proc proc /proc 2>/dev/null || true
mount -t sysfs sysfs /sys 2>/dev/null || true
mount -t devtmpfs dev /dev 2>/dev/null || true

# Try to start tetra-node
if [ -x /bin/tetra-node ]; then
    echo "Starting Tetra-Node on port 0x1F (US codepoint)..."
    /bin/tetra-node 31 &
fi

echo "Tetra-Node block chain ready."
echo ""
echo "Commands: sid, chain, exit"

while true; do
    echo -n "tetra> "
    read cmd
    case "$cmd" in
        sid)
            echo "Computing SID of current state..."
            ;;
        chain)
            echo "FS ← GS ← RS ← US"
            ;;
        info)
            echo "Layer: US (0x1F)"
            echo "s-bit: 1"
            echo "Constitutional constant: 0x1D (GS)"
            ;;
        exit|shutdown)
            echo "Shutting down..."
            poweroff -f
            ;;
        *)
            echo "Commands: sid, chain, info, exit"
            ;;
    esac
done
EOF

chmod +x tetra-root/init

# Create minimal fstab
cat > tetra-root/etc/fstab << 'EOF'
/dev/root / ext2 rw,noauto 0 1
proc /proc proc rw 0 0
sysfs /sys sysfs rw 0 0
devtmpfs /dev devtmpfs rw 0 0
EOF

# Create initramfs
cd tetra-root
find . -print0 | cpio --null -ov --format=newc 2>/dev/null | gzip -9 > ../tetra-initramfs.cpio.gz
cd ..

echo "Created tetra-initramfs.cpio.gz"

# -------------------------------------------------------------------
# 5. Check for QEMU system
# -------------------------------------------------------------------

echo ""
echo "=== QEMU System Check ==="

if command -v qemu-system-riscv64 &> /dev/null; then
    echo "QEMU RISC-V found: qemu-system-riscv64"
    
    echo ""
    echo "=== Ready to Launch ==="
    echo ""
    echo "To run the Tetra-Node in QEMU:"
    echo ""
    echo "  qemu-system-riscv64 \\"
    echo "    -machine virt \\"
    echo "    -cpu rv64 \\"
    echo "    -smp 2 \\"
    echo "    -m 128M \\"
    echo "    -drive file=us.qcow2,format=qcow2,if=virtio \\"
    echo "    -initrd tetra-initramfs.cpio.gz \\"
    echo "    -nographic \\"
    echo "    -netdev user,id=net0,hostfwd=tcp::10031-:31 \\"
    echo "    -device virtio-net-device,netdev=net0"
    echo ""
    echo "Or run: ./run-qemu.sh"
    
    # Create run script
    cat > run-qemu.sh << 'RUNEOF'
#!/bin/bash
qemu-system-riscv64 \
    -machine virt \
    -cpu rv64 \
    -smp 2 \
    -m 128M \
    -drive file=us.qcow2,format=qcow2,if=virtio \
    -initrd tetra-initramfs.cpio.gz \
    -nographic \
    -netdev user,id=net0,hostfwd=tcp::10031-:31 \
    -device virtio-net-device,netdev=net0
RUNEOF
    chmod +x run-qemu.sh
    
else
    echo "QEMU RISC-V not found. Install qemu-system-riscv64."
    echo ""
    echo "You can still test locally with:"
    echo "  ./tetra-node 31415"
    echo "  ./tetra-client 127.0.0.1 31415"
fi

# -------------------------------------------------------------------
# 6. Summary
# -------------------------------------------------------------------

echo ""
echo "╔══════════════════════════════════════════════════════════════╗"
echo "║              BLOCK CHAIN READY                             ║"
echo "╠══════════════════════════════════════════════════════════════╣"
echo "║  Layer  │ Codepoint │ Plane           │ s-bit │ Role    ║"
echo "╠══════════════════════════════════════════════════════════════╣"
echo "║  FS      │   0x1C    │ Hex×Decimal    │   0   │ File    ║"
echo "║  GS      │   0x1D    │ Hex×Binary     │   0   │ Group   ║"
echo "║  RS      │   0x1E    │ Decimal×Binary │   0   │ Record  ║"
echo "║  US      │   0x1F    │ All three      │   1   │ Unit    ║"
echo "╚══════════════════════════════════════════════════════════════╝"

echo ""
echo "Files created:"
ls -la *.qcow2 tetra-initramfs.cpio.gz run-qemu.sh 2>/dev/null | awk '{print "  " $9}'

echo ""
echo "Test locally: ./tetra-node 31415 & ./tetra-client"
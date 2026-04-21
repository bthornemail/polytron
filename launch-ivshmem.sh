#!/bin/bash
#
# TETRA-IVSHMEM-LAUNCH — Launch QEMU with IVSHMEM framebuffer
#
# Usage:
#   ./launch-ivshmem.sh [vm_id] [memory_size]
#
# Example:
#   ./launch-ivshmem.sh 0 2G   # Launch VM 0 with 2GB RAM
#

set -e

VM_ID=${1:-0}
MEM_SIZE=${2:-2G}
PORT=$((31415 + VM_ID))

# Paths
SHM_FB_PATH="/dev/shm/mesh_fb"
FB_SIZE="16M"
HOLE_MAP_PATH="/dev/shm/hole_map"
HOLE_SIZE="1M"

# Create shared memory files if they don't exist
if [ ! -f "$SHM_FB_PATH" ]; then
    echo "Creating shared framebuffer: $SHM_FB_PATH"
    truncate -s "$FB_SIZE" "$SHM_FB_PATH"
    chmod 666 "$SHM_FB_PATH"
fi

if [ ! -f "$HOLE_MAP_PATH" ]; then
    echo "Creating hole map: $HOLE_MAP_PATH"
    truncate -s "$HOLE_SIZE" "$HOLE_MAP_PATH"
    chmod 666 "$HOLE_MAP_PATH"
fi

# QEMU disk image
IMG_PATH="us.qcow2"

if [ ! -f "$IMG_PATH" ]; then
    echo "Error: Disk image not found: $IMG_PATH"
    echo "Run tetra-block-chain.sh first to create images"
    exit 1
fi

echo "═══ TETRA-IVSHMEM LAUNCH ═══"
echo "VM ID: $VM_ID"
echo "Memory: $MEM_SIZE"
echo "Port: $PORT"
echo "Framebuffer: $SHM_FB_PATH ($FB_SIZE)"
echo ""

# Launch QEMU with IVSHMEM
exec qemu-system-riscv64 \
    -machine virt \
    -cpu rv64gc \
    -smp 2 \
    -m "$MEM_SIZE" \
    -nographic \
    \
    -device ivshmem-plain,memdev=fb_mem,shm="$SHM_FB_PATH" \
    -object memory-backend-file,id=fb_mem,size="$FB_SIZE",mem-path="$SHM_FB_PATH,share=on" \
    \
    -device ivshmem-plain,memdev=hole_mem,shm="$HOLE_MAP_PATH" \
    -object memory-backend-file,id=hole_mem,size="$HOLE_SIZE",mem-path="$HOLE_MAP_PATH,share=on" \
    \
    -kernel opensbi.bin \
    -append "console=ttyS0" \
    -drive file="$IMG_PATH",format=qcow2,if=virtio \
    \
    -netdev user,id=net0,hostfwd=tcp::${PORT}-:22 \
    -device virtio-net-device,netdev=net0

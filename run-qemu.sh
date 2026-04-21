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

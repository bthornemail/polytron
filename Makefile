# Tetragrammatron Makefile
# 5 QEMU Versions: RISC-V, TCG, ESP32, iMIMO, Virt-Ctrl

CROSS   := riscv64-linux-gnu-
CC      := $(CROSS)gcc
OBJCOPY := $(CROSS)objcopy

STAGE   ?= 3
QEMU    ?= qemu-system-riscv64

CFLAGS  := -march=rv64imac -mabi=lp64 -mcmodel=medany \
           -ffreestanding -nostdlib -nostartfiles -no-pie \
           -O2 -Wall -DSTAGE=$(STAGE)

LDFLAGS := -T linker.ld -nostdlib -nostartfiles

TARGET  := kernel.elf
SRCS    := start.S kernel.c

DEVICE_DIR := qemu/hw/misc
DEVICES   := $(DEVICE_DIR)/tetra-ctrl.c $(DEVICE_DIR)/tetra-clock-qom.c $(DEVICE_DIR)/tetra-semantic.c

QEMU_PORT_BASE ?= 10001

.PHONY: all clean help \
        run run1 run2 run3 run4 pgm view \
        build-riscv build-tcg build-esp32 build-imimo build-virtctrl \
        launch-node1 launch-node2 launch-node3 launch-mesh \
        profile-tcg profile-perf profile-flamegraph \
        install-devices qom-inspect

all: $(TARGET)

$(TARGET): $(SRCS) linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRCS)

# ============================================================
# KERNEL STAGES (existing)
# ============================================================

run1: STAGE=1
run1: all
	$(QEMU) -machine virt -cpu rv64 -bios none -nographic -kernel $(TARGET)

run2: STAGE=2
run2: all
	$(QEMU) -machine virt -cpu rv64 -bios none -nographic -kernel $(TARGET)

run3: STAGE=3
run3: all
	$(QEMU) -machine virt -cpu rv64 -bios none -nographic -kernel $(TARGET)

run4: STAGE=4
run4: all
	$(QEMU) -machine virt -cpu rv64 -bios none -nographic -kernel $(TARGET)

pgm: STAGE=4
pgm: all
	$(QEMU) -machine virt -cpu rv64 -bios none -nographic -kernel $(TARGET) > out.pgm
	@echo "wrote out.pgm"

view: out.pgm
	xdg-open out.pgm 2>/dev/null || feh out.pgm 2>/dev/null || cat out.pgm

run: run3

# ============================================================
# 5 QEMU VERSIONS
# ============================================================

# 1. RISC-V virt - Main Tetragrammatron runtime
build-riscv:
	@echo "=== RISC-V virt ==="
	@echo "Uses system qemu-system-riscv64"
	@echo "Run: make launch-node1"

# 2. TCG - Tiny Code Generator with profiling
build-tcg:
	@echo "=== TCG Build ==="
	@echo "Configure QEMU with: --enable-tcg --disable-kvm"
	@echo "TCG flags: -DTCG_PROFILE -DTCG_DEBUG"
	@echo "Profile with: make profile-tcg"

# 3. ESP32 - Xtensa emulation
build-esp32:
	@echo "=== ESP32 Build ==="
	@echo "Requires ESP-IDF: cd esp32 && idf.py build"
	@echo "Or QEMU xtensa: qemu-system-xtensa -L ... -kernel esp32/build/boot.elf"

# 4. iMIMO - IVSHMEM + MMIO zero-copy framebuffer
build-imimo:
	@echo "=== iMIMO Build ==="
	@echo "Requires ivshmem-platform device: -device ivshmem-plain,shm=mesh_fb"
	@echo "Run: ./launch-ivshmem.sh"

# 5. Virt-Ctrl - QOM device integration
build-virtctrl:
	@echo "=== Virt-Ctrl Device ==="
	@echo "Devices: $(DEVICES)"
	@echo "Install: make install-devices"

# ============================================================
# DEVICE MANAGEMENT
# ============================================================

install-devices:
	@echo "Installing Tetragrammatron devices..."
	@mkdir -p $(DEVICE_DIR)
	@for dev in $(DEVICES); do \
		if [ -f "$$dev" ]; then \
			cp $$dev $(DEVICE_DIR)/; \
			echo "  Copied $$dev"; \
		else \
			echo "  Skipped $$dev (not found)"; \
		fi; \
	done
	@echo "Device installation complete."

# ============================================================
# NODE LAUNCH (3-node mesh)
# ============================================================

launch-node1:
	$(QEMU) \
		-name tetra-node-1 \
		-machine virt -cpu rv64 -smp 2 -m 256M \
		-drive file=node1.qcow2,format=qcow2,if=virtio \
		-device tetra-ctrl \
		-device tetra-clock-qom \
		-device tetra-semantic \
		-netdev user,id=net0,hostfwd=tcp::$(QEMU_PORT_BASE)-:31415 \
		-device virtio-net-device,netdev=net0 \
		-qmp unix:/tmp/qmp-node1.sock,server,nowait \
		-nographic

launch-node2:
	$(QEMU) \
		-name tetra-node-2 \
		-machine virt -cpu rv64 -smp 2 -m 256M \
		-drive file=node2.qcow2,format=qcow2,if=virtio \
		-device tetra-ctrl \
		-device tetra-clock-qom \
		-device tetra-semantic \
		-netdev user,id=net0,hostfwd=tcp::$(shell echo $(QEMU_PORT_BASE) + 1 | bc)-:31415 \
		-device virtio-net-device,netdev=net0 \
		-qmp unix:/tmp/qmp-node2.sock,server,nowait \
		-nographic

launch-node3:
	$(QEMU) \
		-name tetra-node-3 \
		-machine virt -cpu rv64 -smp 2 -m 256M \
		-drive file=node3.qcow2,format=qcow2,if=virtio \
		-device tetra-ctrl \
		-device tetra-clock-qom \
		-device tetra-semantic \
		-netdev user,id=net0,hostfwd=tcp::$(shell echo $(QEMU_PORT_BASE) + 2 | bc)-:31415 \
		-device virtio-net-device,netdev=net0 \
		-qmp unix:/tmp/qmp-node3.sock,server,nowait \
		-nographic

launch-mesh: launch-node1 launch-node2 launch-node3
	@echo "3-node Tetragrammatron mesh launched"
	@echo "Node 1: port $(QEMU_PORT_BASE), QMP /tmp/qmp-node1.sock"
	@echo "Node 2: port $$(($(QEMU_PORT_BASE) + 1)), QMP /tmp/qmp-node2.sock"
	@echo "Node 3: port $$(($(QEMU_PORT_BASE) + 2)), QMP /tmp/qmp-node3.sock"

# ============================================================
# PROFILING
# ============================================================

profile-tcg:
	$(QEMU) -d op,op_opt,out_asm -D tcg-profile.log \
		-machine virt -cpu rv64 -smp 1 -m 128M \
		-device tetra-semantic \
		-kernel $(TARGET)
	@echo "TCG profile written to tcg-profile.log"
	@grep -E "tetra_kernel|K\(" tcg-profile.log | head -20 || true

profile-perf:
	perf record -g $(QEMU) \
		-machine virt -cpu rv64 -smp 1 -m 128M \
		-device tetra-semantic \
		-kernel $(TARGET)
	perf report

profile-flamegraph:
	perf record -g -F 99 $(QEMU) \
		-machine virt -cpu rv64 -smp 1 -m 128M \
		-device tetra-semantic \
		-kernel $(TARGET)
	perf script | stackcollapse-perf.pl 2>/dev/null | flamegraph.pl > tetra-flamegraph.svg || \
	perf script > perf.raw && echo "FlameGraph: perf script > perf.raw"

# ============================================================
# QOM INTROSPECTION
# ============================================================

qom-inspect:
	@echo "=== QOM Tree ==="
	@echo '{"execute":"qom-list","arguments":{"path":"/machine/peripheral"}}' | \
		nc -U /tmp/qmp-node1.sock 2>/dev/null | grep -i tetra || \
		echo "No QEMU running. Launch with: make launch-node1"
	@echo ""
	@echo "=== tetra-semantic SID ==="
	@echo '{"execute":"qom-get","arguments":{"path":"/machine/peripheral/tetra-semantic","property":"sid"}}' | \
		nc -U /tmp/qmp-node1.sock 2>/dev/null || true

# ============================================================
# CLEAN
# ============================================================

clean:
	rm -f $(TARGET) out.pgm tcg-profile.log perf.raw *.svg
	rm -f /tmp/qmp-node*.sock /tmp/mon-node*.sock
	-pkill -f "qemu-system-riscv64.*tetra-node" 2>/dev/null || true

# ============================================================
# HELP
# ============================================================

help:
	@echo "Tetragrammatron QEMU Matrix"
	@echo ""
	@echo "KERNEL:"
	@echo "  make run        - Run kernel (stage 3)"
	@echo "  make run1-4     - Run kernel stages 1-4"
	@echo "  make pgm        - Generate PGM output"
	@echo ""
	@echo "5 QEMU VERSIONS:"
	@echo "  make build-riscv    - RISC-V virt machine"
	@echo "  make build-tcg      - TCG with profiling"
	@echo "  make build-esp32    - ESP32 Xtensa"
	@echo "  make build-imimo    - IVSHMEM + MMIO"
	@echo "  make build-virtctrl - Virt-Ctrl QOM device"
	@echo ""
	@echo "LAUNCH:"
	@echo "  make launch-node1   - Launch node 1"
	@echo "  make launch-node2   - Launch node 2"
	@echo "  make launch-node3   - Launch node 3"
	@echo "  make launch-mesh    - Launch 3-node mesh"
	@echo ""
	@echo "PROFILING:"
	@echo "  make profile-tcg       - TCG opcode profiling"
	@echo "  make profile-perf      - Linux perf record"
	@echo "  make profile-flamegraph - Generate FlameGraph"
	@echo ""
	@echo "DEVICES:"
	@echo "  make install-devices   - Install QOM devices"
	@echo "  make qom-inspect       - Query QOM tree"
	@echo ""
	@echo "  make clean             - Stop all nodes, clean files"

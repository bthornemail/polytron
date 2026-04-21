CROSS   := riscv64-linux-gnu-
CC      := $(CROSS)gcc
OBJCOPY := $(CROSS)objcopy

STAGE   ?= 3

CFLAGS  := -march=rv64imac -mabi=lp64 -mcmodel=medany \
           -ffreestanding -nostdlib -nostartfiles -no-pie \
           -O2 -Wall -DSTAGE=$(STAGE)

LDFLAGS := -T linker.ld -nostdlib -nostartfiles

TARGET  := kernel.elf

SRCS    := start.S kernel.c

.PHONY: all clean run run1 run2 run3 run4 pgm view

all: $(TARGET)

$(TARGET): $(SRCS) linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRCS)

run1: STAGE=1
run1: all
	qemu-system-riscv64 -machine virt -cpu rv64 -bios none -nographic -kernel $(TARGET)

run2: STAGE=2
run2: all
	qemu-system-riscv64 -machine virt -cpu rv64 -bios none -nographic -kernel $(TARGET)

run3: STAGE=3
run3: all
	qemu-system-riscv64 -machine virt -cpu rv64 -bios none -nographic -kernel $(TARGET)

run4: STAGE=4
run4: all
	qemu-system-riscv64 -machine virt -cpu rv64 -bios none -nographic -kernel $(TARGET)

pgm: STAGE=4
pgm: all
	qemu-system-riscv64 -machine virt -cpu rv64 -bios none -nographic -kernel $(TARGET) > out.pgm
	@echo "wrote out.pgm"

view: out.pgm
	xdg-open out.pgm 2>/dev/null || feh out.pgm 2>/dev/null || cat out.pgm

clean:
	rm -f $(TARGET) out.pgm
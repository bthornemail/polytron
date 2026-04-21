# Polyform Makefile - Build all polyform generators

CC      ?= gcc
CFLAGS  := -lm -Wall -O2

TARGETS := karnaugh-gen barcode-render barcode-accurate qemu-profiler termux-probe

.PHONY: all clean test benchmark install termux docker probe help

all: $(TARGETS)

karnaugh-gen: karnaugh-gen.c
	$(CC) $(CFLAGS) -o $@ $<

barcode-render: barcode-render.c
	$(CC) $(CFLAGS) -o $@ $<

barcode-accurate: barcode-accurate.c
	$(CC) $(CFLAGS) -o $@ $<

qemu-profiler: qemu-profiler.c
	$(CC) $(CFLAGS) -o $@ $<

termux-probe: termux-probe.c
	$(CC) $(CFLAGS) -o $@ $<

test: all
	@echo "=== Testing Karnaugh generator ==="
	./karnaugh-gen
	@echo ""
	@echo "=== Testing barcode generator ==="
	./barcode-accurate
	@echo ""
	@echo "=== Testing QEMU profiler ==="
	./qemu-profiler -n 100
	@echo ""
	@echo "=== Testing Termux probe ==="
	./termux-probe --test-only

benchmark: all
	@echo "=== Benchmarking Karnaugh ==="
	./karnaugh-gen | grep -E "^(Rate|Time)"
	@echo ""
	@echo "=== Benchmarking QEMU profiler ==="
	./qemu-profiler -b 100

probe: termux-probe
	@echo "=== Running system probe ==="
	./termux-probe

termux:
	@echo "=== Termux Build Setup ==="
	@echo "Run this on Android/Termux:"
	@echo ""
	@echo "  1. Install Termux from F-Droid"
	@echo "  2. Run: apt install clang make"
	@echo "  3. Copy polyform source to Termux"
	@echo "  4. Run: make"
	@echo ""
	@echo "Or use the build script:"
	@echo "  bash build-termux.sh"
	@echo ""
	@echo "Inside Termux, to view SVGs:"
	@echo "  termux-open polyform/*.svg"

docker:
	@echo "=== Docker Build ==="
	@echo "Building Docker image..."
	docker build -t polyform-engine .
	@echo ""
	@echo "Run with:"
	@echo "  docker run --rm -v \$$(pwd)/output:/home/main/Documents/Tron/polyform/output polyform-engine"
	@echo ""
	@echo "Run specific tool:"
	@echo "  docker run --rm polyform-engine ./karnaugh-gen"

clean:
	rm -f $(TARGETS)
	rm -f *.svg *.ppm *.png
	rm -rf output

install:
	mkdir -p $(HOME)/bin
	install -m 755 $(TARGETS) $(HOME)/bin/

help:
	@echo "Polyform Logic Engine - Build Targets"
	@echo ""
	@echo "BUILD:"
	@echo "  make all           - Build all tools"
	@echo "  make karnaugh-gen - Build K-map generator"
	@echo "  make barcode-accurate - Build barcode generator"
	@echo "  make qemu-profiler - Build QEMU profiler"
	@echo "  make termux-probe  - Build Termux probe"
	@echo ""
	@echo "RUN:"
	@echo "  make test          - Run all tests"
	@echo "  make benchmark     - Run benchmarks"
	@echo "  make probe         - Run system probe"
	@echo ""
	@echo "DEPLOY:"
	@echo "  make termux        - Show Termux deployment"
	@echo "  make docker        - Build Docker image"
	@echo ""
	@echo "CLEAN:"
	@echo "  make clean         - Remove builds and outputs"
	@echo ""
	@echo "OUTPUTS: polyform/*.svg"

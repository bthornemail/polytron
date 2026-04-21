# Polyform Makefile - Build all polyform generators

CC      ?= gcc
CFLAGS  := -lm -Wall -O2

TARGETS := karnaugh-gen barcode-render

.PHONY: all clean test benchmark install

all: $(TARGETS)

karnaugh-gen: karnaugh-gen.c
	$(CC) $(CFLAGS) -o $@ $<

barcode-render: barcode-render.c
	$(CC) $(CFLAGS) -o $@ $<

test: all
	@echo "=== Testing Karnaugh generator ==="
	./karnaugh-gen
	@echo ""
	@echo "=== Testing barcode generator ==="
	./barcode-render

benchmark: all
	@echo "=== Benchmarking Karnaugh ==="
	./karnaugh-gen | grep -E "^(Rate|Time)"
	@echo ""
	@echo "=== Benchmarking barcode ==="
	@time ./barcode-render

clean:
	rm -f $(TARGETS)
	rm -f *.svg *.ppm *.png

install:
	mkdir -p $(HOME)/bin
	install -m 755 $(TARGETS) $(HOME)/bin/
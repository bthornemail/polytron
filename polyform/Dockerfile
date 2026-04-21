# Dockerfile for Polyform Logic Engine
# Build: docker build -t polyform-engine .
# Run:   docker run --rm -v $(pwd)/output:/home/main/Documents/Tron/polyform/output polyform-engine

FROM debian:bookworm-slim

# Install build tools
RUN apt-get update && apt-get install -y \
    gcc \
    make \
    clang \
    qemu-user \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /home/main/Documents/Tron/polyform

# Copy source
COPY *.c ./
COPY Makefile ./

# Build all tools
RUN make clean && make all

# Create output directory
RUN mkdir -p output/{barcodes,karnaugh,qemu,probes}

# Default command - run probe
CMD ["./termux-probe"]

# Run specific tool examples:
# docker run --rm polyform-engine ./karnaugh-gen
# docker run --rm polyform-engine ./barcode-accurate
# docker run --rm polyform-engine ./qemu-profiler -n 1000

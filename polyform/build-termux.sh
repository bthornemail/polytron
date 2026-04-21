#!/usr/bin/env bash
# build-termux.sh - Build Polyform Logic Engine for Termux/Android
# Run this script inside Termux on Android

set -e

echo "=== Polyform Logic Engine - Termux Build ==="
echo ""

# Check if running in Termux
if [ ! -d "/data/data/com.termux" ]; then
    echo "WARNING: Not running in Termux. Continuing anyway..."
fi

# Install dependencies
echo "[1/5] Installing Termux dependencies..."
apt-get update -qq
apt-get install -y -qq clang make qemu-user procps coreutils > /dev/null 2>&1 || \
    pkg install -y clang make qemu-user procps coreutils > /dev/null 2>&1 || true

# Check for required tools
for tool in clang make qemu-x86_64; do
    if ! command -v $tool &> /dev/null; then
        echo "ERROR: $tool not found. Install with: pkg install ${tool%-*}"
        exit 1
    fi
done

# Build polyform tools
echo "[2/5] Building polyform tools..."
cd "$(dirname "$0")"

# Clean previous builds
make clean 2>/dev/null || true

# Build with clang for Termux
make CC=clang CFLAGS="-lm -Wall -O2 -target=aarch64-linux-Android" 2>/dev/null || \
make CC=clang CFLAGS="-lm -Wall -O2" 2>/dev/null || \
make 2>/dev/null || true

echo "  Built: $(ls -1 *.c 2>/dev/null | wc -l) source files"

# Create output directory structure
echo "[3/5] Creating output directories..."
mkdir -p output/{barcodes,karnaugh,qemu,probes}

# Copy binaries and scripts
echo "[4/5] Packaging for distribution..."
cp -f karnaugh-gen output/ 2>/dev/null || true
cp -f barcode-render output/ 2>/dev/null || true  
cp -f barcode-accurate output/ 2>/dev/null || true
cp -f qemu-profiler output/ 2>/dev/null || true

# Create run wrapper
echo "[5/5] Creating run wrapper..."
cat > run-polyform.sh << 'WRAPPER'
#!/usr/bin/env bash
# run-polyform.sh - Run polyform tools on Termux/Android
# Usage: ./run-polyform.sh [tool] [args...]

TOOL_DIR="$(dirname "$0")"
TOOL="$1"
shift

if [ -z "$TOOL" ]; then
    echo "Usage: $0 <tool> [args...]"
    echo ""
    echo "Available tools:"
    ls -1 "$TOOL_DIR"/*.c 2>/dev/null | while read f; do
        name=$(basename "$f" .c)
        echo "  $name"
    done
    exit 1
fi

BINARY="$TOOL_DIR/$TOOL"
if [ ! -f "$BINARY" ]; then
    # Try to build
    make "$TOOL" 2>/dev/null && BINARY="$TOOL_DIR/$TOOL" || \
    (cd "$TOOL_DIR" && gcc -o "$TOOL" "${TOOL}.c" -lm -Wall)
fi

if [ ! -x "$BINARY" ]; then
    echo "ERROR: Cannot build or find: $TOOL"
    exit 1
fi

"$BINARY" "$@"
WRAPPER

chmod +x run-polyform.sh

echo ""
echo "=== Termux Build Complete ==="
echo ""
echo "Output directory: $(pwd)/output/"
echo ""
echo "To run tools:"
echo "  ./run-polyform.sh karnaugh-gen"
echo "  ./run-polyform.sh barcode-accurate"
echo "  ./run-polyform.sh qemu-profiler -n 1000"
echo ""
echo "View generated SVGs:"
echo "  termux-open output/karnaugh/*.svg"
echo ""

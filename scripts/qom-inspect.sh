#!/bin/bash
# qom-inspect.sh - QOM introspection for Tetragrammatron devices
# Usage: ./scripts/qom-inspect.sh [socket]

QMP_SOCKET=${1:-/tmp/qmp-node1.sock}

if [ ! -S "$QMP_SOCKET" ]; then
    echo "ERROR: QMP socket not found: $QMP_SOCKET"
    echo "Launch a node first: make launch-node1"
    exit 1
fi

qmp_send() {
    echo "$1" | nc -U "$QMP_SOCKET" 2>/dev/null
}

qmp_send '{"execute":"qmp_capabilities"}' > /dev/null

echo "=== Tetragrammatron QOM Tree ==="
echo "Socket: $QMP_SOCKET"
echo ""

echo "--- Devices ---"
qmp_send '{"execute":"qom-list","arguments":{"path":"/machine/peripheral"}}' | \
    grep -o '"name":"[^"]*"' | cut -d'"' -f4 | grep -E "tetra|clock|semantic" || echo "  (no tetra devices)"

echo ""
echo "--- tetra-ctrl properties ---"
qmp_send '{"execute":"qom-list","arguments":{"path":"/machine/peripheral/tetra-ctrl"}}' | \
    grep -o '"name":"[^"]*"' | cut -d'"' -f4 | head -10

echo ""
echo "--- tetra-semantic values ---"
SID=$(qmp_send '{"execute":"qom-get","arguments":{"path":"/machine/peripheral/tetra-semantic","property":"sid"}}' 2>/dev/null | grep -o '"return":[0-9a-f]*' | cut -d':' -f2)
MODE=$(qmp_send '{"execute":"qom-get","arguments":{"path":"/machine/peripheral/tetra-semantic","property":"mode"}}' 2>/dev/null | grep -o '"return":[0-9]*' | cut -d':' -f2)
COUNT=$(qmp_send '{"execute":"qom-get","arguments":{"path":"/machine/peripheral/tetra-semantic","property":"token_count"}}' 2>/dev/null | grep -o '"return":[0-9]*' | cut -d':' -f2)

echo "  SID: 0x${SID:-00000000}"
echo "  Mode: ${MODE:-0} (0=XX,1=Xx,2=xX,3=xx)"
echo "  Token count: ${COUNT:-0}"

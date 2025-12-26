#!/bin/bash
# Crypto Dashboard Runner Script
# This script runs the crypto dashboard and handles errors gracefully

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/../build/examples"
EXECUTABLE="${BUILD_DIR}/crypto_dashboard"

# Check if executable exists
if [ ! -f "$EXECUTABLE" ]; then
    echo "Error: crypto_dashboard executable not found at $EXECUTABLE"
    echo "Please build the project first:"
    echo "  cd /home/jg/code/e-Paper/build"
    echo "  cmake --build . --target crypto_dashboard"
    exit 1
fi

# Check if running as root (required for GPIO access)
if [ "$EUID" -ne 0 ]; then
    echo "This script requires root privileges for GPIO access."
    echo "Restarting with sudo..."
    exec sudo "$0" "$@"
fi

# Change to examples directory (where wallets.json should be)
cd "$SCRIPT_DIR" || exit 1

echo "Starting Crypto Dashboard..."
echo "Working directory: $(pwd)"
echo "Executable: $EXECUTABLE"
echo ""

# Run the dashboard
"$EXECUTABLE"

EXIT_CODE=$?

if [ $EXIT_CODE -eq 0 ]; then
    echo ""
    echo "Dashboard completed successfully!"
else
    echo ""
    echo "Dashboard exited with error code: $EXIT_CODE"
fi

exit $EXIT_CODE


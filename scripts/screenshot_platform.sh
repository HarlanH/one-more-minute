#!/bin/bash
# screenshot_platform.sh - Install app to emulator, start a timer, and capture screenshot
# Usage: ./scripts/screenshot_platform.sh <platform> [output_path]

set -e

PLATFORM="${1:?Usage: $0 <platform> [output_path]}"
OUTPUT="${2:-screenshots/${PLATFORM}.png}"
EMULATOR_PORT="${PEBBLE_EMULATOR_PORT:-9000}"

echo "=== Screenshot: ${PLATFORM} ==="

# Kill any running emulators
echo "Killing any running emulators..."
pkill -f "Pebble Emulator" 2>/dev/null || true
pkill -f "qemu-system" 2>/dev/null || true
sleep 2

# Install the app
echo "Installing app..."
pebble install --emulator "${PLATFORM}" 2>&1

# Wait for app to load
echo "Waiting 5s for app to load..."
sleep 5

# Press UP button to start timer 1
echo "Pressing UP button to start timer..."
pebble emu-button --emulator "${PLATFORM}" click up 2>&1 || {
  echo "Warning: Could not press button. Screenshot will show initial state."
}

# Wait for progress bar to fill (20 seconds)
echo "Waiting 20s for progress bar..."
sleep 20

# Take screenshot
echo "Capturing screenshot to ${OUTPUT}..."
mkdir -p "$(dirname "${OUTPUT}")"
pebble screenshot --emulator "${PLATFORM}" "${OUTPUT}" 2>&1

echo "=== Done: ${OUTPUT} ==="

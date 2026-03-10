#!/usr/bin/env bash
# src/build.sh — convenience wrapper for CMake preset builds
# Usage: bash src/build.sh [debug|release|gcc-debug] [source_dir]

set -euo pipefail

PRESET="${1:-debug}"
SOURCE_DIR="${2:-$(dirname "$0")}"

# Resolve absolute source path
SOURCE_DIR="$(cd "${SOURCE_DIR}" && pwd)"

# Validate preset
case "${PRESET}" in
    debug|release|gcc-debug) ;;
    *)
        echo "[build] ERROR: Unknown preset '${PRESET}'. Use: debug, release, gcc-debug" >&2
        exit 1
        ;;
esac

echo "[build] Configuring (preset=${PRESET})..."
cmake --preset "${PRESET}" -S "${SOURCE_DIR}"

echo "[build] Building..."
# Run cmake --build from source dir so it finds CMakePresets.json
(cd "${SOURCE_DIR}" && cmake --build --preset "${PRESET}")

echo "[build] Done."

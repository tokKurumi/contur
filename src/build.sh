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

if ! command -v conan >/dev/null 2>&1; then
    echo "[build] ERROR: Conan is required but not found in PATH." >&2
    exit 1
fi

case "${PRESET}" in
    debug|gcc-debug)
        CONAN_BUILD_TYPE="Debug"
        ;;
    release)
        CONAN_BUILD_TYPE="Release"
        ;;
esac

CONAN_OUTPUT_DIR="${SOURCE_DIR}/build/${PRESET}"
CONAN_GENERATORS_DIR="${CONAN_OUTPUT_DIR}/build/${CONAN_BUILD_TYPE}/generators"

echo "[build] Installing test dependencies with Conan (preset=${PRESET}, build_type=${CONAN_BUILD_TYPE})..."
conan install "${SOURCE_DIR}/tests" \
    -of "${CONAN_OUTPUT_DIR}" \
    -s build_type="${CONAN_BUILD_TYPE}" \
    --build=missing

echo "[build] Configuring (preset=${PRESET})..."
cmake --preset "${PRESET}" \
    -S "${SOURCE_DIR}" \
    -UGTest_DIR \
    -DCMAKE_PREFIX_PATH="${CONAN_GENERATORS_DIR}" \
    -DCMAKE_MODULE_PATH="${CONAN_GENERATORS_DIR}" \
    -DCMAKE_FIND_PACKAGE_PREFER_CONFIG=ON

echo "[build] Building..."
# Run cmake --build from source dir so it finds CMakePresets.json
(cd "${SOURCE_DIR}" && cmake --build --preset "${PRESET}")

echo "[build] Done."

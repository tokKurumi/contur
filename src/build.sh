#!/usr/bin/env bash
set -euo pipefail

# build.sh
# Builder for CMake C++ project (no external dependencies).
#
# This script automates the complete build workflow:
# 1. Validates input arguments and required files (CMakeLists.txt)
# 2. Configures CMake
# 3. Builds the project binary

print_usage() {
  cat <<'EOF'
Usage:
  build.sh <debug|release> <source_dir>

Positional args:
  <debug|release>   Build type. Only these two are supported. Case-insensitive.
  <source_dir>      Path to directory that contains CMakeLists.txt

Examples:
  sh src/build.sh debug ./src
  sh src/build.sh release ./src
EOF
}

err() { echo "[build] ERROR: $*" >&2; }
info() { echo "[build] $*"; }

if [[ ${#} -lt 2 ]]; then
  err "Not enough arguments."
  print_usage
  exit 2
fi

# Normalize build type (case-insensitive input, capitalized output)
BUILD_TYPE_RAW="$1"; shift
SRC_DIR_INPUT="$1"; shift || true

BUILD_TYPE_LC="${BUILD_TYPE_RAW,,}"
case "${BUILD_TYPE_LC}" in
  debug)   CMAKE_BUILD_TYPE="Debug" ;;
  release) CMAKE_BUILD_TYPE="Release" ;;
  *)
    err "Unsupported build type: '${BUILD_TYPE_RAW}'. Only 'debug' or 'release' are allowed."
    print_usage
    exit 3
    ;;
esac

# Resolve absolute source path
if [[ ! -d "${SRC_DIR_INPUT}" ]]; then
  err "Source directory does not exist: ${SRC_DIR_INPUT}"
  exit 4
fi
SRC_DIR="$(cd "${SRC_DIR_INPUT}" && pwd)"

# Validate required files
CMAKELISTS_TXT="${SRC_DIR}/CMakeLists.txt"
if [[ ! -f "${CMAKELISTS_TXT}" ]]; then
  err "Missing CMakeLists.txt in ${SRC_DIR}"
  exit 5
fi

# Build directory structure: build/Debug or build/Release
BUILD_DIR="${SRC_DIR}/build/${CMAKE_BUILD_TYPE}"
mkdir -p "${BUILD_DIR}"

# Configure CMake project
info "Configuring CMake (type=${CMAKE_BUILD_TYPE})..."
cmake -S "${SRC_DIR}" -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Build the project
info "Building..."
cmake --build "${BUILD_DIR}" --config "${CMAKE_BUILD_TYPE}"
info "Done. Artifacts are in: ${BUILD_DIR}"

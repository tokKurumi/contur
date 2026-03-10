#!/bin/bash
# format.sh - Format all C++ source and header files according to .clang-format

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SRC_DIR="$PROJECT_ROOT/src"

if [ ! -d "$SRC_DIR" ]; then
    echo "Error: src directory not found at $SRC_DIR"
    exit 1
fi

if ! command -v clang-format &> /dev/null; then
    echo "Error: clang-format not found. Please install clang-format."
    exit 1
fi

echo "Formatting C++ files in $SRC_DIR..."
find "$SRC_DIR" \( -name "*.cpp" -o -name "*.h" \) \
    -not -path "*/build/*" \
    -not -path "*/.git/*" | while read -r file; do
    echo "  Formatting: $file"
    clang-format -i "$file"
done

echo "✓ Formatting complete"

#!/bin/sh

# Make all *.sh files executable
find . -type f -name '*.sh' -exec chmod +x {} +

# Make all ELF binaries executable
if command -v file >/dev/null 2>&1; then
    find . -type f -exec sh -c '
        for f; do
            if file -b "$f" | grep -q "^ELF"; then
                chmod +x "$f"
            fi
        done
    ' sh {} +
else
    echo "Warning: 'file' command not found - ELF files not processed" >&2
    exit 1
fi
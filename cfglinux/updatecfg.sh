#!/bin/bash
set -euo pipefail

# Source and destination definitions
src_dir="$HOME/missionsave/cfglinux"
backup_dir="$HOME/.backupcfg"
fnames="~/.tmux.conf ~/.xinitrc ~/.Xresources ~/.bashrc ~/.SciTEUser.properties ~/.config/openbox/ ~/.config/nnn/"

# Convert tilde paths to absolute paths
expanded_files=()
for f in $fnames; do
    [[ $f == "~/"* ]] && f="${f/\~/$HOME}"
    expanded_files+=("$f")
done

# Create backup directory if needed
mkdir -p "$backup_dir"

for dest_path in "${expanded_files[@]}"; do
    # Get relative path (without $HOME prefix)
    rel_path="${dest_path#$HOME/}"
    src_path="$src_dir/$rel_path"

    # Skip if source doesn't exist
    if [[ ! -e "$src_path" ]]; then
        echo "NOT FOUND: $src_path"
        continue
    fi

    # Handle new files (no backup needed)
    if [[ ! -e "$dest_path" ]]; then
        echo "COPYING NEW: $src_path -> $dest_path"
        mkdir -p "$(dirname "$dest_path")"
        rsync -a "$src_path" "$(dirname "$dest_path")"
        continue
    fi

    # Compare files/directories
    if diff -rq "$src_path" "$dest_path" &>/dev/null; then
        echo "UNCHANGED: $dest_path"
        continue
    fi

    # Create timestamped backup path
    timestamp=$(date +%Y-%m-%d-%H-%M)
    backup_path="$backup_dir/${rel_path}.${timestamp}"

    # Backup existing file/directory
    echo "BACKING UP: $dest_path -> $backup_path"
    mkdir -p "$(dirname "$backup_path")"
    mv "$dest_path" "$backup_path"

    # Copy new version
    echo "UPDATING: $src_path -> $dest_path"
    rsync -a "$src_path" "$(dirname "$dest_path")"
done
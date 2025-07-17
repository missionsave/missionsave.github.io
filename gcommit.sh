#!/bin/sh

# Usage: ./gcommit.sh "Your commit message"

if [ -z "$1" ]; then
    echo "Error: Please provide a commit message."
    echo "Usage: $(basename "$0") \"Your commit message here\""
    exit 1
fi

# Get current branch name
branch=$(git rev-parse --abbrev-ref HEAD)

# Git commands
if ! git add .; then
    echo "Error: Failed to 'git add .'"
    exit 1
fi

if ! git commit -m "$1"; then
    echo "Error: Failed to commit changes."
    exit 1
fi

if ! git push origin "$branch"; then
    echo "Error: Failed to push to origin/$branch."
    exit 1
fi

echo "Successfully added, committed, and pushed changes to '$branch'!"

# Want to add automatic pull before push or stash handling too? I can help with that.

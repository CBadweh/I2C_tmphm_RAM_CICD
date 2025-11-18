#!/bin/bash
# Helper script to protect .metadata files from Git operations
# Run this once to prevent Git from modifying .metadata files during checkouts

echo "Protecting .metadata files from Git modifications..."

# Protect all existing .metadata files
if [ -d .metadata ]; then
    find .metadata -type f 2>/dev/null | while read file; do
        git update-index --skip-worktree "$file" 2>/dev/null
    done
    echo "✓ All .metadata files are now protected"
    echo "  Git will never modify these files, even during checkouts"
else
    echo "No .metadata directory found"
fi

echo ""
echo "To unprotect (if needed):"
echo "  find .metadata -type f | xargs git update-index --no-skip-worktree"


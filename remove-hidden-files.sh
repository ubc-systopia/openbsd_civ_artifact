#!/usr/bin/env sh

set -eu

# Remove hidden files recursively from the current directory downward.
find . -type f -name '.*' -exec rm -f -- {} +

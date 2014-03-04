#!/bin/sh

set -e

f="$1"
tmpf="_tmp.h"

[ -z "$f" ] && {
    echo "No input file specified."
    exit 1
}

grep "#" "$f" > "$tmpf"
mv "$tmpf" "$f"

exit 0

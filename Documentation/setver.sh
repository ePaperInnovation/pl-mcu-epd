#!/bin/sh

set -e

ver="$1"

sed -i s/"^\(:Revision: \)\(.*\)\$"/"\1v$ver"/ index.rst
git add index.rst

exit 0

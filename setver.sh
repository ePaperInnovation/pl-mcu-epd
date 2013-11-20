#!/bin/sh

set -e

ver="$1"

sed -i s/"\(VERSION\[\] = \"\)\(.*\)\(\";\)"/"\1v$ver\3"/ main.c
sed -i s/"^\(:Revision: \)\(.*\)\$"/"\1v$ver"/ Documentation/index.rst
git add main.c Documentation/index.rst

exit 0

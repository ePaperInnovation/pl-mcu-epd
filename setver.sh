#!/bin/sh

set -e

ver="$1"
date=`date +"%d %B %Y"`

sed -i -e s/"\(VERSION\[\] = \"\)\(.*\)\(\";\)"/"\1v$ver\3"/ main.c
sed -i -e s/"^\(:Revision: \)\(.*\)\$"/"\1v$ver"/ Documentation/index.rst
sed -i -e s/"^\(:Date: \)\(.*\)\$"/"\1$date"/ Documentation/index.rst
sed -i -e s/"\(PROJECT_NUMBER.* =\) \"\(.*\)\""/"\1 \"v$ver\""/ Doxyfile
unix2dos main.c Documentation/index.rst Doxyfile
git add main.c Documentation/index.rst Doxyfile

exit 0

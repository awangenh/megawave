#! /bin/sh
#
# builds
# * a map of the symbols and variables with ncc
# * a graph of the relevant function calls
# * a svg representation of the graph

set -x
set -e

LIBS="mw mw-x11 mw-cmdline mw-modules"
LIBC_FUNCTIONS=$(grep -v "#" devtools/functions_libc.txt | tr "\n" " ")
IGNORE_FUNCTIONS="$LIBC_FUNCTIONS _init _fini"

NCC2DOT="python devtools/gengraph.py"

for LIB in $LIBS; do
    make CC="nccgen -ncgcc -ncld -ncfabs" AR=nccar lib$LIB
    FUNCTIONS=$(nm -f posix build/lib/lib$LIB.so \
	| grep " T " | cut -d\  -f1 | tr "\n" " ")
    NCCOUT=build/lib/lib$LIB.so.nccout
    $NCC2DOT -i "$IGNORE_FUNCTIONS" $NCCOUT $FUNCTIONS > lib$LIB.dot
    dot -Tsvg lib$LIB.dot > lib$LIB.svg
done

#!/bin/sh
set -e
CC=${CC:-gcc}
CFLAGS="-Wall -Wextra -std=c99"
LDFLAGS="-lm"
case "$(uname)" in
  Darwin*)
    CC=${CC:-clang}
    ;;
  *)
    ;;
esac
mkdir -p build
$CC $CFLAGS -Iinclude -o build/u404shell src/main.c $LDFLAGS
printf 'Built executable at %s\n' build/u404shell

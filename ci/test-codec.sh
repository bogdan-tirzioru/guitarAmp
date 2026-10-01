#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "${BASH_SOURCE[0]}")/.."
build_dir=$(mktemp -d)
trap 'rm -rf "$build_dir"' EXIT
"${CC:-cc}" -std=c11 -Wall -Wextra -Werror -pedantic \
  -I firmwaredsp/Core/Inc \
  firmwaredsp/Core/Src/tlv320aic3104.c tests/test_tlv320aic3104.c \
  -o "$build_dir/test-codec"
"$build_dir/test-codec"

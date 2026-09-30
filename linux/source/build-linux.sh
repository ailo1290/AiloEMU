#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")/.." && pwd)"
g++ -std=c++17 -O2 -pipe -Wall -Wextra -Wno-unused-parameter \
  "$root/source/main_linux.cpp" -o "$root/bin/ailoemu" \
  -Wl,-rpath,'$ORIGIN/../lib' -Wl,--as-needed \
  -l:libSDL2-2.0.so.0 -ldl -pthread

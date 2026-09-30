#!/usr/bin/env bash
set -euo pipefail
SOURCE="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
OUT="$(cd -- "$SOURCE/.." && pwd)"
CC="${CC:-gcc}"
CXX="${CXX:-g++}"
WINDRES="${WINDRES:-windres}"
CMAKE="${CMAKE:-cmake}"
JOBS="${JOBS:-4}"
make -C "$SOURCE/vendor/fceumm" -f Makefile.libretro platform=win HAVE_HDPACK=0 clean
make -C "$SOURCE/vendor/fceumm" -f Makefile.libretro platform=win HAVE_HDPACK=0 \
    CC="$CC" GIT_VERSION='" 236ccdf"' -j"$JOBS"
cp "$SOURCE/vendor/fceumm/fceumm_libretro.dll" "$OUT/fceumm_libretro.dll"
make -C "$SOURCE/vendor/snes9x/libretro" platform=win clean
make -C "$SOURCE/vendor/snes9x/libretro" platform=win CC="$CC" CXX="$CXX" LTO= \
    SHARED='-shared -static -static-libgcc -static-libstdc++ -s -Wl,--version-script=link.T' -j"$JOBS"
cp "$SOURCE/vendor/snes9x/libretro/snes9x_libretro.dll" "$OUT/snes9x_libretro.dll"
"$CMAKE" -S "$SOURCE/vendor/mgba" -B "$SOURCE/build-mgba-win" \
    -DCMAKE_SYSTEM_NAME=Windows -DCMAKE_C_COMPILER="$CC" -DCMAKE_CXX_COMPILER="$CXX" \
    -DCMAKE_RC_COMPILER="$WINDRES" -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_LIBRETRO=ON -DSKIP_LIBRARY=ON -DDISABLE_FRONTENDS=ON -DDISABLE_DEPS=ON \
    -DBUILD_QT=OFF -DBUILD_SDL=OFF -DUSE_LZMA=OFF -DUSE_FFMPEG=OFF \
    -DUSE_LIBZIP=OFF -DUSE_MINIZIP=OFF -DUSE_ELF=OFF -DUSE_LUA=OFF -DUSE_EPOXY=OFF \
    -DBUILD_GL=OFF -DBUILD_GLES2=OFF -DBUILD_GLES3=OFF \
    -DCMAKE_SHARED_LINKER_FLAGS='-static-libgcc -static-libstdc++ -static'
"$CMAKE" --build "$SOURCE/build-mgba-win" --target mgba_libretro -j"$JOBS"
cp "$SOURCE/build-mgba-win/mgba_libretro.dll" "$OUT/mgba_libretro.dll"
cd "$SOURCE"
"$WINDRES" --preprocessor="$CC" --preprocessor-arg=-E --preprocessor-arg=-xc \
    --preprocessor-arg=-DRC_INVOKED -I . app.rc -O coff -o app-res.o
"$CXX" -std=c++17 -O2 -Wall -Wextra -Werror -static -static-libgcc -static-libstdc++ \
    -municode -mwindows main.cpp app-res.o -o "$OUT/AiloEMU.exe" \
    -lcomdlg32 -lgdi32 -luser32 -lshell32 -lwinmm
echo "Built frontend and base cores: $OUT/AiloEMU.exe"
echo "AiloEMU 2.0 additional core build instructions: $SOURCE/BUILD.md"

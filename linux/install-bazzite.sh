#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "$0")" && pwd)"
target="${XDG_DATA_HOME:-$HOME/.local/share}/ailoemu"
mkdir -p "$target" "$HOME/.local/bin" "$HOME/.local/share/applications" "$HOME/.local/share/icons/hicolor/scalable/apps"
cp -a "$root/bin" "$root/cores" "$root/share" "$target/"
ln -sfn "$target/bin/ailoemu" "$HOME/.local/bin/ailoemu"
cp "$root/io.github.ailoemu.AiloEMU.desktop" "$HOME/.local/share/applications/"
cp "$root/io.github.ailoemu.AiloEMU.svg" "$HOME/.local/share/icons/hicolor/scalable/apps/"
printf 'AiloEMU installato. Lo trovi nel menu Applicazioni > Giochi.\n'

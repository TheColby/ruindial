#!/usr/bin/env bash
set -euo pipefail

DRY_RUN=0
if [[ "${1:-}" == "--dry-run" ]]; then
  DRY_RUN=1
elif [[ $# -gt 0 ]]; then
  echo "Usage: scripts/uninstall.sh [--dry-run]" >&2
  exit 2
fi

TARGETS=(
  "$HOME/Library/Audio/Plug-Ins/Components/RuinDial.component"
  "$HOME/Library/Audio/Plug-Ins/VST3/RuinDial.vst3"
  "$HOME/Applications/RuinDial.app"
)

for target in "${TARGETS[@]}"; do
  if [[ -e "$target" ]]; then
    if [[ "$DRY_RUN" -eq 1 ]]; then
      echo "Would remove $target"
    else
      rm -rf "$target"
      echo "Removed $target"
    fi
  fi
done

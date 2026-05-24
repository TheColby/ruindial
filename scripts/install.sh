#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-"$ROOT_DIR/build"}"
CONFIG="${CONFIG:-Release}"
INSTALL_PLUGINS=1

usage() {
  cat <<'USAGE'
Usage: scripts/install.sh [options]

Build and install RuinDial for the current macOS user.

Options:
  --build-dir PATH     Build directory. Defaults to ./build.
  --config NAME        Build configuration. Defaults to Release.
  --juce PATH          Path to a local JUCE checkout.
  --skip-install       Build only; do not copy plugin bundles.
  -h, --help           Show this help.

Environment:
  BUILD_DIR            Build directory override.
  CONFIG               Build configuration override.
  JUCE_SOURCE_DIR      Path to a local JUCE checkout.
USAGE
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    --build-dir)
      BUILD_DIR="$2"
      shift 2
      ;;
    --config)
      CONFIG="$2"
      shift 2
      ;;
    --juce)
      JUCE_SOURCE_DIR="$2"
      shift 2
      ;;
    --skip-install)
      INSTALL_PLUGINS=0
      shift
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    *)
      echo "Unknown option: $1" >&2
      usage >&2
      exit 2
      ;;
  esac
done

if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "RuinDial install currently targets macOS AU/VST3 locations." >&2
  exit 1
fi

cmake_args=(-S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$CONFIG")
if [[ -n "${JUCE_SOURCE_DIR:-}" ]]; then
  cmake_args+=("-DJUCE_SOURCE_DIR=$JUCE_SOURCE_DIR")
fi

echo "Configuring RuinDial..."
cmake "${cmake_args[@]}"

echo "Building RuinDial ($CONFIG)..."
cmake --build "$BUILD_DIR" --config "$CONFIG"

if [[ "$INSTALL_PLUGINS" -eq 0 ]]; then
  echo "Build complete. Skipping install."
  exit 0
fi

ARTIFACTS_DIR="$BUILD_DIR/RuinDial_artefacts"
if [[ -d "$ARTIFACTS_DIR/$CONFIG" ]]; then
  ARTIFACTS_DIR="$ARTIFACTS_DIR/$CONFIG"
fi
AU_BUNDLE="$ARTIFACTS_DIR/AU/RuinDial.component"
VST3_BUNDLE="$ARTIFACTS_DIR/VST3/RuinDial.vst3"
STANDALONE_APP="$ARTIFACTS_DIR/Standalone/RuinDial.app"

for bundle in "$AU_BUNDLE" "$VST3_BUNDLE" "$STANDALONE_APP"; do
  if [[ ! -e "$bundle" ]]; then
    echo "Expected build artifact was not found: $bundle" >&2
    exit 1
  fi
done

AU_INSTALL_DIR="$HOME/Library/Audio/Plug-Ins/Components"
VST3_INSTALL_DIR="$HOME/Library/Audio/Plug-Ins/VST3"
APP_INSTALL_DIR="$HOME/Applications"

mkdir -p "$AU_INSTALL_DIR" "$VST3_INSTALL_DIR" "$APP_INSTALL_DIR"

echo "Installing Audio Unit..."
ditto "$AU_BUNDLE" "$AU_INSTALL_DIR/RuinDial.component"

echo "Installing VST3..."
ditto "$VST3_BUNDLE" "$VST3_INSTALL_DIR/RuinDial.vst3"

echo "Installing standalone app..."
ditto "$STANDALONE_APP" "$APP_INSTALL_DIR/RuinDial.app"

echo "RuinDial installed:"
echo "  $AU_INSTALL_DIR/RuinDial.component"
echo "  $VST3_INSTALL_DIR/RuinDial.vst3"
echo "  $APP_INSTALL_DIR/RuinDial.app"

#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-"$ROOT_DIR/build"}"
DIST_DIR="${DIST_DIR:-"$ROOT_DIR/dist"}"
CONFIG="${CONFIG:-Release}"
VERSION="${VERSION:-1.0.0}"
PACKAGE_NAME="RuinDial-$VERSION-macOS"
STAGE_DIR="$DIST_DIR/$PACKAGE_NAME"
ARCHIVE="$DIST_DIR/$PACKAGE_NAME.zip"

if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "Release packaging currently targets macOS." >&2
  exit 1
fi

"$ROOT_DIR/scripts/install.sh" --build-dir "$BUILD_DIR" --config "$CONFIG" --skip-install

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

mkdir -p "$DIST_DIR"
rm -rf "$STAGE_DIR"
rm -f "$ARCHIVE"
mkdir -p "$STAGE_DIR/Plugins"

ditto "$AU_BUNDLE" "$STAGE_DIR/Plugins/RuinDial.component"
ditto "$VST3_BUNDLE" "$STAGE_DIR/Plugins/RuinDial.vst3"
ditto "$STANDALONE_APP" "$STAGE_DIR/RuinDial.app"
cp "$ROOT_DIR/README.md" "$ROOT_DIR/LICENSE" "$ROOT_DIR/CHANGELOG.md" "$STAGE_DIR/"
cp "$ROOT_DIR/docs/MANUAL.md" "$STAGE_DIR/Manual.md"

if [[ -n "${CODESIGN_IDENTITY:-}" ]]; then
  echo "Signing release bundles with $CODESIGN_IDENTITY..."
  codesign --force --deep --options runtime --timestamp --sign "$CODESIGN_IDENTITY" "$STAGE_DIR/Plugins/RuinDial.component"
  codesign --force --deep --options runtime --timestamp --sign "$CODESIGN_IDENTITY" "$STAGE_DIR/Plugins/RuinDial.vst3"
  codesign --force --deep --options runtime --timestamp --sign "$CODESIGN_IDENTITY" "$STAGE_DIR/RuinDial.app"
fi

ditto -c -k --norsrc --keepParent "$STAGE_DIR" "$ARCHIVE"

if [[ -n "${NOTARY_PROFILE:-}" ]]; then
  echo "Submitting release archive for notarization..."
  xcrun notarytool submit "$ARCHIVE" --keychain-profile "$NOTARY_PROFILE" --wait
  xcrun stapler staple "$STAGE_DIR/Plugins/RuinDial.component"
  xcrun stapler staple "$STAGE_DIR/Plugins/RuinDial.vst3"
  xcrun stapler staple "$STAGE_DIR/RuinDial.app"
  rm -f "$ARCHIVE"
  ditto -c -k --norsrc --keepParent "$STAGE_DIR" "$ARCHIVE"
fi

echo "Created $ARCHIVE"

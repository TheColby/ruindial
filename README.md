# RuinDial

RuinDial is a JUCE audio-destruction plugin from UglySoundGenerator Labs. Its main `Destroy` macro moves from transparent audio through saturation, bit reduction, sample-rate damage, wobble, filtering, noise, and controlled aliasing.

The project builds Audio Unit, VST3, and standalone targets on macOS. Windows and Linux build VST3 and standalone targets.

## Features

- One primary musical destruction macro with a transparent zero position
- Tape Scab, Toy DAC, Voltage Sag, and Bitrot character models
- Input trim, dry/wet mix, output trim, smooth bypass, and soft output limiting
- Raw, 2x, and 4x quality modes with latency-compensated dry/wet mixing
- Eight factory programs, two A/B snapshots, and constrained randomization
- Resizable interface with a brushed-metal main control and output meter
- Stable parameter IDs, host state restoration, and deterministic DSP tests

## Install on macOS

```bash
scripts/install.sh
```

This builds, tests, and installs:

- `~/Library/Audio/Plug-Ins/Components/RuinDial.component`
- `~/Library/Audio/Plug-Ins/VST3/RuinDial.vst3`
- `~/Applications/RuinDial.app`

Use `scripts/uninstall.sh --dry-run` to inspect the uninstall targets, then `scripts/uninstall.sh` to remove them.

## Build

With an existing JUCE checkout:

```bash
cmake -S . -B build -DJUCE_SOURCE_DIR=/path/to/JUCE -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build --build-config Release --output-on-failure
```

Without `JUCE_SOURCE_DIR`, CMake fetches JUCE 8.0.6 automatically.

## Package a Release

```bash
VERSION=1.0.0 scripts/package.sh
```

Set `CODESIGN_IDENTITY` to sign the bundles and `NOTARY_PROFILE` to submit them through an existing `notarytool` keychain profile. Tagged GitHub releases use the same script and support the signing secrets documented in [the release guide](docs/RELEASING.md).

## Documentation

- [User manual](docs/MANUAL.md)
- [Compatibility and validation](docs/COMPATIBILITY.md)
- [Release process](docs/RELEASING.md)
- [Roadmap](ROADMAP.md)
- [Contributing](CONTRIBUTING.md)

RuinDial is available under the [MIT License](LICENSE).

# RuinDial

`RuinDial` is a JUCE Audio Unit/VST3/standalone plugin. One macro value drives saturation, bit depth, sample-rate damage, wobble, filtering, and noise.

## First milestone

- Build AU, VST3, and standalone plugin targets from one CMake project.
- Keep the DSP path readable enough to tune against the original offline Python sketch.
- Add presets once the macro curve feels good.

## Install

Build and install the Audio Unit, VST3, and standalone app for the current macOS user:

```bash
scripts/install.sh
```

The script installs to:

- `~/Library/Audio/Plug-Ins/Components/RuinDial.component`
- `~/Library/Audio/Plug-Ins/VST3/RuinDial.vst3`
- `~/Applications/RuinDial.app`

If you already have JUCE checked out:

```bash
scripts/install.sh --juce /path/to/JUCE
```

For a build-only check:

```bash
scripts/install.sh --skip-install
```

## Build

If you already have JUCE checked out:

```bash
cmake -S . -B build -DJUCE_SOURCE_DIR=/path/to/JUCE
cmake --build build
```

If you want CMake to fetch JUCE:

```bash
cmake -S . -B build
cmake --build build
```

The CMake project builds `AU`, `VST3`, and `Standalone` formats on macOS. The generated plugin bundles live under the CMake build tree at `build/RuinDial_artefacts/`.

## Offline reference

The original Python sketch remains useful for fast algorithm checks:

```bash
python3 destroyer.py input.wav out/hum_destroyed.wav --amount 0.82
```

## Why this should become a repo

Small character plugins are useful because they make fast decisions. This version is intentionally one-control, with the ugliness curve concentrated in `Source/DestroyerDsp.h` so it is easy to retune by ear.

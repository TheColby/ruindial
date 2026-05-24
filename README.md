# RuinDial

`RuinDial` is a JUCE Audio Unit/VST3/standalone plugin. One macro value drives saturation, bit depth, sample-rate damage, wobble, filtering, and noise.

## First milestone

- Build AU, VST3, and standalone plugin targets from one CMake project.
- Keep the DSP path readable enough to tune against the original offline Python sketch.
- Add presets once the macro curve feels good.

## Build

If you already have JUCE checked out:

```bash
cmake -S labs/one-knob-destroyer -B out/build/one-knob-destroyer -DJUCE_SOURCE_DIR=/path/to/JUCE
cmake --build out/build/one-knob-destroyer
```

If you want CMake to fetch JUCE:

```bash
cmake -S labs/one-knob-destroyer -B out/build/one-knob-destroyer
cmake --build out/build/one-knob-destroyer
```

When this folder is used as a standalone repo, run the same commands from the repo root:

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

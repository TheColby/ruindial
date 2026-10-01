# RuinDial Roadmap

RuinDial is a one-control damage plugin. The project should stay fast, opinionated, and musical: one big decision, a small number of tasteful supporting controls, and no maze of expert parameters.

## v0.1 - Plugin Foundation

Status: done

- JUCE CMake project
- Audio Unit, VST3, and standalone targets
- One `Destroy` macro parameter
- Saturation, bit-depth reduction, sample hold, wobble, filtering, noise, and alias blend
- Custom dark faceplate with brushed-metal knob
- macOS build/install script
- Offline Python reference processor

## v0.2 - Usability Pass

- Add input and output gain trims
- Add output soft clip or limiter to prevent surprise level jumps
- Add dry/wet mix while keeping `Destroy` as the main control
- Add A/B-safe default parameter values
- Make the numeric value display more musical, such as `Clean`, `Scuffed`, `Damaged`, `Ruined`
- Add a visible bypass state in the editor
- Verify AU/VST3 behavior in Logic, Ableton Live, REAPER, and a standalone host

## v0.3 - Presets and Character Modes

- Add a small preset bank:
  - `Tape Scab`
  - `Toy DAC`
  - `Voltage Sag`
  - `Phone Speaker`
  - `Bitrot`
  - `Motor Wobble`
- Add 3 or 4 hidden macro curves behind a mode switch
- Keep the main screen one-knob-first; avoid exposing raw DSP internals
- Add preset save/load behavior if JUCE host support is not enough

## v0.4 - Visual Polish

- Add resize-safe layout
- Add retina-friendly detail and better meter contrast
- Add a subtle output activity meter
- Add a small status strip for mode, clipping, and sample-rate context
- Replace the default standalone app chrome where practical
- Add screenshot assets for GitHub and plugin listings

## v0.5 - Release Hygiene

- Add CI build checks for macOS
- Add signed/notarized macOS release artifacts
- Add release packaging for AU, VST3, and standalone app
- Add a changelog
- Add license and contribution docs
- Tag a first alpha release

## v1.0 - Stable Character Plugin

- Stable parameter IDs
- Host automation tested
- No known denormal/performance issues
- Presets curated by ear
- Installer/package workflow documented
- Demo audio examples published
- Clear compatibility notes for macOS, AU, VST3, Intel, and Apple Silicon

## Later Ideas

- Windows VST3 support
- Linux VST3 or CLAP support
- CLAP target through a companion framework or port
- Oversampling mode for smoother heavy drive
- MIDI-triggered damage bursts
- Randomization button with undo-safe parameter changes
- Companion CLI that batch-processes files using the same DSP curve

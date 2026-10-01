# RuinDial Roadmap

RuinDial stays one-knob-first: supporting controls should make the result safer and easier to place without turning the plugin into a generic multi-effect.

## v0.1 - Foundation

Status: complete

- JUCE CMake project with AU, VST3, and standalone targets
- Initial destruction macro and custom metal-knob interface
- macOS build and user-level installation script
- Offline Python reference processor

## v0.2 - Release-Safe Core

Status: complete

- Transparent zero position and safer clean default
- Input and output trims, dry/wet mix, and smooth bypass
- Final soft limiter and visible clip indication
- Stable parameter state and smoothed automation
- Deterministic DSP tests for transparency, bounded output, and finite samples

## v0.3 - Sound and Presets

Status: complete

- Tape Scab, Toy DAC, Voltage Sag, and Bitrot character modes
- Eight curated factory programs
- Raw, 2x, and 4x quality modes
- A/B snapshots and constrained randomization

## v0.4 - Interface

Status: complete

- Resizable layout
- Output activity meter and clipping indicator
- Compact mode, quality, gain, mix, bypass, A/B, and preset controls
- Retina-safe vector drawing and a brushed-metal primary control

## v0.5 - Release Engineering

Status: complete

- macOS, Windows, and Linux CI configuration
- Automated DSP test execution
- macOS packaging with optional signing and notarization
- Tagged GitHub release workflow for macOS and Windows artifacts
- Installer, uninstaller, changelog, manual, compatibility notes, and contribution guide

## v1.0 - Validation Gate

Implementation is complete. Release validation still requires access to the named commercial hosts and Apple distribution credentials.

- Apple `auval` validation complete for version 1.0.0
- Verify state, automation, resize, bypass, and latency in Logic Pro, Ableton Live, and REAPER
- Verify the Windows VST3 artifact on physical Windows hardware
- Sign and notarize using the UglySoundGenerator Labs Apple Developer identity
- Record final demo audio and capture release screenshots
- Tag `v1.0.0` only after the compatibility checklist is signed off

## Beyond v1.0

- CLAP distribution
- MIDI-triggered damage bursts
- Batch-processing command-line companion
- User preset browser and portable preset exchange

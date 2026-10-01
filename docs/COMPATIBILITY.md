# Compatibility and Validation

## Build Targets

| Platform | AU | VST3 | Standalone | Automated build |
| --- | --- | --- | --- | --- |
| macOS | Yes | Yes | Yes | GitHub Actions and local build |
| Windows | No | Yes | Yes | GitHub Actions |
| Linux | No | Yes | Yes | GitHub Actions |

JUCE 8.0.6 and CMake 3.22 or newer are the supported build baseline. The macOS install script uses current-user plugin directories and does not require administrator access.

## Host Checklist

For every host and format, verify:

- Mono and stereo insertion
- Project save, close, reopen, and preset restoration
- Automation write/read for every parameter
- Bypass transitions and dry/wet phase alignment in all quality modes
- Editor opening, closing, and resizing at minimum and maximum dimensions
- Sample-rate changes at 44.1, 48, 88.2, 96, and 192 kHz
- Buffer sizes from 32 through 2048 samples
- Offline bounce matching real-time playback

## Current Validation

- macOS AU, VST3, and standalone targets compile locally.
- Deterministic DSP tests pass locally.
- Apple `auval` passes component properties, presets, parameter retention, automation ramps, custom UI loading, mono/stereo rendering, and sample rates through 192 kHz.
- Commercial DAW validation remains a release gate because it requires those installed hosts.
- Signing and notarization remain a release gate because they require Apple Developer credentials.

Record host test results here before tagging `v1.0.0`.

| Host | Version | Format | Architecture | Result |
| --- | --- | --- | --- | --- |
| Logic Pro | TBD | AU | TBD | Pending |
| Ableton Live | TBD | AU/VST3 | TBD | Pending |
| REAPER | TBD | AU/VST3 | TBD | Pending |

# RuinDial Manual

## Signal Flow

RuinDial applies input trim, the selected destruction character, dry/wet mixing, output trim, and a final soft limiter. The limiter is a safety stage rather than a loudness maximizer.

## Controls

`Destroy` is the primary macro. At zero, the destruction engine is transparent. Higher values progressively increase drive, quantization, sample holding, movement, filtering, noise, and aliasing.

`Input` controls how hard the destruction stages are driven. Reduce it for dynamic material or increase it when more saturation is wanted before changing the macro.

`Mix` blends latency-compensated clean and processed paths.

`Output` places the result in the mix after processing. The clip light reports activity at the safety limiter.

`Character` selects one of four macro curves:

- **Tape Scab:** warm asymmetric drive, slow movement, and darkening.
- **Toy DAC:** coarse conversion, sample holding, and a compact bandwidth.
- **Voltage Sag:** level-dependent saturation with pronounced pitch instability.
- **Bitrot:** aggressive quantization, folding, noise, and aliasing.

`Quality` selects Raw, 2x, or 4x processing. Raw retains the most digital edge. Oversampling makes the nonlinear stages smoother at the cost of CPU and a small reported latency.

`Bypass` uses a short smoothing ramp so it can be automated without a hard discontinuity.

`A` and `B` recall temporary comparison snapshots. Select a slot and press `STORE` to overwrite it. Snapshots live for the current plugin instance and are intentionally separate from host presets.

`RND` generates a constrained variation. Output trim is kept at or below unity to reduce surprise level jumps.

## Factory Programs

RuinDial includes Clean Start, Tape Scab, Toy DAC, Voltage Sag, Phone Speaker, Bitrot, Motor Wobble, and Total Ruin. Programs use the same stable parameters exposed to host automation.

## Automation

All published parameters have stable version-1 IDs. The plugin smooths gain, mix, bypass, and the primary macro. Character and quality are stepped parameters and are best changed between musical phrases.

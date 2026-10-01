# Contributing

RuinDial favors focused musical behavior over parameter count. Changes to the destruction curve should include a short rationale, before/after audio where practical, and tests for numerical safety.

## Development

```bash
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build --build-config Release --output-on-failure
```

Keep real-time processing allocation-free after `prepareToPlay`, preserve published parameter IDs, and avoid locks or filesystem access on the audio thread. New user-facing behavior should be checked in mono and stereo at multiple sample rates and buffer sizes.

Pull requests should be narrow, explain audible changes, and update the changelog when behavior changes.

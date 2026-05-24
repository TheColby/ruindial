#!/usr/bin/env python3
"""Offline one-knob audio degradation sketch."""

from __future__ import annotations

import argparse
import math
import random
import sys
from pathlib import Path

for helper_path in (Path(__file__).resolve().parent, Path(__file__).resolve().parents[1]):
    sys.path.append(str(helper_path))
from audio_common import clamp, lowpass_one_pole, normalize, read_wav_mono, write_wav_mono


def main() -> None:
    parser = argparse.ArgumentParser(description="Destroy a WAV with one macro knob.")
    parser.add_argument("input", help="input WAV")
    parser.add_argument("output", help="output WAV")
    parser.add_argument("--amount", type=float, default=0.7, help="0 clean-ish, 1 fully damaged")
    parser.add_argument("--seed", type=int, default=808, help="noise seed")
    args = parser.parse_args()

    sample_rate, samples = read_wav_mono(args.input)
    amount = clamp(args.amount, 0.0, 1.0)
    out = destroy(samples, sample_rate, amount, random.Random(args.seed))
    write_wav_mono(args.output, sample_rate, out)
    print(f"wrote: {args.output}")


def destroy(samples: list[float], sample_rate: int, amount: float, rng: random.Random) -> list[float]:
    drive = 1.0 + amount * 22.0
    crushed = [math.tanh(sample * drive) / math.tanh(drive) for sample in samples]

    bits = max(3, int(round(16 - amount * 12)))
    levels = (2 ** bits) - 1
    crushed = [round((sample + 1.0) * 0.5 * levels) / levels * 2.0 - 1.0 for sample in crushed]

    hold = max(1, int(1 + amount * amount * 28))
    held: list[float] = []
    current = 0.0
    for index, sample in enumerate(crushed):
        if index % hold == 0:
            current = sample
        wobble = math.sin(2.0 * math.pi * 0.65 * index / sample_rate) * amount * 0.08
        noise = rng.uniform(-1.0, 1.0) * amount * 0.035
        held.append(clamp(current + wobble + noise))

    dark = lowpass_one_pole(held, amount * 0.82)
    mixed = []
    for index, sample in enumerate(dark):
        alias = held[index - hold] if index >= hold else 0.0
        mixed.append(sample * (1.0 - amount * 0.25) + alias * amount * 0.25)
    return normalize(mixed)


if __name__ == "__main__":
    main()

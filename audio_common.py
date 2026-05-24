"""Small dependency-free WAV helpers for audio lab prototypes."""

from __future__ import annotations

import math
import struct
import wave
from pathlib import Path


def clamp(value: float, lower: float = -1.0, upper: float = 1.0) -> float:
    return max(lower, min(upper, value))


def read_wav_mono(path: str | Path) -> tuple[int, list[float]]:
    """Read a PCM WAV file and return sample_rate, mono samples in -1..1."""
    try:
        with wave.open(str(path), "rb") as handle:
            channels = handle.getnchannels()
            width = handle.getsampwidth()
            sample_rate = handle.getframerate()
            frame_count = handle.getnframes()
            raw = handle.readframes(frame_count)
        sample_format = "pcm"
    except wave.Error:
        sample_rate, channels, width, raw, sample_format = _read_riff_wav(path)

    if width not in (1, 2, 3, 4):
        raise ValueError(f"unsupported WAV sample width: {width} bytes")

    samples: list[float] = []
    step = width * channels
    for offset in range(0, len(raw), step):
        total = 0.0
        for channel in range(channels):
            start = offset + channel * width
            chunk = raw[start : start + width]
            total += _bytes_to_float(chunk, width, sample_format)
        samples.append(total / channels)
    return sample_rate, samples


def write_wav_mono(path: str | Path, sample_rate: int, samples: list[float]) -> None:
    """Write mono 16-bit PCM WAV."""
    Path(path).parent.mkdir(parents=True, exist_ok=True)
    peak = max((abs(sample) for sample in samples), default=1.0)
    gain = 0.98 / peak if peak > 0.98 else 1.0
    payload = bytearray()
    for sample in samples:
        value = int(clamp(sample * gain) * 32767.0)
        payload.extend(value.to_bytes(2, "little", signed=True))

    with wave.open(str(path), "wb") as handle:
        handle.setnchannels(1)
        handle.setsampwidth(2)
        handle.setframerate(sample_rate)
        handle.writeframes(bytes(payload))


def normalize(samples: list[float], peak: float = 0.98) -> list[float]:
    observed = max((abs(sample) for sample in samples), default=0.0)
    if observed == 0.0:
        return samples[:]
    gain = peak / observed
    return [sample * gain for sample in samples]


def fade_edges(samples: list[float], sample_rate: int, seconds: float = 0.01) -> list[float]:
    faded = samples[:]
    count = min(len(faded) // 2, max(1, int(sample_rate * seconds)))
    for index in range(count):
        ramp = index / count
        faded[index] *= ramp
        faded[-index - 1] *= ramp
    return faded


def lowpass_one_pole(samples: list[float], amount: float) -> list[float]:
    amount = clamp(amount, 0.0, 0.995)
    current = 0.0
    out: list[float] = []
    for sample in samples:
        current = current * amount + sample * (1.0 - amount)
        out.append(current)
    return out


def resample_linear(samples: list[float], ratio: float) -> list[float]:
    if ratio <= 0:
        raise ValueError("ratio must be positive")
    if not samples:
        return []
    out_len = max(1, int(len(samples) * ratio))
    out: list[float] = []
    for index in range(out_len):
        pos = index / ratio
        left = int(pos)
        right = min(left + 1, len(samples) - 1)
        frac = pos - left
        out.append(samples[left] * (1.0 - frac) + samples[right] * frac)
    return out


def sine_sweep(sample_rate: int, seconds: float, start_hz: float, end_hz: float) -> list[float]:
    count = max(1, int(sample_rate * seconds))
    phase = 0.0
    out: list[float] = []
    for index in range(count):
        t = index / max(1, count - 1)
        hz = start_hz * ((end_hz / start_hz) ** t)
        phase += 2.0 * math.pi * hz / sample_rate
        out.append(math.sin(phase))
    return out


def _bytes_to_float(chunk: bytes, width: int, sample_format: str) -> float:
    if sample_format == "float":
        if width != 4:
            raise ValueError("only 32-bit float WAV is supported")
        return float(struct.unpack("<f", chunk)[0])
    if width == 1:
        return (chunk[0] - 128) / 128.0
    if width == 2:
        return int.from_bytes(chunk, "little", signed=True) / 32768.0
    if width == 3:
        padded = chunk + (b"\xff" if chunk[-1] & 0x80 else b"\x00")
        return int.from_bytes(padded, "little", signed=True) / 8388608.0
    return int.from_bytes(chunk, "little", signed=True) / 2147483648.0


def _read_riff_wav(path: str | Path) -> tuple[int, int, int, bytes, str]:
    data = Path(path).read_bytes()
    if data[:4] != b"RIFF" or data[8:12] != b"WAVE":
        raise ValueError(f"not a RIFF/WAVE file: {path}")

    fmt: bytes | None = None
    audio: bytes | None = None
    offset = 12
    while offset + 8 <= len(data):
        chunk_id = data[offset : offset + 4]
        chunk_size = struct.unpack_from("<I", data, offset + 4)[0]
        chunk_start = offset + 8
        chunk_end = chunk_start + chunk_size
        chunk = data[chunk_start:chunk_end]
        if chunk_id == b"fmt ":
            fmt = chunk
        elif chunk_id == b"data":
            audio = chunk
        offset = chunk_end + (chunk_size % 2)

    if fmt is None or audio is None:
        raise ValueError(f"missing fmt or data chunk: {path}")

    format_code, channels, sample_rate, _, _, bits = struct.unpack_from("<HHIIHH", fmt, 0)
    sample_format = "pcm"
    if format_code == 3:
        sample_format = "float"
    elif format_code == 65534 and len(fmt) >= 40:
        subformat = fmt[24:40]
        if subformat.startswith(b"\x03\x00\x00\x00"):
            sample_format = "float"
        elif subformat.startswith(b"\x01\x00\x00\x00"):
            sample_format = "pcm"
        else:
            raise ValueError(f"unsupported WAVE extensible subformat in {path}")
    elif format_code != 1:
        raise ValueError(f"unsupported WAV format code {format_code} in {path}")

    return sample_rate, channels, bits // 8, audio, sample_format

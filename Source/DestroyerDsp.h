#pragma once

#include <JuceHeader.h>

enum class CharacterMode
{
    tapeScab = 0,
    toyDac,
    voltageSag,
    bitrot
};

class DestroyerDsp
{
public:
    void prepare (double newSampleRate, int maxBlockSize)
    {
        sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
        delayBuffer.assign (static_cast<size_t> (juce::jmax (maxBlockSize * 4, 4096)), 0.0f);
        reset();
    }

    void reset()
    {
        std::fill (delayBuffer.begin(), delayBuffer.end(), 0.0f);
        delayWrite = 0;
        lowpassState = 0.0f;
        envelopeState = 0.0f;
        heldSample = 0.0f;
        holdCounter = 0;
        phase = 0.0;
        rngState = 0x12345678u;
    }

    float processSample (float input, float amount, CharacterMode mode = CharacterMode::tapeScab)
    {
        amount = juce::jlimit (0.0f, 1.0f, amount);
        if (amount <= 0.00001f)
            return input;

        const auto profile = getProfile (mode);
        const auto drive = 1.0f + amount * profile.drive;
        auto damaged = saturate (input, drive, mode, amount);

        const auto bits = juce::jlimit (2, 16, static_cast<int> (std::round (16.0f - amount * profile.bitLoss)));
        const auto levels = static_cast<float> ((1 << bits) - 1);
        damaged = std::round ((juce::jlimit (-1.0f, 1.0f, damaged) + 1.0f) * 0.5f * levels)
                    / levels * 2.0f - 1.0f;

        const auto holdLength = juce::jmax (1, static_cast<int> (1.0f + amount * amount * profile.maxHold));
        if (holdCounter <= 0)
        {
            heldSample = damaged;
            holdCounter = holdLength;
        }
        --holdCounter;

        phase += juce::MathConstants<double>::twoPi * profile.wobbleHz / sampleRate;
        if (phase > juce::MathConstants<double>::twoPi)
            phase -= juce::MathConstants<double>::twoPi;

        const auto wobble = static_cast<float> (std::sin (phase)) * amount * profile.wobbleDepth;
        const auto noise = nextNoise() * amount * profile.noise;
        auto held = juce::jlimit (-1.25f, 1.25f, heldSample + wobble + noise);

        const auto lowpassAmount = juce::jlimit (0.0f, 0.96f, amount * profile.lowpass);
        lowpassState = lowpassState * lowpassAmount + held * (1.0f - lowpassAmount);

        delayBuffer[delayWrite] = held;
        const auto readIndex = (delayWrite + delayBuffer.size() - static_cast<size_t> (holdLength)) % delayBuffer.size();
        const auto alias = delayBuffer[readIndex];
        delayWrite = (delayWrite + 1) % delayBuffer.size();

        const auto crushed = lowpassState * (1.0f - amount * profile.aliasBlend)
                           + alias * amount * profile.aliasBlend;
        return juce::jmap (amount, input, crushed);
    }

    static float softLimit (float sample) noexcept
    {
        constexpr auto knee = 0.9f;
        const auto magnitude = std::abs (sample);
        if (magnitude <= knee)
            return sample;

        const auto limited = knee + (1.0f - knee) * std::tanh ((magnitude - knee) / (1.0f - knee));
        return std::copysign (limited, sample);
    }

private:
    struct Profile
    {
        float drive;
        float bitLoss;
        float maxHold;
        double wobbleHz;
        float wobbleDepth;
        float noise;
        float lowpass;
        float aliasBlend;
    };

    static Profile getProfile (CharacterMode mode) noexcept
    {
        switch (mode)
        {
            case CharacterMode::toyDac:     return { 14.0f, 11.0f, 42.0f, 0.35, 0.025f, 0.018f, 0.68f, 0.42f };
            case CharacterMode::voltageSag: return { 28.0f,  7.0f, 12.0f, 1.10, 0.095f, 0.012f, 0.88f, 0.18f };
            case CharacterMode::bitrot:     return { 18.0f, 14.0f, 64.0f, 3.70, 0.045f, 0.055f, 0.54f, 0.58f };
            case CharacterMode::tapeScab:
            default:                        return { 22.0f,  9.0f, 24.0f, 0.65, 0.060f, 0.022f, 0.82f, 0.25f };
        }
    }

    float saturate (float input, float drive, CharacterMode mode, float amount)
    {
        if (mode == CharacterMode::voltageSag)
        {
            envelopeState += (std::abs (input) - envelopeState) * 0.0015f;
            const auto sag = 1.0f / (1.0f + envelopeState * amount * 5.0f);
            return std::tanh (input * drive * sag) / std::tanh (drive);
        }

        if (mode == CharacterMode::bitrot)
        {
            const auto folded = std::sin (input * drive * (1.0f + amount * 1.8f));
            return juce::jmap (amount * 0.45f, std::tanh (input * drive), folded);
        }

        const auto asymmetry = mode == CharacterMode::toyDac ? amount * 0.08f : amount * 0.025f;
        return std::tanh ((input + asymmetry) * drive) / std::tanh (drive) - asymmetry;
    }

    float nextNoise()
    {
        rngState = rngState * 1664525u + 1013904223u;
        const auto normalized = static_cast<float> ((rngState >> 8) & 0x00ffffffu)
                              / static_cast<float> (0x00ffffffu);
        return normalized * 2.0f - 1.0f;
    }

    double sampleRate = 44100.0;
    std::vector<float> delayBuffer { 4096, 0.0f };
    size_t delayWrite = 0;
    float lowpassState = 0.0f;
    float envelopeState = 0.0f;
    float heldSample = 0.0f;
    int holdCounter = 0;
    double phase = 0.0;
    uint32_t rngState = 0x12345678u;
};

#pragma once

#include <JuceHeader.h>

class DestroyerDsp
{
public:
    void prepare (double newSampleRate, int maxBlockSize)
    {
        sampleRate = newSampleRate > 0.0 ? newSampleRate : 44100.0;
        delayBuffer.assign (static_cast<size_t> (juce::jmax (maxBlockSize * 2, 4096)), 0.0f);
        delayWrite = 0;
        lowpassState = 0.0f;
        heldSample = 0.0f;
        holdCounter = 0;
        phase = 0.0;
        rngState = 0x12345678u;
    }

    void reset()
    {
        std::fill (delayBuffer.begin(), delayBuffer.end(), 0.0f);
        delayWrite = 0;
        lowpassState = 0.0f;
        heldSample = 0.0f;
        holdCounter = 0;
        phase = 0.0;
        rngState = 0x12345678u;
    }

    float processSample (float input, float amount)
    {
        amount = juce::jlimit (0.0f, 1.0f, amount);

        const auto drive = 1.0f + amount * 22.0f;
        auto sample = std::tanh (input * drive) / std::tanh (drive);

        const auto bits = juce::jmax (3, static_cast<int> (std::round (16.0f - amount * 12.0f)));
        const auto levels = static_cast<float> ((1 << bits) - 1);
        sample = std::round ((sample + 1.0f) * 0.5f * levels) / levels * 2.0f - 1.0f;

        const auto holdLength = juce::jmax (1, static_cast<int> (1.0f + amount * amount * 28.0f));
        if (holdCounter <= 0)
        {
            heldSample = sample;
            holdCounter = holdLength;
        }
        --holdCounter;

        phase += juce::MathConstants<double>::twoPi * 0.65 / sampleRate;
        if (phase > juce::MathConstants<double>::twoPi)
            phase -= juce::MathConstants<double>::twoPi;

        const auto wobble = static_cast<float> (std::sin (phase)) * amount * 0.08f;
        const auto noise = nextNoise() * amount * 0.035f;
        auto held = juce::jlimit (-1.0f, 1.0f, heldSample + wobble + noise);

        const auto lowpassAmount = amount * 0.82f;
        lowpassState = lowpassState * lowpassAmount + held * (1.0f - lowpassAmount);

        delayBuffer[delayWrite] = held;
        const auto readIndex = (delayWrite + delayBuffer.size() - static_cast<size_t> (holdLength)) % delayBuffer.size();
        const auto alias = delayBuffer[readIndex];
        delayWrite = (delayWrite + 1) % delayBuffer.size();

        return lowpassState * (1.0f - amount * 0.25f) + alias * amount * 0.25f;
    }

private:
    float nextNoise()
    {
        rngState = rngState * 1664525u + 1013904223u;
        const auto normalized = static_cast<float> ((rngState >> 8) & 0x00ffffffu) / static_cast<float> (0x00ffffffu);
        return normalized * 2.0f - 1.0f;
    }

    double sampleRate = 44100.0;
    std::vector<float> delayBuffer;
    size_t delayWrite = 0;
    float lowpassState = 0.0f;
    float heldSample = 0.0f;
    int holdCounter = 0;
    double phase = 0.0;
    uint32_t rngState = 0x12345678u;
};

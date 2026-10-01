#include <JuceHeader.h>
#include "DestroyerDsp.h"

#include <cmath>
#include <iostream>

namespace
{
bool require (bool condition, const char* message)
{
    if (! condition)
        std::cerr << "FAIL: " << message << '\n';
    return condition;
}

float testSignal (int sample, double sampleRate)
{
    const auto time = static_cast<double> (sample) / sampleRate;
    return static_cast<float> (0.72 * std::sin (juce::MathConstants<double>::twoPi * 997.0 * time));
}
}

int main()
{
    constexpr auto sampleRate = 48000.0;
    constexpr auto sampleCount = 8192;
    auto passed = true;

    DestroyerDsp transparent;
    transparent.prepare (sampleRate, 512);
    for (int sample = 0; sample < sampleCount; ++sample)
    {
        const auto input = testSignal (sample, sampleRate);
        passed &= require (std::abs (transparent.processSample (input, 0.0f) - input) <= 1.0e-8f,
                           "zero amount must be bit-transparent");
        if (! passed)
            return 1;
    }

    for (int modeIndex = 0; modeIndex < 4; ++modeIndex)
    {
        DestroyerDsp processor;
        processor.prepare (sampleRate, 512);
        for (int sample = 0; sample < sampleCount; ++sample)
        {
            const auto output = processor.processSample (testSignal (sample, sampleRate), 1.0f,
                                                         static_cast<CharacterMode> (modeIndex));
            passed &= require (std::isfinite (output), "every mode must produce finite output");
            passed &= require (std::abs (output) <= 1.5f, "DSP output must remain bounded");
        }
    }

    DestroyerDsp first;
    DestroyerDsp second;
    first.prepare (sampleRate, 512);
    second.prepare (sampleRate, 512);
    for (int sample = 0; sample < sampleCount; ++sample)
    {
        const auto input = testSignal (sample, sampleRate);
        passed &= require (std::abs (first.processSample (input, 0.73f, CharacterMode::bitrot)
                                     - second.processSample (input, 0.73f, CharacterMode::bitrot)) <= 1.0e-8f,
                           "identical state must render deterministically");
    }

    for (float value = -8.0f; value <= 8.0f; value += 0.01f)
        passed &= require (std::abs (DestroyerDsp::softLimit (value)) <= 1.0f,
                           "soft limiter must cap output at unity");

    if (passed)
        std::cout << "RuinDial DSP tests passed\n";
    return passed ? 0 : 1;
}

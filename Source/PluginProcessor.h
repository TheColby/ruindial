#pragma once

#include <JuceHeader.h>
#include "DestroyerDsp.h"

class OneKnobDestroyerAudioProcessor final : public juce::AudioProcessor
{
public:
    OneKnobDestroyerAudioProcessor();
    ~OneKnobDestroyerAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    juce::AudioProcessorParameter* getBypassParameter() const override;

    void captureSnapshot (int slot);
    void recallSnapshot (int slot);
    void randomizeParameters();
    float getOutputLevelDb() const noexcept { return outputLevelDb.load(); }
    bool consumeClipFlag() noexcept { return clipFlag.exchange (false); }

    juce::AudioProcessorValueTreeState parameters;

    static constexpr auto amountId = "amount";
    static constexpr auto mixId = "mix";
    static constexpr auto inputGainId = "inputGain";
    static constexpr auto outputGainId = "outputGain";
    static constexpr auto modeId = "mode";
    static constexpr auto qualityId = "quality";
    static constexpr auto bypassId = "bypass";

private:
    struct Snapshot
    {
        std::array<float, 7> values {};
        bool valid = false;
    };

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void processWetBlock (juce::dsp::AudioBlock<float> block, int bankIndex, int oversamplingFactor, CharacterMode mode);
    void setParameterValue (const juce::String& parameterId, float plainValue);
    float getParameterValue (const juce::String& parameterId) const;

    std::array<std::array<DestroyerDsp, 2>, 3> channelDsp;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling2x;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling4x;
    juce::dsp::DelayLine<float> dryDelayLeft { 1024 };
    juce::dsp::DelayLine<float> dryDelayRight { 1024 };
    juce::dsp::DelayLine<float> wetDelayLeft { 1024 };
    juce::dsp::DelayLine<float> wetDelayRight { 1024 };
    juce::AudioBuffer<float> dryBuffer;
    std::vector<float> amountEnvelope;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> amountSmoother;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> inputGainSmoother;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> outputGainSmoother;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> mixSmoother;

    std::array<Snapshot, 2> snapshots;
    std::atomic<float> outputLevelDb { -100.0f };
    std::atomic<bool> clipFlag { false };
    std::atomic<int> currentProgram { 0 };
    int pluginLatency = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OneKnobDestroyerAudioProcessor)
};

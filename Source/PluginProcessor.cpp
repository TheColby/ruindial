#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
struct FactoryPreset
{
    const char* name;
    float amount;
    float mix;
    float inputGain;
    float outputGain;
    int mode;
    int quality;
};

constexpr std::array<FactoryPreset, 8> factoryPresets {{
    { "Clean Start",  0.00f, 1.00f,  0.0f,  0.0f, 0, 0 },
    { "Tape Scab",    0.48f, 0.84f,  1.5f, -1.5f, 0, 1 },
    { "Toy DAC",      0.62f, 0.78f,  0.0f, -1.0f, 1, 0 },
    { "Voltage Sag",  0.71f, 0.92f,  2.0f, -2.5f, 2, 2 },
    { "Phone Speaker",0.54f, 1.00f,  3.0f, -2.0f, 1, 0 },
    { "Bitrot",       0.82f, 0.86f, -1.0f, -3.0f, 3, 1 },
    { "Motor Wobble", 0.67f, 0.72f,  1.0f, -1.5f, 2, 1 },
    { "Total Ruin",   1.00f, 1.00f, -3.0f, -4.0f, 3, 2 }
}};

juce::String amountLabel (float value)
{
    if (value < 0.02f) return "Clean";
    if (value < 0.34f) return "Scuffed";
    if (value < 0.67f) return "Damaged";
    if (value < 0.90f) return "Ruined";
    return "Destroyed";
}
}

OneKnobDestroyerAudioProcessor::OneKnobDestroyerAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput ("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout()),
      oversampling2x (std::make_unique<juce::dsp::Oversampling<float>> (
          2, 1, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, true)),
      oversampling4x (std::make_unique<juce::dsp::Oversampling<float>> (
          2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, true))
{
}

juce::AudioProcessorValueTreeState::ParameterLayout OneKnobDestroyerAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { amountId, 1 }, "Destroy",
        juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f }, 0.0f,
        juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float value, int) { return amountLabel (value); })
            .withValueFromStringFunction ([] (const juce::String& text) { return text.getFloatValue(); })));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { mixId, 1 }, "Mix",
        juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f }, 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { inputGainId, 1 }, "Input",
        juce::NormalisableRange<float> { -24.0f, 12.0f, 0.1f }, 0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { outputGainId, 1 }, "Output",
        juce::NormalisableRange<float> { -24.0f, 6.0f, 0.1f }, 0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { modeId, 1 }, "Character",
        juce::StringArray { "Tape Scab", "Toy DAC", "Voltage Sag", "Bitrot" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { qualityId, 1 }, "Quality",
        juce::StringArray { "Raw", "2x", "4x" }, 0));
    params.push_back (std::make_unique<juce::AudioParameterBool> (
        juce::ParameterID { bypassId, 1 }, "Bypass", false));
    return { params.begin(), params.end() };
}

void OneKnobDestroyerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const std::array<int, 3> factors { 1, 2, 4 };
    for (size_t bank = 0; bank < channelDsp.size(); ++bank)
        for (auto& dsp : channelDsp[bank])
            dsp.prepare (sampleRate * factors[bank], samplesPerBlock * factors[bank]);

    oversampling2x->initProcessing (static_cast<size_t> (samplesPerBlock));
    oversampling4x->initProcessing (static_cast<size_t> (samplesPerBlock));
    oversampling2x->reset();
    oversampling4x->reset();

    dryBuffer.setSize (juce::jmax (2, getTotalNumInputChannels()), samplesPerBlock, false, false, true);
    amountEnvelope.resize (static_cast<size_t> (samplesPerBlock), 0.0f);

    juce::dsp::ProcessSpec delaySpec { sampleRate, static_cast<juce::uint32> (samplesPerBlock), 1 };
    dryDelayLeft.prepare (delaySpec);
    dryDelayRight.prepare (delaySpec);
    wetDelayLeft.prepare (delaySpec);
    wetDelayRight.prepare (delaySpec);
    dryDelayLeft.reset();
    dryDelayRight.reset();
    wetDelayLeft.reset();
    wetDelayRight.reset();

    amountSmoother.reset (sampleRate, 0.025);
    inputGainSmoother.reset (sampleRate, 0.025);
    outputGainSmoother.reset (sampleRate, 0.025);
    mixSmoother.reset (sampleRate, 0.025);
    amountSmoother.setCurrentAndTargetValue (getParameterValue (amountId));
    inputGainSmoother.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (getParameterValue (inputGainId)));
    outputGainSmoother.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (getParameterValue (outputGainId)));
    mixSmoother.setCurrentAndTargetValue (getParameterValue (mixId));

    pluginLatency = juce::roundToInt (oversampling4x->getLatencyInSamples());
    dryDelayLeft.setDelay (static_cast<float> (pluginLatency));
    dryDelayRight.setDelay (static_cast<float> (pluginLatency));
    setLatencySamples (pluginLatency);
    outputLevelDb.store (-100.0f);
    clipFlag.store (false);
}

void OneKnobDestroyerAudioProcessor::releaseResources()
{
    oversampling2x->reset();
    oversampling4x->reset();
    dryDelayLeft.reset();
    dryDelayRight.reset();
    wetDelayLeft.reset();
    wetDelayRight.reset();
}

bool OneKnobDestroyerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& input = layouts.getMainInputChannelSet();
    const auto& output = layouts.getMainOutputChannelSet();
    return input == output && (output == juce::AudioChannelSet::mono() || output == juce::AudioChannelSet::stereo());
}

void OneKnobDestroyerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const auto channels = juce::jmin (getTotalNumInputChannels(), 2);
    const auto numSamples = buffer.getNumSamples();

    for (auto channel = channels; channel < getTotalNumOutputChannels(); ++channel)
        buffer.clear (channel, 0, numSamples);

    inputGainSmoother.setTargetValue (juce::Decibels::decibelsToGain (getParameterValue (inputGainId)));
    for (auto sample = 0; sample < numSamples; ++sample)
    {
        const auto gain = inputGainSmoother.getNextValue();
        for (auto channel = 0; channel < channels; ++channel)
            buffer.getWritePointer (channel)[sample] *= gain;
    }

    for (auto channel = 0; channel < channels; ++channel)
        dryBuffer.copyFrom (channel, 0, buffer, channel, 0, numSamples);

    amountSmoother.setTargetValue (getParameterValue (amountId));
    jassert (amountEnvelope.size() >= static_cast<size_t> (numSamples));
    for (auto sample = 0; sample < numSamples; ++sample)
    {
        const auto amount = amountSmoother.getNextValue();
        if (static_cast<size_t> (sample) < amountEnvelope.size())
            amountEnvelope[static_cast<size_t> (sample)] = amount;
    }

    const auto mode = static_cast<CharacterMode> (juce::jlimit (0, 3, juce::roundToInt (getParameterValue (modeId))));
    const auto quality = juce::jlimit (0, 2, juce::roundToInt (getParameterValue (qualityId)));
    auto block = juce::dsp::AudioBlock<float> (buffer).getSubsetChannelBlock (0, static_cast<size_t> (channels));

    int latency = 0;
    if (quality == 1)
    {
        auto upsampled = oversampling2x->processSamplesUp (block);
        processWetBlock (upsampled, 1, 2, mode);
        oversampling2x->processSamplesDown (block);
        latency = juce::roundToInt (oversampling2x->getLatencyInSamples());
    }
    else if (quality == 2)
    {
        auto upsampled = oversampling4x->processSamplesUp (block);
        processWetBlock (upsampled, 2, 4, mode);
        oversampling4x->processSamplesDown (block);
        latency = juce::roundToInt (oversampling4x->getLatencyInSamples());
    }
    else
    {
        processWetBlock (block, 0, 1, mode);
    }

    const auto wetCompensation = juce::jmax (0, pluginLatency - latency);
    wetDelayLeft.setDelay (static_cast<float> (wetCompensation));
    wetDelayRight.setDelay (static_cast<float> (wetCompensation));

    const auto bypassed = getParameterValue (bypassId) > 0.5f;
    mixSmoother.setTargetValue (bypassed ? 0.0f : getParameterValue (mixId));
    outputGainSmoother.setTargetValue (juce::Decibels::decibelsToGain (getParameterValue (outputGainId)));

    auto peak = 0.0f;
    auto clipped = false;
    for (auto sample = 0; sample < numSamples; ++sample)
    {
        const auto mix = mixSmoother.getNextValue();
        const auto outputGain = outputGainSmoother.getNextValue();
        for (auto channel = 0; channel < channels; ++channel)
        {
            auto dry = dryBuffer.getSample (channel, sample);
            auto& delay = channel == 0 ? dryDelayLeft : dryDelayRight;
            delay.pushSample (0, dry);
            dry = delay.popSample (0);
            auto wet = buffer.getSample (channel, sample);
            auto& wetDelay = channel == 0 ? wetDelayLeft : wetDelayRight;
            wetDelay.pushSample (0, wet);
            wet = wetDelay.popSample (0);
            const auto preLimited = (dry + (wet - dry) * mix) * outputGain;
            clipped = clipped || std::abs (preLimited) > 1.0f;
            const auto output = DestroyerDsp::softLimit (preLimited);
            buffer.setSample (channel, sample, output);
            peak = juce::jmax (peak, std::abs (output));
        }
    }

    outputLevelDb.store (juce::Decibels::gainToDecibels (peak, -100.0f));
    if (clipped)
        clipFlag.store (true);
}

void OneKnobDestroyerAudioProcessor::processWetBlock (juce::dsp::AudioBlock<float> block,
                                                       int bankIndex,
                                                       int oversamplingFactor,
                                                       CharacterMode mode)
{
    const auto numEnvelopeSamples = amountEnvelope.size();
    for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
    {
        auto* data = block.getChannelPointer (channel);
        auto& dsp = channelDsp[static_cast<size_t> (bankIndex)][juce::jmin (channel, size_t { 1 })];
        for (size_t sample = 0; sample < block.getNumSamples(); ++sample)
        {
            const auto envelopeIndex = juce::jmin (sample / static_cast<size_t> (oversamplingFactor),
                                                   numEnvelopeSamples - 1);
            data[sample] = dsp.processSample (data[sample], amountEnvelope[envelopeIndex], mode);
        }
    }
}

int OneKnobDestroyerAudioProcessor::getNumPrograms()
{
    return static_cast<int> (factoryPresets.size());
}

int OneKnobDestroyerAudioProcessor::getCurrentProgram()
{
    return currentProgram.load();
}

void OneKnobDestroyerAudioProcessor::setCurrentProgram (int index)
{
    index = juce::jlimit (0, getNumPrograms() - 1, index);
    const auto& preset = factoryPresets[static_cast<size_t> (index)];
    setParameterValue (amountId, preset.amount);
    setParameterValue (mixId, preset.mix);
    setParameterValue (inputGainId, preset.inputGain);
    setParameterValue (outputGainId, preset.outputGain);
    setParameterValue (modeId, static_cast<float> (preset.mode));
    setParameterValue (qualityId, static_cast<float> (preset.quality));
    setParameterValue (bypassId, 0.0f);
    currentProgram.store (index);
}

const juce::String OneKnobDestroyerAudioProcessor::getProgramName (int index)
{
    if (juce::isPositiveAndBelow (index, getNumPrograms()))
        return factoryPresets[static_cast<size_t> (index)].name;
    return {};
}

void OneKnobDestroyerAudioProcessor::captureSnapshot (int slot)
{
    if (! juce::isPositiveAndBelow (slot, 2))
        return;

    constexpr std::array<const char*, 7> ids {
        amountId, mixId, inputGainId, outputGainId, modeId, qualityId, bypassId
    };
    auto& snapshot = snapshots[static_cast<size_t> (slot)];
    for (size_t index = 0; index < ids.size(); ++index)
        snapshot.values[index] = getParameterValue (ids[index]);
    snapshot.valid = true;
}

void OneKnobDestroyerAudioProcessor::recallSnapshot (int slot)
{
    if (! juce::isPositiveAndBelow (slot, 2))
        return;

    constexpr std::array<const char*, 7> ids {
        amountId, mixId, inputGainId, outputGainId, modeId, qualityId, bypassId
    };
    const auto& snapshot = snapshots[static_cast<size_t> (slot)];
    if (! snapshot.valid)
        return;
    for (size_t index = 0; index < ids.size(); ++index)
        setParameterValue (ids[index], snapshot.values[index]);
}

void OneKnobDestroyerAudioProcessor::randomizeParameters()
{
    auto& random = juce::Random::getSystemRandom();
    setParameterValue (amountId, 0.25f + random.nextFloat() * 0.75f);
    setParameterValue (mixId, 0.55f + random.nextFloat() * 0.45f);
    setParameterValue (inputGainId, -3.0f + random.nextFloat() * 7.0f);
    setParameterValue (outputGainId, -4.0f + random.nextFloat() * 4.0f);
    setParameterValue (modeId, static_cast<float> (random.nextInt (4)));
}

void OneKnobDestroyerAudioProcessor::setParameterValue (const juce::String& parameterId, float plainValue)
{
    if (auto* parameter = parameters.getParameter (parameterId))
        parameter->setValueNotifyingHost (parameter->convertTo0to1 (plainValue));
}

float OneKnobDestroyerAudioProcessor::getParameterValue (const juce::String& parameterId) const
{
    if (const auto* value = parameters.getRawParameterValue (parameterId))
        return value->load();
    return 0.0f;
}

juce::AudioProcessorParameter* OneKnobDestroyerAudioProcessor::getBypassParameter() const
{
    return parameters.getParameter (bypassId);
}

void OneKnobDestroyerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    state.setProperty ("program", currentProgram.load(), nullptr);
    if (state.isValid())
    {
        std::unique_ptr<juce::XmlElement> xml (state.createXml());
        copyXmlToBinary (*xml, destData);
    }
}

void OneKnobDestroyerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (parameters.state.getType()))
    {
        const auto restored = juce::ValueTree::fromXml (*xml);
        auto migrated = parameters.copyState();

        for (int index = 0; index < restored.getNumChildren(); ++index)
        {
            const auto source = restored.getChild (index);
            const auto parameterId = source.getProperty ("id");
            auto destination = migrated.getChildWithProperty ("id", parameterId);
            if (destination.isValid() && source.hasProperty ("value"))
                destination.setProperty ("value", source.getProperty ("value"), nullptr);
        }

        const auto program = static_cast<int> (restored.getProperty ("program", -1));
        currentProgram.store (program);
        migrated.setProperty ("program", program, nullptr);
        parameters.replaceState (migrated);
    }
}

juce::AudioProcessorEditor* OneKnobDestroyerAudioProcessor::createEditor()
{
    return new OneKnobDestroyerAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OneKnobDestroyerAudioProcessor();
}

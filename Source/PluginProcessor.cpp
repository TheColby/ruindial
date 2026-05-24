#include "PluginProcessor.h"
#include "PluginEditor.h"

OneKnobDestroyerAudioProcessor::OneKnobDestroyerAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput ("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout OneKnobDestroyerAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "amount", 1 },
        "Destroy",
        juce::NormalisableRange<float> { 0.0f, 1.0f, 0.001f },
        0.7f));
    return { params.begin(), params.end() };
}

void OneKnobDestroyerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    for (auto& dsp : channelDsp)
        dsp.prepare (sampleRate, samplesPerBlock);

    amountSmoother.reset (sampleRate, 0.025);
    amountSmoother.setCurrentAndTargetValue (*parameters.getRawParameterValue ("amount"));
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

    const auto totalInputChannels = getTotalNumInputChannels();
    const auto totalOutputChannels = getTotalNumOutputChannels();
    for (auto channel = totalInputChannels; channel < totalOutputChannels; ++channel)
        buffer.clear (channel, 0, buffer.getNumSamples());

    amountSmoother.setTargetValue (*parameters.getRawParameterValue ("amount"));

    for (auto sample = 0; sample < buffer.getNumSamples(); ++sample)
    {
        const auto amount = amountSmoother.getNextValue();
        for (auto channel = 0; channel < totalInputChannels; ++channel)
        {
            auto* data = buffer.getWritePointer (channel);
            data[sample] = channelDsp[static_cast<size_t> (juce::jmin (channel, 1))].processSample (data[sample], amount);
        }
    }
}

void OneKnobDestroyerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = parameters.copyState(); state.isValid())
    {
        std::unique_ptr<juce::XmlElement> xml (state.createXml());
        copyXmlToBinary (*xml, destData);
    }
}

void OneKnobDestroyerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (parameters.state.getType()))
        parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* OneKnobDestroyerAudioProcessor::createEditor()
{
    return new OneKnobDestroyerAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OneKnobDestroyerAudioProcessor();
}

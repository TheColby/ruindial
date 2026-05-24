#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class OneKnobDestroyerAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit OneKnobDestroyerAudioProcessorEditor (OneKnobDestroyerAudioProcessor&);
    ~OneKnobDestroyerAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    OneKnobDestroyerAudioProcessor& audioProcessor;
    juce::Slider amountSlider;
    juce::Label titleLabel;
    juce::Label amountLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> amountAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OneKnobDestroyerAudioProcessorEditor)
};

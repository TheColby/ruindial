#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class OneKnobDestroyerAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                                    private juce::Timer
{
public:
    explicit OneKnobDestroyerAudioProcessorEditor (OneKnobDestroyerAudioProcessor&);
    ~OneKnobDestroyerAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    class RuinDialLookAndFeel;
    void timerCallback() override;
    void configureSmallKnob (juce::Slider& slider, juce::Label& label, const juce::String& text);
    void selectSnapshot (int slot);

    OneKnobDestroyerAudioProcessor& audioProcessor;
    std::unique_ptr<RuinDialLookAndFeel> lookAndFeel;

    juce::Slider amountSlider;
    juce::Slider inputSlider;
    juce::Slider mixSlider;
    juce::Slider outputSlider;
    juce::ComboBox presetBox;
    juce::ComboBox modeBox;
    juce::ComboBox qualityBox;
    juce::ToggleButton bypassButton { "BYPASS" };
    juce::TextButton snapshotAButton { "A" };
    juce::TextButton snapshotBButton { "B" };
    juce::TextButton storeButton { "STORE" };
    juce::TextButton randomizeButton { "RND" };

    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::Label amountLabel;
    juce::Label inputLabel;
    juce::Label mixLabel;
    juce::Label outputLabel;
    juce::Label modeLabel;
    juce::Label qualityLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SliderAttachment> amountAttachment;
    std::unique_ptr<SliderAttachment> inputAttachment;
    std::unique_ptr<SliderAttachment> mixAttachment;
    std::unique_ptr<SliderAttachment> outputAttachment;
    std::unique_ptr<ComboAttachment> modeAttachment;
    std::unique_ptr<ComboAttachment> qualityAttachment;
    std::unique_ptr<ButtonAttachment> bypassAttachment;

    float displayedLevelDb = -100.0f;
    int clipHoldFrames = 0;
    int selectedSnapshot = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OneKnobDestroyerAudioProcessorEditor)
};

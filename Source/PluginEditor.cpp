#include "PluginEditor.h"

OneKnobDestroyerAudioProcessorEditor::OneKnobDestroyerAudioProcessorEditor (OneKnobDestroyerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    titleLabel.setText ("One Knob Destroyer", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setFont (juce::FontOptions (24.0f, juce::Font::bold));
    addAndMakeVisible (titleLabel);

    amountSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    amountSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 84, 24);
    amountSlider.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xfff15b2a));
    amountSlider.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff2a2a2a));
    amountSlider.setColour (juce::Slider::thumbColourId, juce::Colour (0xffffd166));
    amountSlider.setTextValueSuffix (" destroy");
    addAndMakeVisible (amountSlider);

    amountLabel.setText ("Destroy", juce::dontSendNotification);
    amountLabel.setJustificationType (juce::Justification::centred);
    amountLabel.setColour (juce::Label::textColourId, juce::Colour (0xffeeeeee));
    addAndMakeVisible (amountLabel);

    amountAttachment = std::make_unique<SliderAttachment> (audioProcessor.parameters, "amount", amountSlider);

    setSize (360, 280);
}

void OneKnobDestroyerAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff151515));

    auto bounds = getLocalBounds().toFloat().reduced (18.0f);
    g.setColour (juce::Colour (0xff242424));
    g.fillRoundedRectangle (bounds, 8.0f);

    g.setColour (juce::Colour (0xff3a3a3a));
    g.drawRoundedRectangle (bounds, 8.0f, 1.0f);

    g.setColour (juce::Colour (0xfff15b2a).withAlpha (0.16f));
    g.fillEllipse (static_cast<float> (getWidth()) * 0.5f - 92.0f, 86.0f, 184.0f, 184.0f);
}

void OneKnobDestroyerAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (24);
    titleLabel.setBounds (bounds.removeFromTop (42));
    bounds.removeFromTop (8);
    amountSlider.setBounds (bounds.removeFromTop (168).withSizeKeepingCentre (168, 168));
    amountLabel.setBounds (bounds.removeFromTop (28));
}

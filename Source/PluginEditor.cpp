#include "PluginEditor.h"

namespace
{
juce::Colour panelTop() { return juce::Colour (0xff25272a); }
juce::Colour panelBottom() { return juce::Colour (0xff111214); }
juce::Colour accent() { return juce::Colour (0xffff7a32); }
juce::Colour amber() { return juce::Colour (0xffffc65a); }
juce::Colour meterColour() { return juce::Colour (0xff52d3b1); }
}

class OneKnobDestroyerAudioProcessorEditor::RuinDialLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    RuinDialLookAndFeel()
    {
        setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xfff4efe6));
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xff151618));
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0xff44474b));
        setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff17181b));
        setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff45484c));
        setColour (juce::ComboBox::textColourId, juce::Colour (0xfff1ede7));
        setColour (juce::ComboBox::arrowColourId, amber());
        setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff1b1c1f));
        setColour (juce::PopupMenu::textColourId, juce::Colour (0xfff1ede7));
        setColour (juce::TextButton::buttonColourId, juce::Colour (0xff24262a));
        setColour (juce::TextButton::buttonOnColourId, accent().darker (0.25f));
        setColour (juce::TextButton::textColourOffId, juce::Colour (0xffc9c5be));
        setColour (juce::TextButton::textColourOnId, juce::Colours::white);
        setColour (juce::ToggleButton::textColourId, juce::Colour (0xffc9c5be));
        setColour (juce::ToggleButton::tickColourId, accent());
        setColour (juce::ToggleButton::tickDisabledColourId, juce::Colour (0xff56595d));
    }

    void drawRotarySlider (juce::Graphics& g,
                           int x,
                           int y,
                           int width,
                           int height,
                           float sliderPos,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider&) override
    {
        const auto bounds = juce::Rectangle<float> (static_cast<float> (x), static_cast<float> (y),
                                                     static_cast<float> (width), static_cast<float> (height)).reduced (7.0f);
        const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto centre = bounds.getCentre();
        const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        const auto isPrimary = radius > 65.0f;

        if (isPrimary)
        {
            juce::ColourGradient glow (accent().withAlpha (0.22f), centre.x, centre.y - radius,
                                       juce::Colour (0x00000000), centre.x, centre.y + radius * 1.25f, false);
            g.setGradientFill (glow);
            g.fillEllipse (centre.x - radius * 1.12f, centre.y - radius * 1.12f,
                           radius * 2.24f, radius * 2.24f);
        }

        const auto trackRadius = radius * (isPrimary ? 1.02f : 0.94f);
        juce::Path base;
        base.addCentredArc (centre.x, centre.y, trackRadius, trackRadius, 0.0f,
                            rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (juce::Colour (0xff3b3e42));
        g.strokePath (base, juce::PathStrokeType (isPrimary ? 7.0f : 4.0f,
                                                  juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

        juce::Path active;
        active.addCentredArc (centre.x, centre.y, trackRadius, trackRadius, 0.0f,
                              rotaryStartAngle, angle, true);
        juce::ColourGradient arcGradient (amber(), centre.x - radius, centre.y,
                                          accent(), centre.x + radius, centre.y, false);
        g.setGradientFill (arcGradient);
        g.strokePath (active, juce::PathStrokeType (isPrimary ? 7.0f : 4.0f,
                                                    juce::PathStrokeType::curved,
                                                    juce::PathStrokeType::rounded));

        if (isPrimary)
        {
            for (int tick = 0; tick <= 12; ++tick)
            {
                const auto tickAngle = rotaryStartAngle + (static_cast<float> (tick) / 12.0f)
                                                          * (rotaryEndAngle - rotaryStartAngle);
                const auto inner = centre.getPointOnCircumference (radius * 1.14f, tickAngle);
                const auto outer = centre.getPointOnCircumference (radius * 1.23f, tickAngle);
                g.setColour (juce::Colour (0xffd2d2d2).withAlpha (tick % 3 == 0 ? 0.70f : 0.30f));
                g.drawLine ({ inner, outer }, tick % 3 == 0 ? 1.6f : 1.0f);
            }
        }

        auto knobBounds = juce::Rectangle<float> (centre.x - radius, centre.y - radius,
                                                   radius * 2.0f, radius * 2.0f)
                              .reduced (isPrimary ? 11.0f : 8.0f);
        const auto knobRadius = knobBounds.getWidth() * 0.5f;
        const auto knobCentre = knobBounds.getCentre();
        juce::ColourGradient body (juce::Colour (0xfff8f8f8), knobCentre.x - knobRadius * 0.45f,
                                   knobCentre.y - knobRadius,
                                   juce::Colour (0xff555a5f), knobCentre.x + knobRadius * 0.65f,
                                   knobCentre.y + knobRadius, false);
        body.addColour (0.34, juce::Colour (0xffc9ccd0));
        body.addColour (0.52, juce::Colour (0xff777c82));
        body.addColour (0.73, juce::Colour (0xffeeeeee));
        g.setGradientFill (body);
        g.fillEllipse (knobBounds);

        const auto lineCount = isPrimary ? 44 : 18;
        for (int line = -lineCount / 2; line <= lineCount / 2; ++line)
        {
            const auto yPos = knobCentre.y + static_cast<float> (line) * knobRadius
                                           / (static_cast<float> (lineCount) * 0.5f);
            const auto halfWidth = std::sqrt (juce::jmax (0.0f, knobRadius * knobRadius
                                                               - std::pow (yPos - knobCentre.y, 2.0f)));
            g.setColour (juce::Colours::white.withAlpha (line % 2 == 0 ? 0.11f : 0.04f));
            g.drawHorizontalLine (juce::roundToInt (yPos), knobCentre.x - halfWidth * 0.87f,
                                                           knobCentre.x + halfWidth * 0.87f);
        }

        g.setColour (juce::Colour (0xff08090a).withAlpha (0.42f));
        g.drawEllipse (knobBounds.reduced (1.0f), 2.0f);
        g.setColour (juce::Colours::white.withAlpha (0.24f));
        g.drawEllipse (knobBounds.reduced (3.0f), 1.2f);

        const auto pointerStart = knobCentre.getPointOnCircumference (knobRadius * 0.22f, angle);
        const auto pointerEnd = knobCentre.getPointOnCircumference (knobRadius * 0.72f, angle);
        g.setColour (juce::Colour (0xff151719).withAlpha (0.72f));
        g.drawLine ({ pointerStart, pointerEnd }, isPrimary ? 5.0f : 3.0f);
        g.setColour (amber());
        g.fillEllipse (pointerEnd.x - (isPrimary ? 5.5f : 3.5f), pointerEnd.y - (isPrimary ? 5.5f : 3.5f),
                       isPrimary ? 11.0f : 7.0f, isPrimary ? 11.0f : 7.0f);
    }
};

OneKnobDestroyerAudioProcessorEditor::OneKnobDestroyerAudioProcessorEditor (OneKnobDestroyerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), lookAndFeel (std::make_unique<RuinDialLookAndFeel>())
{
    setLookAndFeel (lookAndFeel.get());
    setResizable (true, true);
    setResizeLimits (520, 380, 900, 650);

    titleLabel.setText ("RuinDial", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centredLeft);
    titleLabel.setFont (juce::FontOptions (30.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colour (0xfff8f2e8));
    addAndMakeVisible (titleLabel);

    subtitleLabel.setText ("SATURATION  /  BIT-CRUSH  /  WOBBLE  /  GRIT", juce::dontSendNotification);
    subtitleLabel.setJustificationType (juce::Justification::centredLeft);
    subtitleLabel.setFont (juce::FontOptions (10.5f));
    subtitleLabel.setColour (juce::Label::textColourId, juce::Colour (0xffa9a092));
    addAndMakeVisible (subtitleLabel);

    amountSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    amountSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 110, 25);
    amountSlider.setDoubleClickReturnValue (true, 0.0);
    amountSlider.setTooltip ("Destroy amount");
    addAndMakeVisible (amountSlider);

    amountLabel.setText ("DESTROY", juce::dontSendNotification);
    amountLabel.setJustificationType (juce::Justification::centred);
    amountLabel.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    amountLabel.setColour (juce::Label::textColourId, juce::Colour (0xfff2d6bd));
    addAndMakeVisible (amountLabel);

    configureSmallKnob (inputSlider, inputLabel, "INPUT");
    configureSmallKnob (mixSlider, mixLabel, "MIX");
    configureSmallKnob (outputSlider, outputLabel, "OUTPUT");
    inputSlider.setTextValueSuffix (" dB");
    outputSlider.setTextValueSuffix (" dB");
    for (int index = 0; index < audioProcessor.getNumPrograms(); ++index)
        presetBox.addItem (audioProcessor.getProgramName (index), index + 1);
    presetBox.setTextWhenNothingSelected ("Factory presets");
    presetBox.setTooltip ("Factory preset");
    presetBox.onChange = [this] {
        audioProcessor.setCurrentProgram (presetBox.getSelectedItemIndex());
    };
    addAndMakeVisible (presetBox);

    modeBox.addItemList ({ "Tape Scab", "Toy DAC", "Voltage Sag", "Bitrot" }, 1);
    qualityBox.addItemList ({ "Raw", "2x", "4x" }, 1);
    addAndMakeVisible (modeBox);
    addAndMakeVisible (qualityBox);

    modeLabel.setText ("CHARACTER", juce::dontSendNotification);
    qualityLabel.setText ("QUALITY", juce::dontSendNotification);
    for (auto* label : { &modeLabel, &qualityLabel })
    {
        label->setFont (juce::FontOptions (10.0f, juce::Font::bold));
        label->setColour (juce::Label::textColourId, juce::Colour (0xffa9a092));
        addAndMakeVisible (*label);
    }

    for (auto* button : { &snapshotAButton, &snapshotBButton, &storeButton, &randomizeButton })
        addAndMakeVisible (*button);
    addAndMakeVisible (bypassButton);
    bypassButton.setTooltip ("Smooth host-automatable bypass");
    storeButton.setTooltip ("Store the current settings in the selected A/B slot");
    randomizeButton.setTooltip ("Generate a level-conscious variation");

    snapshotAButton.onClick = [this] { selectSnapshot (0); };
    snapshotBButton.onClick = [this] { selectSnapshot (1); };
    storeButton.onClick = [this] { audioProcessor.captureSnapshot (selectedSnapshot); };
    randomizeButton.onClick = [this] { audioProcessor.randomizeParameters(); };
    audioProcessor.captureSnapshot (0);
    audioProcessor.captureSnapshot (1);
    snapshotAButton.setToggleState (true, juce::dontSendNotification);

    amountAttachment = std::make_unique<SliderAttachment> (audioProcessor.parameters,
                                                            OneKnobDestroyerAudioProcessor::amountId,
                                                            amountSlider);
    inputAttachment = std::make_unique<SliderAttachment> (audioProcessor.parameters,
                                                           OneKnobDestroyerAudioProcessor::inputGainId,
                                                           inputSlider);
    mixAttachment = std::make_unique<SliderAttachment> (audioProcessor.parameters,
                                                         OneKnobDestroyerAudioProcessor::mixId,
                                                         mixSlider);
    outputAttachment = std::make_unique<SliderAttachment> (audioProcessor.parameters,
                                                            OneKnobDestroyerAudioProcessor::outputGainId,
                                                            outputSlider);
    modeAttachment = std::make_unique<ComboAttachment> (audioProcessor.parameters,
                                                         OneKnobDestroyerAudioProcessor::modeId,
                                                         modeBox);
    qualityAttachment = std::make_unique<ComboAttachment> (audioProcessor.parameters,
                                                            OneKnobDestroyerAudioProcessor::qualityId,
                                                            qualityBox);
    bypassAttachment = std::make_unique<ButtonAttachment> (audioProcessor.parameters,
                                                            OneKnobDestroyerAudioProcessor::bypassId,
                                                            bypassButton);
    mixSlider.textFromValueFunction = [] (double value) {
        return juce::String (juce::roundToInt (value * 100.0)) + "%";
    };
    mixSlider.valueFromTextFunction = [] (const juce::String& text) {
        return text.getDoubleValue() / 100.0;
    };

    setSize (620, 440);
    startTimerHz (30);
}

OneKnobDestroyerAudioProcessorEditor::~OneKnobDestroyerAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void OneKnobDestroyerAudioProcessorEditor::configureSmallKnob (juce::Slider& slider,
                                                               juce::Label& label,
                                                               const juce::String& text)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 20);
    slider.setDoubleClickReturnValue (true, text == "MIX" ? 1.0 : 0.0);
    addAndMakeVisible (slider);
    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    label.setColour (juce::Label::textColourId, juce::Colour (0xffbbb5ac));
    addAndMakeVisible (label);
}

void OneKnobDestroyerAudioProcessorEditor::selectSnapshot (int slot)
{
    selectedSnapshot = juce::jlimit (0, 1, slot);
    audioProcessor.recallSnapshot (selectedSnapshot);
    snapshotAButton.setToggleState (selectedSnapshot == 0, juce::dontSendNotification);
    snapshotBButton.setToggleState (selectedSnapshot == 1, juce::dontSendNotification);
}

void OneKnobDestroyerAudioProcessorEditor::timerCallback()
{
    const auto target = audioProcessor.getOutputLevelDb();
    displayedLevelDb = target > displayedLevelDb ? target : displayedLevelDb - 1.4f;
    displayedLevelDb = juce::jmax (-60.0f, displayedLevelDb);
    if (audioProcessor.consumeClipFlag())
        clipHoldFrames = 24;
    else if (clipHoldFrames > 0)
        --clipHoldFrames;
    repaint();
}

void OneKnobDestroyerAudioProcessorEditor::paint (juce::Graphics& g)
{
    juce::ColourGradient background (panelTop(), 0.0f, 0.0f, panelBottom(),
                                     0.0f, static_cast<float> (getHeight()), false);
    background.addColour (0.58, juce::Colour (0xff191a1d));
    g.setGradientFill (background);
    g.fillAll();

    g.setColour (juce::Colours::white.withAlpha (0.026f));
    for (int y = 0; y < getHeight(); y += 3)
        g.drawHorizontalLine (y, 0.0f, static_cast<float> (getWidth()));

    auto panel = getLocalBounds().toFloat().reduced (14.0f);
    juce::ColourGradient panelGradient (juce::Colour (0xff2b2d31), panel.getX(), panel.getY(),
                                        juce::Colour (0xff141518), panel.getX(), panel.getBottom(), false);
    g.setGradientFill (panelGradient);
    g.fillRoundedRectangle (panel, 8.0f);
    g.setColour (juce::Colour (0xff4b4e53));
    g.drawRoundedRectangle (panel, 8.0f, 1.0f);
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.drawRoundedRectangle (panel.reduced (2.0f), 6.0f, 1.0f);

    const auto ruleY = 75.0f;
    juce::ColourGradient rule (juce::Colour (0x00ff7a32), 28.0f, ruleY,
                               accent().withAlpha (0.78f), static_cast<float> (getWidth()) * 0.5f, ruleY, false);
    rule.addColour (1.0, juce::Colour (0x00ff7a32));
    g.setGradientFill (rule);
    g.fillRect (28.0f, ruleY, static_cast<float> (getWidth() - 56), 2.0f);

    auto meter = juce::Rectangle<float> (static_cast<float> (getWidth() - 29), 91.0f,
                                         5.0f, static_cast<float> (getHeight() - 125));
    g.setColour (juce::Colour (0xff090a0b));
    g.fillRoundedRectangle (meter.expanded (2.0f), 2.0f);
    const auto normalized = juce::jlimit (0.0f, 1.0f, (displayedLevelDb + 60.0f) / 60.0f);
    auto active = meter.withTop (meter.getBottom() - meter.getHeight() * normalized);
    juce::ColourGradient meterGradient (meterColour(), active.getX(), active.getBottom(),
                                        amber(), active.getX(), active.getY(), false);
    g.setGradientFill (meterGradient);
    g.fillRoundedRectangle (active, 1.5f);
    g.setColour (clipHoldFrames > 0 ? accent() : juce::Colour (0xff45484c));
    g.fillEllipse (meter.getX() - 1.0f, meter.getY() - 11.0f, 7.0f, 7.0f);
}

void OneKnobDestroyerAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (28);
    bounds.removeFromRight (18);

    auto header = bounds.removeFromTop (48);
    titleLabel.setBounds (header.removeFromLeft (juce::jmin (210, header.getWidth() / 2)));
    subtitleLabel.setBounds (header);
    bounds.removeFromTop (16);

    auto footer = bounds.removeFromBottom (112);
    bounds.removeFromBottom (4);

    const auto mainWidth = juce::roundToInt (static_cast<float> (bounds.getWidth()) * 0.62f);
    auto main = bounds.removeFromLeft (mainWidth);
    auto controls = bounds.reduced (8, 2);

    auto amountArea = main;
    amountLabel.setBounds (amountArea.removeFromBottom (18));
    const auto amountSize = juce::jmin (amountArea.getWidth(), amountArea.getHeight());
    amountSlider.setBounds (amountArea.withSizeKeepingCentre (amountSize, amountSize));

    presetBox.setBounds (controls.removeFromTop (30));
    controls.removeFromTop (12);
    modeLabel.setBounds (controls.removeFromTop (16));
    modeBox.setBounds (controls.removeFromTop (30));
    controls.removeFromTop (8);
    qualityLabel.setBounds (controls.removeFromTop (16));
    qualityBox.setBounds (controls.removeFromTop (30));
    controls.removeFromTop (12);

    auto buttonRow = controls.removeFromTop (26);
    const auto compactWidth = juce::jmax (28, (buttonRow.getWidth() - 9) / 4);
    snapshotAButton.setBounds (buttonRow.removeFromLeft (compactWidth));
    buttonRow.removeFromLeft (3);
    snapshotBButton.setBounds (buttonRow.removeFromLeft (compactWidth));
    buttonRow.removeFromLeft (3);
    storeButton.setBounds (buttonRow.removeFromLeft (compactWidth));
    buttonRow.removeFromLeft (3);
    randomizeButton.setBounds (buttonRow);
    controls.removeFromTop (8);
    bypassButton.setBounds (controls.removeFromTop (24));

    const auto knobWidth = footer.getWidth() / 3;
    auto layoutKnob = [&footer, knobWidth] (juce::Slider& slider, juce::Label& label) {
        auto area = footer.removeFromLeft (knobWidth);
        label.setBounds (area.removeFromTop (17));
        slider.setBounds (area);
    };
    layoutKnob (inputSlider, inputLabel);
    layoutKnob (mixSlider, mixLabel);
    layoutKnob (outputSlider, outputLabel);
}

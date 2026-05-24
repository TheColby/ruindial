#include "PluginEditor.h"

namespace
{
juce::Colour panelTop() { return juce::Colour (0xff202124); }
juce::Colour panelBottom() { return juce::Colour (0xff111214); }
juce::Colour accent() { return juce::Colour (0xffff7a32); }
juce::Colour amber() { return juce::Colour (0xffffc65a); }
}

class OneKnobDestroyerAudioProcessorEditor::RuinDialLookAndFeel final : public juce::LookAndFeel_V4
{
public:
    RuinDialLookAndFeel()
    {
        setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xfff4efe6));
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xff171717));
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0xff494949));
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
                                                   static_cast<float> (width), static_cast<float> (height))
                              .reduced (8.0f);
        const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto centre = bounds.getCentre();
        const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        drawOuterGlow (g, centre, radius);
        drawTrack (g, centre, radius, sliderPos, rotaryStartAngle, rotaryEndAngle);
        drawMetalBody (g, centre, radius, angle);
        drawPointer (g, centre, radius, angle);
    }

private:
    static void drawOuterGlow (juce::Graphics& g, juce::Point<float> centre, float radius)
    {
        juce::ColourGradient glow (accent().withAlpha (0.24f), centre.x, centre.y - radius,
                                   juce::Colour (0x00000000), centre.x, centre.y + radius * 1.25f, false);
        g.setGradientFill (glow);
        g.fillEllipse (centre.x - radius * 1.13f, centre.y - radius * 1.13f, radius * 2.26f, radius * 2.26f);
    }

    static void drawTrack (juce::Graphics& g,
                           juce::Point<float> centre,
                           float radius,
                           float sliderPos,
                           float startAngle,
                           float endAngle)
    {
        const auto trackRadius = radius * 1.04f;
        juce::Path base;
        base.addCentredArc (centre.x, centre.y, trackRadius, trackRadius, 0.0f, startAngle, endAngle, true);
        g.setColour (juce::Colour (0xff34373a));
        g.strokePath (base, juce::PathStrokeType (7.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path active;
        active.addCentredArc (centre.x, centre.y, trackRadius, trackRadius, 0.0f,
                              startAngle, startAngle + sliderPos * (endAngle - startAngle), true);
        juce::ColourGradient arcGradient (amber(), centre.x - radius, centre.y,
                                          accent(), centre.x + radius, centre.y, false);
        g.setGradientFill (arcGradient);
        g.strokePath (active, juce::PathStrokeType (7.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        for (int tick = 0; tick <= 12; ++tick)
        {
            const auto alpha = tick % 3 == 0 ? 0.70f : 0.34f;
            const auto tickAngle = startAngle + (static_cast<float> (tick) / 12.0f) * (endAngle - startAngle);
            const auto inner = juce::Point<float> (centre.x + std::cos (tickAngle) * radius * 1.16f,
                                                  centre.y + std::sin (tickAngle) * radius * 1.16f);
            const auto outer = juce::Point<float> (centre.x + std::cos (tickAngle) * radius * 1.25f,
                                                  centre.y + std::sin (tickAngle) * radius * 1.25f);
            g.setColour (juce::Colour (0xffd2d2d2).withAlpha (alpha));
            g.drawLine ({ inner, outer }, tick % 3 == 0 ? 1.6f : 1.0f);
        }
    }

    static void drawMetalBody (juce::Graphics& g, juce::Point<float> centre, float radius, float angle)
    {
        const auto knobBounds = juce::Rectangle<float> (centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f)
                                  .reduced (10.0f);
        const auto knobRadius = knobBounds.getWidth() * 0.5f;
        const auto knobCentre = knobBounds.getCentre();

        juce::ColourGradient body (juce::Colour (0xfffafafa), knobCentre.x - knobRadius * 0.45f, knobCentre.y - knobRadius,
                                   juce::Colour (0xff5c6064), knobCentre.x + knobRadius * 0.65f, knobCentre.y + knobRadius, false);
        body.addColour (0.36, juce::Colour (0xffc8cace));
        body.addColour (0.52, juce::Colour (0xff83878c));
        body.addColour (0.72, juce::Colour (0xffeeeeee));
        g.setGradientFill (body);
        g.fillEllipse (knobBounds);

        for (int line = -24; line <= 24; ++line)
        {
            const auto yPos = knobCentre.y + static_cast<float> (line) * knobRadius / 24.0f;
            const auto halfWidth = std::sqrt (juce::jmax (0.0f, knobRadius * knobRadius - std::pow (yPos - knobCentre.y, 2.0f)));
            const auto bright = line % 2 == 0 ? 0.12f : 0.05f;
            g.setColour (juce::Colour (0xffffffff).withAlpha (bright));
            g.drawHorizontalLine (static_cast<int> (std::round (yPos)),
                                  knobCentre.x - halfWidth * 0.88f,
                                  knobCentre.x + halfWidth * 0.88f);
        }

        g.setColour (juce::Colour (0xff121314).withAlpha (0.30f));
        g.drawEllipse (knobBounds.reduced (1.0f), 2.0f);

        juce::ColourGradient bevel (juce::Colour (0xffffffff).withAlpha (0.64f), knobCentre.x, knobCentre.y - knobRadius,
                                    juce::Colour (0xff050505).withAlpha (0.52f), knobCentre.x, knobCentre.y + knobRadius, false);
        g.setGradientFill (bevel);
        g.drawEllipse (knobBounds.reduced (3.0f), 3.0f);

        juce::ColourGradient gloss (juce::Colour (0xffffffff).withAlpha (0.36f),
                                    knobCentre.x - knobRadius * 0.35f, knobCentre.y - knobRadius * 0.75f,
                                    juce::Colour (0x00ffffff),
                                    knobCentre.x + knobRadius * 0.15f, knobCentre.y + knobRadius * 0.05f, false);
        g.setGradientFill (gloss);
        g.fillEllipse (knobBounds.withSizeKeepingCentre (knobRadius * 1.15f, knobRadius * 0.82f)
                                  .translated (-knobRadius * 0.22f, -knobRadius * 0.34f));

        const auto grooveStart = juce::Point<float> (knobCentre.x + std::cos (angle) * knobRadius * 0.22f,
                                                    knobCentre.y + std::sin (angle) * knobRadius * 0.22f);
        const auto grooveEnd = juce::Point<float> (knobCentre.x + std::cos (angle) * knobRadius * 0.74f,
                                                  knobCentre.y + std::sin (angle) * knobRadius * 0.74f);
        g.setColour (juce::Colour (0xff101113).withAlpha (0.54f));
        g.drawLine ({ grooveStart, grooveEnd }, 5.0f);
        g.setColour (juce::Colour (0xffffffff).withAlpha (0.18f));
        g.drawLine ({ grooveStart.translated (-1.0f, -1.0f), grooveEnd.translated (-1.0f, -1.0f) }, 1.2f);
    }

    static void drawPointer (juce::Graphics& g, juce::Point<float> centre, float radius, float angle)
    {
        const auto marker = juce::Point<float> (centre.x + std::cos (angle) * radius * 0.72f,
                                               centre.y + std::sin (angle) * radius * 0.72f);
        g.setColour (juce::Colour (0xff20140d).withAlpha (0.36f));
        g.fillEllipse (marker.x - 8.0f, marker.y - 6.0f, 18.0f, 18.0f);
        g.setColour (amber());
        g.fillEllipse (marker.x - 6.0f, marker.y - 8.0f, 14.0f, 14.0f);
        g.setColour (juce::Colour (0xffffffff).withAlpha (0.42f));
        g.fillEllipse (marker.x - 3.0f, marker.y - 6.0f, 5.0f, 5.0f);
    }
};

OneKnobDestroyerAudioProcessorEditor::OneKnobDestroyerAudioProcessorEditor (OneKnobDestroyerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    lookAndFeel = std::make_unique<RuinDialLookAndFeel>();

    titleLabel.setText ("RuinDial", juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setFont (juce::FontOptions (34.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colour (0xfff8f2e8));
    addAndMakeVisible (titleLabel);

    subtitleLabel.setText ("saturation  bit-crush  wobble  grit", juce::dontSendNotification);
    subtitleLabel.setJustificationType (juce::Justification::centred);
    subtitleLabel.setFont (juce::FontOptions (12.0f));
    subtitleLabel.setColour (juce::Label::textColourId, juce::Colour (0xffa9a092));
    addAndMakeVisible (subtitleLabel);

    amountSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    amountSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 104, 25);
    amountSlider.setLookAndFeel (lookAndFeel.get());
    amountSlider.setTextValueSuffix (" destroy");
    addAndMakeVisible (amountSlider);

    amountLabel.setText ("DESTROY", juce::dontSendNotification);
    amountLabel.setJustificationType (juce::Justification::centred);
    amountLabel.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    amountLabel.setColour (juce::Label::textColourId, juce::Colour (0xfff2d6bd));
    addAndMakeVisible (amountLabel);

    amountAttachment = std::make_unique<SliderAttachment> (audioProcessor.parameters, "amount", amountSlider);

    setSize (460, 360);
}

void OneKnobDestroyerAudioProcessorEditor::paint (juce::Graphics& g)
{
    juce::ColourGradient background (panelTop(), 0.0f, 0.0f, panelBottom(), 0.0f, static_cast<float> (getHeight()), false);
    background.addColour (0.55, juce::Colour (0xff191a1d));
    g.setGradientFill (background);
    g.fillAll();

    g.setColour (juce::Colour (0xffffffff).withAlpha (0.035f));
    for (int y = 0; y < getHeight(); y += 3)
        g.drawHorizontalLine (y, 0.0f, static_cast<float> (getWidth()));

    auto panel = getLocalBounds().toFloat().reduced (18.0f);
    juce::ColourGradient panelGradient (juce::Colour (0xff2a2b2f), panel.getX(), panel.getY(),
                                        juce::Colour (0xff151619), panel.getX(), panel.getBottom(), false);
    g.setGradientFill (panelGradient);
    g.fillRoundedRectangle (panel, 14.0f);

    g.setColour (juce::Colour (0xff4b4d52));
    g.drawRoundedRectangle (panel, 14.0f, 1.2f);
    g.setColour (juce::Colour (0xffffffff).withAlpha (0.10f));
    g.drawRoundedRectangle (panel.reduced (2.0f), 12.0f, 1.0f);

    auto topRule = panel.reduced (26.0f).removeFromTop (70.0f).withTrimmedTop (66.0f);
    juce::ColourGradient rule (juce::Colour (0x00ff7a32), topRule.getX(), topRule.getY(),
                               accent().withAlpha (0.82f), topRule.getCentreX(), topRule.getY(), false);
    rule.addColour (1.0, juce::Colour (0x00ff7a32));
    g.setGradientFill (rule);
    g.fillRoundedRectangle (topRule.withHeight (2.0f), 1.0f);

    g.setColour (juce::Colour (0xff050505).withAlpha (0.38f));
    g.fillEllipse (static_cast<float> (getWidth()) * 0.5f - 118.0f,
                   static_cast<float> (getHeight()) * 0.5f - 72.0f,
                   236.0f,
                   236.0f);
}

void OneKnobDestroyerAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (32);
    titleLabel.setBounds (bounds.removeFromTop (42));
    subtitleLabel.setBounds (bounds.removeFromTop (24));
    bounds.removeFromTop (12);
    amountSlider.setBounds (bounds.removeFromTop (218).withSizeKeepingCentre (218, 218));
    amountLabel.setBounds (bounds.removeFromTop (28));
}

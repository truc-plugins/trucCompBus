#pragma once
#include <JuceHeader.h>

class TrucLookAndFeel : public juce::LookAndFeel_V4
{
public:
    TrucLookAndFeel()
    {
        setColour(juce::ResizableWindow::backgroundColourId,  juce::Colour(0xFF1A1A1A));
        setColour(juce::TextButton::buttonColourId,           juce::Colour(0xFF2C2C2C));
        setColour(juce::TextButton::buttonOnColourId,         juce::Colour(0xFFE8E8E8));
        setColour(juce::TextButton::textColourOffId,          juce::Colour(0xFF666666));
        setColour(juce::TextButton::textColourOnId,           juce::Colour(0xFF1A1A1A));
        setColour(juce::Label::textColourId,                  juce::Colour(0xFF666666));
        setColour(juce::Slider::thumbColourId,                juce::Colour(0xFFE8E8E8));
        setColour(juce::Slider::rotarySliderFillColourId,     juce::Colour(0xFFE8E8E8));
        setColour(juce::Slider::rotarySliderOutlineColourId,  juce::Colour(0xFF333333));
    }

    juce::Font getLabelFont(juce::Label&) override
    {
        return juce::Font(juce::FontOptions{}
            .withName(juce::Font::getDefaultMonospacedFontName())
            .withHeight(9.0f));
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider&) override
    {
        const float radius = std::min(width, height) * 0.5f - 4.0f;
        const float cx     = x + width  * 0.5f;
        const float cy     = y + height * 0.5f;

        juce::Path track;
        track.addCentredArc(cx, cy, radius, radius, 0.0f,
                            rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(juce::Colour(0xFF333333));
        g.strokePath(track, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        juce::Path fill;
        fill.addCentredArc(cx, cy, radius, radius, 0.0f, rotaryStartAngle, angle, true);
        g.setColour(juce::Colour(0xFFE8E8E8));
        g.strokePath(fill, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

        juce::Path pointer;
        pointer.startNewSubPath(0.0f, -radius * 0.3f);
        pointer.lineTo(0.0f, -radius + 2.0f);
        g.strokePath(pointer, juce::PathStrokeType(2.0f),
                     juce::AffineTransform::rotation(angle).translated(cx, cy));
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                               const juce::Colour&, bool, bool) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        g.setColour(button.getToggleState() ? juce::Colour(0xFFE8E8E8) : juce::Colour(0xFF2C2C2C));
        g.fillRect(bounds);
        g.setColour(juce::Colour(0xFF444444));
        g.drawRect(bounds, 1.0f);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool) override
    {
        g.setColour(button.getToggleState() ? juce::Colour(0xFF1A1A1A) : juce::Colour(0xFF666666));
        g.setFont(juce::Font(juce::FontOptions{}
            .withName(juce::Font::getDefaultMonospacedFontName())
            .withHeight(11.0f)
            .withStyle("Bold")));
        g.drawText(button.getButtonText().toUpperCase(),
                   button.getLocalBounds(), juce::Justification::centred, false);
    }
};

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GRMeter.h"
#include "CustomLookAndFeel.h"

class TrucCompBusEditor : public juce::AudioProcessorEditor,
                          public juce::Timer
{
public:
    explicit TrucCompBusEditor(TrucCompBusProcessor&);
    ~TrucCompBusEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    TrucCompBusProcessor& proc;
    TrucLookAndFeel laf;

    // Threshold knob
    juce::Slider threshKnob;
    juce::Label  threshValueLabel;
    juce::Label  threshTextLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> threshAttach;

    // Output knob
    juce::Slider outputKnob;
    juce::Label  outputValueLabel;
    juce::Label  outputTextLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputAttach;

    // Ratio buttons
    juce::TextButton ratio35Btn { "3.5" };
    juce::TextButton ratio8Btn  { "8"   };
    juce::Label      ratioTextLabel;

    // Mode buttons
    juce::TextButton modeSBtn { "S" };
    juce::TextButton modeABtn { "A" };
    juce::Label      modeTextLabel;

    // GR Meter
    GRMeter     grMeter;
    juce::Label grTextLabel;

    void setupKnob(juce::Slider& knob, juce::Label& valueLabel,
                   juce::Label& textLabel, const juce::String& labelText);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrucCompBusEditor)
};

#include "PluginProcessor.h"
#include "PluginEditor.h"

TrucCompBusProcessor::TrucCompBusProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput ("Input",  juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "STATE", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout
TrucCompBusProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "threshold", "Threshold",
        juce::NormalisableRange<float>(-40.0f, 0.0f, 0.1f), -12.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "output", "Output",
        juce::NormalisableRange<float>(-20.0f, 20.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "ratio", "Ratio",
        juce::StringArray{ "3.5:1", "8:1" }, 0));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "mode", "Mode",
        juce::StringArray{ "S", "A" }, 0));

    return { params.begin(), params.end() };
}

void TrucCompBusProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    outGainSmoothed   = 1.0f;
    compressor.prepare(sampleRate, samplesPerBlock);
}

void TrucCompBusProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                        juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    if (buffer.getNumChannels() < 2) return;

    auto* left  = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);
    const int numSamples = buffer.getNumSamples();

    const float threshDb  = apvts.getRawParameterValue("threshold")->load();
    const float outputDb  = apvts.getRawParameterValue("output")->load();
    const int   ratioIdx  = static_cast<int>(apvts.getRawParameterValue("ratio")->load());
    const int   modeIdx   = static_cast<int>(apvts.getRawParameterValue("mode")->load());

    const float ratioVal = (ratioIdx == 0) ? 3.5f : 8.0f;
    const auto  mode     = (modeIdx == 0) ? BusCompressor::Mode::S : BusCompressor::Mode::A;

    compressor.process(left, right, numSamples, threshDb, ratioVal, mode);

    // Output gain with per-block smoothing (~10 ms) to avoid clicks on automation
    const float targetGain    = juce::Decibels::decibelsToGain(outputDb);
    const float outSmoothCoeff = std::exp(-float(numSamples) / (float(currentSampleRate) * 0.010f));
    outGainSmoothed = targetGain + outSmoothCoeff * (outGainSmoothed - targetGain);
    buffer.applyGain(outGainSmoothed);
}

void TrucCompBusProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void TrucCompBusProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* TrucCompBusProcessor::createEditor()
{
    return new TrucCompBusEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TrucCompBusProcessor();
}

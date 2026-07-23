#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace ParamIDs
{
constexpr auto inputGain = "inputGain";
constexpr auto subsonic = "subsonic";
constexpr auto bassMono = "bassMono";
constexpr auto bassWidth = "bassWidth";
constexpr auto deEssFreq = "deEssFreq";
constexpr auto deEssThreshold = "deEssThreshold";
constexpr auto deEssAmount = "deEssAmount";
constexpr auto softClip = "softClip";
constexpr auto outputCeiling = "outputCeiling";
constexpr auto outputGain = "outputGain";
constexpr auto bypass = "bypass";
}

GuruVynilMasterSuiteAudioProcessor::GuruVynilMasterSuiteAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

void GuruVynilMasterSuiteAudioProcessor::prepareToPlay(double newSampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = newSampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());
    dsp.prepare(spec);
}

void GuruVynilMasterSuiteAudioProcessor::releaseResources()
{
    dsp.reset();
}

bool GuruVynilMasterSuiteAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto output = layouts.getMainOutputChannelSet();
    if (output != juce::AudioChannelSet::mono() && output != juce::AudioChannelSet::stereo())
        return false;
    return output == layouts.getMainInputChannelSet();
}

void GuruVynilMasterSuiteAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (int channel = getTotalNumInputChannels(); channel < getTotalNumOutputChannels(); ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    if (apvts.getRawParameterValue(ParamIDs::bypass)->load() < 0.5f)
        dsp.process(buffer, getCurrentParameters());
}

juce::AudioProcessorEditor* GuruVynilMasterSuiteAudioProcessor::createEditor()
{
    return new GuruVynilMasterSuiteAudioProcessorEditor(*this);
}

void GuruVynilMasterSuiteAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void GuruVynilMasterSuiteAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

GuruVinylParameters GuruVynilMasterSuiteAudioProcessor::getCurrentParameters() const
{
    GuruVinylParameters p;
    p.inputGainDb = apvts.getRawParameterValue(ParamIDs::inputGain)->load();
    p.subsonicHz = apvts.getRawParameterValue(ParamIDs::subsonic)->load();
    p.bassMonoHz = apvts.getRawParameterValue(ParamIDs::bassMono)->load();
    p.bassWidthPercent = apvts.getRawParameterValue(ParamIDs::bassWidth)->load();
    p.deEssHz = apvts.getRawParameterValue(ParamIDs::deEssFreq)->load();
    p.deEssThresholdDb = apvts.getRawParameterValue(ParamIDs::deEssThreshold)->load();
    p.deEssAmountDb = apvts.getRawParameterValue(ParamIDs::deEssAmount)->load();
    p.softClipPercent = apvts.getRawParameterValue(ParamIDs::softClip)->load();
    p.outputCeilingDb = apvts.getRawParameterValue(ParamIDs::outputCeiling)->load();
    p.outputGainDb = apvts.getRawParameterValue(ParamIDs::outputGain)->load();
    return p;
}

juce::AudioProcessorValueTreeState::ParameterLayout GuruVynilMasterSuiteAudioProcessor::createParameterLayout()
{
    using APF = juce::AudioParameterFloat;
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<APF>(ParamIDs::inputGain, "Input", juce::NormalisableRange<float>(-12.0f, 12.0f, 0.1f), 0.0f, "dB"));
    layout.add(std::make_unique<APF>(ParamIDs::subsonic, "Subsonic", juce::NormalisableRange<float>(15.0f, 40.0f, 0.1f), 25.0f, "Hz"));
    layout.add(std::make_unique<APF>(ParamIDs::bassMono, "Bass Control", juce::NormalisableRange<float>(60.0f, 300.0f, 1.0f, 0.45f), 120.0f, "Hz"));
    layout.add(std::make_unique<APF>(ParamIDs::bassWidth, "Low Width", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 15.0f, "%"));
    layout.add(std::make_unique<APF>(ParamIDs::deEssFreq, "HF Control", juce::NormalisableRange<float>(3000.0f, 16000.0f, 10.0f, 0.4f), 6800.0f, "Hz"));
    layout.add(std::make_unique<APF>(ParamIDs::deEssThreshold, "HF Threshold", juce::NormalisableRange<float>(-36.0f, 0.0f, 0.1f), -18.0f, "dB"));
    layout.add(std::make_unique<APF>(ParamIDs::deEssAmount, "HF Max", juce::NormalisableRange<float>(0.0f, 12.0f, 0.1f), 4.0f, "dB"));
    layout.add(std::make_unique<APF>(ParamIDs::softClip, "Soft Clip", juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f), 15.0f, "%"));
    layout.add(std::make_unique<APF>(ParamIDs::outputCeiling, "Ceiling", juce::NormalisableRange<float>(-3.0f, -0.1f, 0.1f), -1.0f, "dB"));
    layout.add(std::make_unique<APF>(ParamIDs::outputGain, "Output", juce::NormalisableRange<float>(-12.0f, 6.0f, 0.1f), 0.0f, "dB"));
    layout.add(std::make_unique<juce::AudioParameterBool>(ParamIDs::bypass, "Bypass", false));

    return layout;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GuruVynilMasterSuiteAudioProcessor();
}

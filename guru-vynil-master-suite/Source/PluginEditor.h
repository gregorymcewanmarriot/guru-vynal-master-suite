#pragma once

#include <JuceHeader.h>
#include <array>
#include <memory>
#include "PluginProcessor.h"
#include "GuruLookAndFeel.h"

class GuruVynilMasterSuiteAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                                        private juce::Timer
{
public:
    explicit GuruVynilMasterSuiteAudioProcessorEditor(GuruVynilMasterSuiteAudioProcessor&);
    ~GuruVynilMasterSuiteAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    struct Knob
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    void configureKnob(Knob& knob, const juce::String& parameterId, const juce::String& title);
    void timerCallback() override;
    void applyPreset(int presetIndex);
    void setParameter(const juce::String& id, float value);
    static juce::String readinessText(float risk);
    static juce::Colour readinessColour(float risk);

    GuruVynilMasterSuiteAudioProcessor& processor;
    GuruLookAndFeel lookAndFeel;

    Knob input;
    Knob subsonic;
    Knob bassControl;
    Knob lowWidth;
    Knob hfControl;
    Knob hfThreshold;
    Knob hfMax;
    Knob softClip;
    Knob ceiling;
    Knob output;

    std::array<Knob*, 10> knobs;

    juce::ComboBox presetBox;
    juce::TextButton autoSafeButton { "AUTO SAFE" };
    juce::TextButton bypassButton { "BYPASS" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    float displayedRisk = 0.0f;
    float displayedCorrelation = 1.0f;
    float displayedInput = -100.0f;
    float displayedOutput = -100.0f;
    float displayedLowSide = -100.0f;
    float displayedDeEss = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GuruVynilMasterSuiteAudioProcessorEditor)
};

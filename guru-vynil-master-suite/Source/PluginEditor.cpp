#include "PluginEditor.h"

namespace
{
constexpr int designWidth = 1040;
constexpr int designHeight = 640;

const juce::StringArray presetNames {
    "12-inch LP - Natural",
    "12-inch LP - Safe",
    "12-inch 45 RPM",
    "7-inch 45 RPM",
    "Heavy Rock",
    "Extreme Metal",
    "Electronic Bass",
    "Acoustic / Jazz",
    "Diagnostic Only"
};
}

GuruVynilMasterSuiteAudioProcessorEditor::GuruVynilMasterSuiteAudioProcessorEditor(
    GuruVynilMasterSuiteAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processor(p),
      knobs { &input, &subsonic, &bassControl, &lowWidth, &hfControl,
              &hfThreshold, &hfMax, &softClip, &ceiling, &output }
{
    setLookAndFeel(&lookAndFeel);
    setResizable(true, true);
    setResizeLimits(830, 520, 1460, 900);
    setSize(designWidth, designHeight);

    configureKnob(input, "inputGain", "INPUT");
    configureKnob(subsonic, "subsonic", "SUBSONIC");
    configureKnob(bassControl, "bassMono", "BASS CONTROL");
    configureKnob(lowWidth, "bassWidth", "LOW WIDTH");
    configureKnob(hfControl, "deEssFreq", "HF CONTROL");
    configureKnob(hfThreshold, "deEssThreshold", "HF THRESHOLD");
    configureKnob(hfMax, "deEssAmount", "HF MAX");
    configureKnob(softClip, "softClip", "SOFT CLIP");
    configureKnob(ceiling, "outputCeiling", "CEILING");
    configureKnob(output, "outputGain", "OUTPUT");

    for (int index = 0; index < presetNames.size(); ++index)
        presetBox.addItem(presetNames[index], index + 1);
    presetBox.setSelectedId(1, juce::dontSendNotification);
    presetBox.onChange = [this] { applyPreset(presetBox.getSelectedItemIndex()); };
    addAndMakeVisible(presetBox);

    autoSafeButton.onClick = [this]
    {
        setParameter("subsonic", 28.0f);
        setParameter("bassMono", 145.0f);
        setParameter("bassWidth", 0.0f);
        setParameter("deEssThreshold", -21.0f);
        setParameter("deEssAmount", 5.0f);
        setParameter("softClip", 10.0f);
        setParameter("outputCeiling", -1.2f);
    };
    addAndMakeVisible(autoSafeButton);

    bypassButton.setClickingTogglesState(true);
    addAndMakeVisible(bypassButton);
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        processor.apvts, "bypass", bypassButton);

    startTimerHz(30);
}

GuruVynilMasterSuiteAudioProcessorEditor::~GuruVynilMasterSuiteAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void GuruVynilMasterSuiteAudioProcessorEditor::configureKnob(Knob& knob,
                                                              const juce::String& parameterId,
                                                              const juce::String& title)
{
    knob.slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    knob.slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 74, 22);
    if (auto* parameter = processor.apvts.getParameter(parameterId))
        knob.slider.setDoubleClickReturnValue(true, parameter->convertFrom0to1(parameter->getDefaultValue()));
    addAndMakeVisible(knob.slider);

    knob.label.setText(title, juce::dontSendNotification);
    knob.label.setJustificationType(juce::Justification::centred);
    knob.label.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    knob.label.setColour(juce::Label::textColourId, juce::Colour(0xffb9b3a7));
    addAndMakeVisible(knob.label);

    knob.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.apvts, parameterId, knob.slider);
}

void GuruVynilMasterSuiteAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0c0d0f));

    juce::ColourGradient background(juce::Colour(0xff202328), 0.0f, 0.0f,
                                    juce::Colour(0xff0b0c0d), 0.0f,
                                    static_cast<float>(getHeight()), false);
    g.setGradientFill(background);
    g.fillRect(getLocalBounds());

    auto header = getLocalBounds().removeFromTop(86);
    g.setColour(juce::Colour(0xff0a0b0c));
    g.fillRect(header);
    g.setColour(juce::Colour(0xffd6922e));
    g.fillRect(0, header.getBottom() - 3, getWidth(), 3);

    g.setColour(juce::Colour(0xfff1e7d2));
    g.setFont(juce::FontOptions(28.0f, juce::Font::bold));
    g.drawText("GURU VYNIL MASTER SUITE", 26, 15, 560, 36, juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff888d91));
    g.setFont(juce::FontOptions(12.0f));
    g.drawText("PREPARE THE MASTER  |  PROTECT THE CUT", 28, 52, 480, 20,
               juce::Justification::centredLeft);

    const auto riskColour = readinessColour(displayedRisk);
    g.setColour(juce::Colour(0xff151719));
    g.fillRoundedRectangle(static_cast<float>(getWidth() - 270), 16.0f, 244.0f, 54.0f, 6.0f);
    g.setColour(riskColour);
    g.drawRoundedRectangle(static_cast<float>(getWidth() - 270), 16.0f, 244.0f, 54.0f, 6.0f, 2.0f);
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.drawText("CUT READINESS", getWidth() - 254, 22, 100, 18, juce::Justification::centredLeft);
    g.setFont(juce::FontOptions(22.0f, juce::Font::bold));
    g.drawText(readinessText(displayedRisk), getWidth() - 254, 39, 210, 25,
               juce::Justification::centredLeft);

    const auto scaleX = static_cast<float>(getWidth()) / static_cast<float>(designWidth);
    const auto scaleY = static_cast<float>(getHeight()) / static_cast<float>(designHeight);
    const auto meterArea = juce::Rectangle<float>(36.0f * scaleX, 382.0f * scaleY,
                                                   968.0f * scaleX, 154.0f * scaleY);
    g.setColour(juce::Colour(0xff121416));
    g.fillRoundedRectangle(meterArea, 7.0f);
    g.setColour(juce::Colour(0xff3a3e42));
    g.drawRoundedRectangle(meterArea, 7.0f, 1.0f);

    struct MeterItem { const char* title; float value; float minimum; float maximum; juce::String suffix; };
    const std::array<MeterItem, 5> meters {{
        { "INPUT PEAK", displayedInput, -60.0f, 0.0f, " dB" },
        { "CORRELATION", displayedCorrelation, -1.0f, 1.0f, "" },
        { "LOW SIDE", displayedLowSide, -60.0f, 0.0f, " dB" },
        { "HF REDUCTION", displayedDeEss, 0.0f, 12.0f, " dB" },
        { "OUTPUT PEAK", displayedOutput, -60.0f, 0.0f, " dB" }
    }};

    const float gap = 14.0f * scaleX;
    const float cellWidth = (meterArea.getWidth() - gap * 6.0f) / 5.0f;
    for (size_t i = 0; i < meters.size(); ++i)
    {
        auto cell = juce::Rectangle<float>(meterArea.getX() + gap + static_cast<float>(i) * (cellWidth + gap),
                                           meterArea.getY() + 18.0f * scaleY,
                                           cellWidth, meterArea.getHeight() - 36.0f * scaleY);
        g.setColour(juce::Colour(0xff8e9295));
        g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
        g.drawText(meters[i].title, cell.removeFromTop(18.0f * scaleY).toNearestInt(),
                   juce::Justification::centred);

        auto bar = cell.removeFromTop(18.0f * scaleY).reduced(2.0f, 4.0f);
        const float normal = juce::jlimit(0.0f, 1.0f,
            juce::jmap(meters[i].value, meters[i].minimum, meters[i].maximum, 0.0f, 1.0f));
        g.setColour(juce::Colour(0xff282b2e));
        g.fillRoundedRectangle(bar, 3.0f);
        g.setColour((i == 1 && displayedCorrelation < 0.0f) ? juce::Colour(0xffc8473d)
                                                            : juce::Colour(0xffd6922e));
        g.fillRoundedRectangle(bar.withWidth(bar.getWidth() * normal), 3.0f);

        g.setColour(juce::Colour(0xfff0e8d7));
        g.setFont(juce::FontOptions(18.0f, juce::Font::bold));
        const auto decimals = i == 1 ? 2 : 1;
        g.drawText(juce::String(meters[i].value, decimals) + meters[i].suffix,
                   cell.toNearestInt(), juce::Justification::centred);
    }

    g.setColour(juce::Colour(0xff6e7377));
    g.setFont(juce::FontOptions(10.0f));
    g.drawText("Diagnostic estimate only - final cutting decisions remain with the cutting engineer.",
               36, getHeight() - 39, getWidth() - 72, 16, juce::Justification::centredLeft);
    g.drawText("v0.1.1", getWidth() - 100, getHeight() - 39, 64, 16,
               juce::Justification::centredRight);
}

void GuruVynilMasterSuiteAudioProcessorEditor::resized()
{
    const float scaleX = static_cast<float>(getWidth()) / static_cast<float>(designWidth);
    const float scaleY = static_cast<float>(getHeight()) / static_cast<float>(designHeight);

    presetBox.setBounds(36, 101, juce::roundToInt(250.0f * scaleX), juce::roundToInt(30.0f * scaleY));
    autoSafeButton.setBounds(juce::roundToInt(300.0f * scaleX), 101,
                             juce::roundToInt(122.0f * scaleX), juce::roundToInt(30.0f * scaleY));
    bypassButton.setBounds(getWidth() - juce::roundToInt(140.0f * scaleX), 101,
                           juce::roundToInt(104.0f * scaleX), juce::roundToInt(30.0f * scaleY));

    const float startX = 31.0f * scaleX;
    const float top = 151.0f * scaleY;
    const float totalWidth = 978.0f * scaleX;
    const float knobWidth = totalWidth / static_cast<float>(knobs.size());
    const int sliderHeight = juce::roundToInt(154.0f * scaleY);

    for (size_t i = 0; i < knobs.size(); ++i)
    {
        const int x = juce::roundToInt(startX + static_cast<float>(i) * knobWidth);
        const int width = juce::roundToInt(knobWidth - 4.0f * scaleX);
        knobs[i]->label.setBounds(x, juce::roundToInt(top), width, juce::roundToInt(22.0f * scaleY));
        knobs[i]->slider.setBounds(x, juce::roundToInt(top + 22.0f * scaleY), width, sliderHeight);
    }
}

void GuruVynilMasterSuiteAudioProcessorEditor::timerCallback()
{
    auto& dsp = processor.getDSP();
    displayedRisk = juce::jmap(0.18f, displayedRisk, dsp.getRiskScore());
    displayedCorrelation = juce::jmap(0.18f, displayedCorrelation, dsp.getCorrelation());
    displayedInput = juce::jmap(0.20f, displayedInput, dsp.getInputPeakDb());
    displayedOutput = juce::jmap(0.20f, displayedOutput, dsp.getOutputPeakDb());
    displayedLowSide = juce::jmap(0.18f, displayedLowSide, dsp.getLowSideDb());
    displayedDeEss = juce::jmap(0.20f, displayedDeEss, dsp.getDeEssReductionDb());
    repaint();
}

void GuruVynilMasterSuiteAudioProcessorEditor::setParameter(const juce::String& id, float value)
{
    if (auto* parameter = processor.apvts.getParameter(id))
    {
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
        parameter->endChangeGesture();
    }
}

void GuruVynilMasterSuiteAudioProcessorEditor::applyPreset(int presetIndex)
{
    struct Values
    {
        float input, sub, bass, width, hf, threshold, max, clip, ceiling, output;
    };

    const std::array<Values, 9> presets {{
        { 0.0f, 22.0f, 105.0f, 30.0f, 7200.0f, -16.0f, 3.0f, 8.0f, -1.0f, 0.0f },
        { 0.0f, 28.0f, 150.0f, 0.0f, 6500.0f, -20.0f, 5.0f, 10.0f, -1.2f, 0.0f },
        { 0.0f, 25.0f, 125.0f, 15.0f, 7000.0f, -18.0f, 4.0f, 12.0f, -1.0f, 0.0f },
        { -1.0f, 30.0f, 170.0f, 0.0f, 6200.0f, -21.0f, 6.0f, 8.0f, -1.5f, 0.0f },
        { -0.5f, 25.0f, 120.0f, 12.0f, 6600.0f, -18.0f, 4.5f, 18.0f, -1.0f, 0.0f },
        { -1.0f, 28.0f, 145.0f, 5.0f, 5800.0f, -22.0f, 6.5f, 22.0f, -1.2f, 0.0f },
        { -1.0f, 30.0f, 180.0f, 0.0f, 7400.0f, -17.0f, 3.0f, 15.0f, -1.2f, 0.0f },
        { 0.0f, 20.0f, 90.0f, 45.0f, 8200.0f, -14.0f, 2.5f, 4.0f, -1.0f, 0.0f },
        { 0.0f, 15.0f, 60.0f, 100.0f, 12000.0f, 0.0f, 0.0f, 0.0f, -0.1f, 0.0f }
    }};

    if (!juce::isPositiveAndBelow(presetIndex, static_cast<int>(presets.size())))
        return;

    const auto& v = presets[static_cast<size_t>(presetIndex)];
    setParameter("inputGain", v.input);
    setParameter("subsonic", v.sub);
    setParameter("bassMono", v.bass);
    setParameter("bassWidth", v.width);
    setParameter("deEssFreq", v.hf);
    setParameter("deEssThreshold", v.threshold);
    setParameter("deEssAmount", v.max);
    setParameter("softClip", v.clip);
    setParameter("outputCeiling", v.ceiling);
    setParameter("outputGain", v.output);
}

juce::String GuruVynilMasterSuiteAudioProcessorEditor::readinessText(float risk)
{
    if (risk < 30.0f)
        return "READY";
    if (risk < 65.0f)
        return "REVIEW";
    return "CUT RISK";
}

juce::Colour GuruVynilMasterSuiteAudioProcessorEditor::readinessColour(float risk)
{
    if (risk < 30.0f)
        return juce::Colour(0xff6cad71);
    if (risk < 65.0f)
        return juce::Colour(0xffd6922e);
    return juce::Colour(0xffc8473d);
}

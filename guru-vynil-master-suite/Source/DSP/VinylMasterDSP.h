#pragma once

#include <JuceHeader.h>
#include <atomic>

struct GuruVinylParameters
{
    float inputGainDb = 0.0f;
    float subsonicHz = 25.0f;
    float bassMonoHz = 120.0f;
    float bassWidthPercent = 15.0f;
    float deEssHz = 6800.0f;
    float deEssThresholdDb = -18.0f;
    float deEssAmountDb = 4.0f;
    float softClipPercent = 15.0f;
    float outputCeilingDb = -1.0f;
    float outputGainDb = 0.0f;
};

class VinylMasterDSP
{
public:
    VinylMasterDSP();

    void prepare(const juce::dsp::ProcessSpec& spec);
    void reset();
    void process(juce::AudioBuffer<float>& buffer, const GuruVinylParameters& params);

    float getInputPeakDb() const noexcept { return inputPeakDb.load(); }
    float getOutputPeakDb() const noexcept { return outputPeakDb.load(); }
    float getInputPeakHoldDb() const noexcept { return inputPeakHoldDb.load(); }
    float getOutputPeakHoldDb() const noexcept { return outputPeakHoldDb.load(); }
    float getCorrelation() const noexcept { return correlation.load(); }
    float getLowSideDb() const noexcept { return lowSideDb.load(); }
    float getDeEssReductionDb() const noexcept { return deEssReductionDb.load(); }
    float getRiskScore() const noexcept { return riskScore.load(); }
    bool hasSignal() const noexcept { return signalPresent.load(); }
    bool isAnalysisReady() const noexcept { return analysisReady.load(); }
    bool hasOverloadWarning() const noexcept { return overloadWarning.load(); }
    bool hasCeilingWarning() const noexcept { return ceilingWarning.load(); }

private:
    void updateFilters(const GuruVinylParameters& params);
    static float gainToDb(float gain) noexcept;
    static float dbToGain(float db) noexcept;

    double sampleRate = 44100.0;
    int maxBlockSize = 512;

    juce::dsp::StateVariableTPTFilter<float> subsonicStageA;
    juce::dsp::StateVariableTPTFilter<float> subsonicStageB;

    juce::dsp::LinkwitzRileyFilter<float> sideCrossover;

    juce::dsp::StateVariableTPTFilter<float> deEssHighPass;
    juce::dsp::Limiter<float> limiter;

    float deEssEnvelope = 0.0f;
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;

    float lastSubsonicHz = -1.0f;
    float lastBassMonoHz = -1.0f;
    float lastDeEssHz = -1.0f;
    float lastCeilingDb = -99.0f;

    std::atomic<float> inputPeakDb { -100.0f };
    std::atomic<float> outputPeakDb { -100.0f };
    std::atomic<float> inputPeakHoldDb { -100.0f };
    std::atomic<float> outputPeakHoldDb { -100.0f };
    std::atomic<float> correlation { 1.0f };
    std::atomic<float> lowSideDb { -100.0f };
    std::atomic<float> deEssReductionDb { 0.0f };
    std::atomic<float> riskScore { 0.0f };
    std::atomic<bool> signalPresent { false };
    std::atomic<bool> analysisReady { false };
    std::atomic<bool> overloadWarning { false };
    std::atomic<bool> ceilingWarning { false };
    double analysedSeconds = 0.0;
    float averagedRisk = 0.0f;
    float inputPeakHold = 0.0f;
    float outputPeakHold = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VinylMasterDSP)
};

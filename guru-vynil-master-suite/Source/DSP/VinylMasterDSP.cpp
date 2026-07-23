#include "VinylMasterDSP.h"
#include <cmath>

namespace
{
constexpr float kMinusInfinityDb = -100.0f;
constexpr float kInvSqrt2 = 0.7071067811865475f;
}

VinylMasterDSP::VinylMasterDSP()
{
    subsonicStageA.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    subsonicStageB.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    deEssHighPass.setType(juce::dsp::StateVariableTPTFilterType::highpass);
}

void VinylMasterDSP::prepare(const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;
    maxBlockSize = static_cast<int>(spec.maximumBlockSize);

    subsonicStageA.prepare(spec);
    subsonicStageB.prepare(spec);
    deEssHighPass.prepare(spec);
    limiter.prepare(spec);

    auto monoSpec = spec;
    monoSpec.numChannels = 1;
    sideCrossover.prepare(monoSpec);

    const auto attackSeconds = 0.002;
    const auto releaseSeconds = 0.080;
    attackCoeff = static_cast<float>(std::exp(-1.0 / (attackSeconds * sampleRate)));
    releaseCoeff = static_cast<float>(std::exp(-1.0 / (releaseSeconds * sampleRate)));

    reset();
}

void VinylMasterDSP::reset()
{
    subsonicStageA.reset();
    subsonicStageB.reset();
    sideCrossover.reset();
    deEssHighPass.reset();
    limiter.reset();
    deEssEnvelope = 0.0f;
    analysedSeconds = 0.0;
    averagedRisk = 0.0f;
    inputPeakHold = 0.0f;
    outputPeakHold = 0.0f;
    signalPresent.store(false);
    analysisReady.store(false);
    overloadWarning.store(false);
    ceilingWarning.store(false);
    inputPeakDb.store(kMinusInfinityDb);
    outputPeakDb.store(kMinusInfinityDb);
    inputPeakHoldDb.store(kMinusInfinityDb);
    outputPeakHoldDb.store(kMinusInfinityDb);
    riskScore.store(0.0f);
}

void VinylMasterDSP::updateFilters(const GuruVinylParameters& params)
{
    if (std::abs(params.subsonicHz - lastSubsonicHz) > 0.01f)
    {
        subsonicStageA.setCutoffFrequency(params.subsonicHz);
        subsonicStageB.setCutoffFrequency(params.subsonicHz);
        subsonicStageA.setResonance(0.7071f);
        subsonicStageB.setResonance(0.7071f);
        lastSubsonicHz = params.subsonicHz;
    }

    if (std::abs(params.bassMonoHz - lastBassMonoHz) > 0.01f)
    {
        sideCrossover.setCutoffFrequency(params.bassMonoHz);
        lastBassMonoHz = params.bassMonoHz;
    }

    if (std::abs(params.deEssHz - lastDeEssHz) > 0.01f)
    {
        deEssHighPass.setCutoffFrequency(params.deEssHz);
        deEssHighPass.setResonance(0.7071f);
        lastDeEssHz = params.deEssHz;
    }

    if (std::abs(params.outputCeilingDb - lastCeilingDb) > 0.01f)
    {
        limiter.setThreshold(params.outputCeilingDb);
        limiter.setRelease(80.0f);
        lastCeilingDb = params.outputCeilingDb;
    }
}

float VinylMasterDSP::gainToDb(float gain) noexcept
{
    return gain <= 0.00001f ? kMinusInfinityDb : 20.0f * std::log10(gain);
}

float VinylMasterDSP::dbToGain(float db) noexcept
{
    return std::pow(10.0f, db / 20.0f);
}

void VinylMasterDSP::process(juce::AudioBuffer<float>& buffer, const GuruVinylParameters& params)
{
    const auto channels = buffer.getNumChannels();
    const auto samples = buffer.getNumSamples();
    if (channels == 0 || samples == 0)
        return;

    updateFilters(params);

    const float inputGain = dbToGain(params.inputGainDb);
    const float outputGain = dbToGain(params.outputGainDb);
    const float lowSideGain = juce::jlimit(0.0f, 1.0f, params.bassWidthPercent / 100.0f);
    const float maxReduction = juce::jmax(0.0f, params.deEssAmountDb);
    const float clipMix = juce::jlimit(0.0f, 1.0f, params.softClipPercent / 100.0f);
    const float clipDrive = 1.0f + clipMix * 3.5f;
    const float clipNorm = 1.0f / std::tanh(clipDrive);

    float blockInputPeak = 0.0f;
    float blockOutputPeak = 0.0f;
    float maxDeEssReduction = 0.0f;
    double sumL2 = 0.0;
    double sumR2 = 0.0;
    double sumLR = 0.0;
    double sumLowSide2 = 0.0;

    for (int sample = 0; sample < samples; ++sample)
    {
        const float dryLeft = buffer.getSample(0, sample);
        const float dryRight = channels > 1 ? buffer.getSample(1, sample) : dryLeft;
        float left = dryLeft * inputGain;
        float right = dryRight * inputGain;

        blockInputPeak = juce::jmax(blockInputPeak, juce::jmax(std::abs(left), std::abs(right)));

        left = subsonicStageA.processSample(0, left);
        left = subsonicStageB.processSample(0, left);
        if (channels > 1)
        {
            right = subsonicStageA.processSample(1, right);
            right = subsonicStageB.processSample(1, right);
        }
        else
        {
            right = left;
        }

        const float mid = (left + right) * kInvSqrt2;
        const float side = (left - right) * kInvSqrt2;
        float sideLow = 0.0f;
        float sideHigh = 0.0f;
        sideCrossover.processSample(0, side, sideLow, sideHigh);
        const float controlledSide = sideHigh + sideLow * lowSideGain;

        left = (mid + controlledSide) * kInvSqrt2;
        right = (mid - controlledSide) * kInvSqrt2;
        sumLowSide2 += static_cast<double>(sideLow) * static_cast<double>(sideLow);

        const float highL = deEssHighPass.processSample(0, left);
        const float highR = channels > 1 ? deEssHighPass.processSample(1, right) : highL;
        const float detector = juce::jmax(std::abs(highL), std::abs(highR));
        const float coeff = detector > deEssEnvelope ? attackCoeff : releaseCoeff;
        deEssEnvelope = coeff * deEssEnvelope + (1.0f - coeff) * detector;

        const float envelopeDb = gainToDb(deEssEnvelope);
        const float overDb = juce::jmax(0.0f, envelopeDb - params.deEssThresholdDb);
        const float reductionDb = juce::jmin(maxReduction, overDb * 0.65f);
        const float highGain = dbToGain(-reductionDb);
        maxDeEssReduction = juce::jmax(maxDeEssReduction, reductionDb);

        left += highL * (highGain - 1.0f);
        right += highR * (highGain - 1.0f);

        if (clipMix > 0.0001f)
        {
            const float clippedL = std::tanh(left * clipDrive) * clipNorm;
            const float clippedR = std::tanh(right * clipDrive) * clipNorm;
            left = left + clipMix * (clippedL - left);
            right = right + clipMix * (clippedR - right);
        }

        left *= outputGain;
        right *= outputGain;

        buffer.setSample(0, sample, left);
        if (channels > 1)
            buffer.setSample(1, sample, right);

        sumL2 += static_cast<double>(left) * static_cast<double>(left);
        sumR2 += static_cast<double>(right) * static_cast<double>(right);
        sumLR += static_cast<double>(left) * static_cast<double>(right);
    }

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    limiter.process(context);

    for (int channel = 0; channel < channels; ++channel)
        blockOutputPeak = juce::jmax(blockOutputPeak, buffer.getMagnitude(channel, 0, samples));

    const double denom = std::sqrt(sumL2 * sumR2);
    const float corr = denom > 1.0e-12 ? static_cast<float>(sumLR / denom) : 1.0f;
    const float sideRms = static_cast<float>(std::sqrt(sumLowSide2 / static_cast<double>(samples)));
    const float sideDb = gainToDb(sideRms);

    float risk = 0.0f;
    if (corr < 0.0f)
        risk += juce::jlimit(0.0f, 40.0f, -corr * 40.0f);
    if (sideDb > -24.0f)
        risk += juce::jlimit(0.0f, 30.0f, (sideDb + 24.0f) * 2.5f);
    risk += juce::jlimit(0.0f, 20.0f, maxDeEssReduction * 3.0f);
    if (gainToDb(blockInputPeak) > -0.5f)
        risk += 10.0f;

    const auto inputDb = gainToDb(blockInputPeak);
    const auto outputDb = gainToDb(blockOutputPeak);
    const bool activeSignal = inputDb > -72.0f;
    if (!activeSignal)
    {
        analysedSeconds = 0.0;
        averagedRisk = 0.0f;
        inputPeakHold = 0.0f;
        outputPeakHold = 0.0f;
    }
    else
    {
        const auto blockSeconds = static_cast<double>(samples) / sampleRate;
        const auto previousSeconds = analysedSeconds;
        analysedSeconds = juce::jmin(4.0, analysedSeconds + blockSeconds);
        const auto alpha = static_cast<float>(blockSeconds / juce::jmax(0.001, analysedSeconds));
        averagedRisk = previousSeconds <= 0.0 ? risk : averagedRisk + alpha * (risk - averagedRisk);
        inputPeakHold = juce::jmax(inputPeakHold * std::pow(0.25f, static_cast<float>(blockSeconds)), blockInputPeak);
        outputPeakHold = juce::jmax(outputPeakHold * std::pow(0.25f, static_cast<float>(blockSeconds)), blockOutputPeak);
    }

    inputPeakDb.store(activeSignal ? inputDb : kMinusInfinityDb);
    outputPeakDb.store(activeSignal ? outputDb : kMinusInfinityDb);
    inputPeakHoldDb.store(gainToDb(inputPeakHold));
    outputPeakHoldDb.store(gainToDb(outputPeakHold));
    correlation.store(juce::jlimit(-1.0f, 1.0f, corr));
    lowSideDb.store(sideDb);
    deEssReductionDb.store(maxDeEssReduction);
    signalPresent.store(activeSignal);
    analysisReady.store(activeSignal && analysedSeconds >= 3.0);
    overloadWarning.store(activeSignal && (inputDb >= -0.1f || outputDb >= -0.1f));
    ceilingWarning.store(activeSignal && (outputDb >= params.outputCeilingDb - 0.15f));
    riskScore.store(juce::jlimit(0.0f, 100.0f, averagedRisk));
}

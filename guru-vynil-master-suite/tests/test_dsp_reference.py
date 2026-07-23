#!/usr/bin/env python3
"""Reference math checks for the Mid/Side and clipping operations."""
import math
import random

INV_SQRT_2 = 1.0 / math.sqrt(2.0)


def encode(left: float, right: float) -> tuple[float, float]:
    return (left + right) * INV_SQRT_2, (left - right) * INV_SQRT_2


def decode(mid: float, side: float) -> tuple[float, float]:
    return (mid + side) * INV_SQRT_2, (mid - side) * INV_SQRT_2


def soft_clip(value: float, amount: float) -> float:
    drive = 1.0 + amount * 3.5
    clipped = math.tanh(value * drive) / math.tanh(drive)
    return value + amount * (clipped - value)


for _ in range(10000):
    left = random.uniform(-2.0, 2.0)
    right = random.uniform(-2.0, 2.0)
    mid, side = encode(left, right)
    rebuilt_left, rebuilt_right = decode(mid, side)
    assert abs(rebuilt_left - left) < 1e-12
    assert abs(rebuilt_right - right) < 1e-12

for amount in (0.0, 0.25, 0.5, 1.0):
    for value in (-10.0, -2.0, -1.0, 0.0, 1.0, 2.0, 10.0):
        result = soft_clip(value, amount)
        assert math.isfinite(result)
        if amount == 0.0:
            assert result == value

print("DSP reference checks passed.")

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
processor_cpp = (ROOT / "Source/PluginProcessor.cpp").read_text(encoding="utf-8")
dsp_cpp = (ROOT / "Source/DSP/VinylMasterDSP.cpp").read_text(encoding="utf-8")
process_block = processor_cpp[
    processor_cpp.index("void GuruVynilMasterSuiteAudioProcessor::processBlock"):
    processor_cpp.index("juce::AudioProcessorEditor*", processor_cpp.index("void GuruVynilMasterSuiteAudioProcessor::processBlock"))
]

assert "deltaAudition" not in dsp_cpp, "DELTA must not affect DSP readiness or metering analysis"
limiter_pos = dsp_cpp.index("limiter.process")
output_meter_pos = dsp_cpp.index("blockOutputPeak", limiter_pos)
assert limiter_pos < output_meter_pos, "Output meters must observe final limiter output"
assert process_block.index("dsp.process") < process_block.index("if (bypassed)") < process_block.index("else if (deltaAudition)")
assert "buffer.copyFrom(channel, 0, dryBuffer" in process_block, "BYPASS must restore dry audio after DSP analysis"
assert "buffer.applyGain(channel, 0, samplesToCopy, -1.0f)" in process_block
assert "buffer.addFrom(channel, 0, dryBuffer, channel, 0, samplesToCopy)" in process_block, "DELTA must output dry minus fully processed audio"
assert "juce::AudioBuffer<float> dry" not in process_block
assert "setSize(" not in process_block
assert "makeCopyOf(" not in process_block

print("Processor output-mode reference checks passed.")

editor_cpp = (ROOT / "Source/PluginEditor.cpp").read_text(encoding="utf-8")
editor_h = (ROOT / "Source/PluginEditor.h").read_text(encoding="utf-8")
assert 'setParameter("bassWidth", 0.0f);' in editor_cpp, "AUTO SAFE must set Low Width to 0%"
assert '{ -1.0f, 28.0f, 145.0f, 15.0f, 5800.0f, -22.0f, 6.5f, 22.0f, -1.2f, 0.0f }' in editor_cpp, "Extreme Metal Low Width must be exactly 15%"
assert "juce::TooltipWindow tooltipWindow" in editor_h, "PluginEditor must own a TooltipWindow for setTooltip calls"

print("Editor regression reference checks passed.")

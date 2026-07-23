#!/usr/bin/env python3
"""Lightweight source-package validation that does not require JUCE."""
from pathlib import Path
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
required = [
    "CMakeLists.txt",
    "Source/PluginProcessor.cpp",
    "Source/PluginProcessor.h",
    "Source/PluginEditor.cpp",
    "Source/PluginEditor.h",
    "Source/GuruLookAndFeel.cpp",
    "Source/GuruLookAndFeel.h",
    "Source/DSP/VinylMasterDSP.cpp",
    "Source/DSP/VinylMasterDSP.h",
    ".github/workflows/build-windows-installer.yml",
    "installer/GuruVynilMasterSuite.iss",
    "CLOUD-BUILD-INSTRUCTIONS.md",
]

errors: list[str] = []
for relative in required:
    if not (ROOT / relative).is_file():
        errors.append(f"Missing required file: {relative}")

processor = (ROOT / "Source/PluginProcessor.cpp").read_text(encoding="utf-8")
editor = (ROOT / "Source/PluginEditor.cpp").read_text(encoding="utf-8")
processor_h = (ROOT / "Source/PluginProcessor.h").read_text(encoding="utf-8")
dsp_cpp = (ROOT / "Source/DSP/VinylMasterDSP.cpp").read_text(encoding="utf-8")
dsp_h = (ROOT / "Source/DSP/VinylMasterDSP.h").read_text(encoding="utf-8")
process_start = processor.find('void GuruVynilMasterSuiteAudioProcessor::processBlock')
process_end = processor.find('juce::AudioProcessorEditor*', process_start)
if process_start < 0 or process_end < 0:
    errors.append("Could not find processBlock body")
else:
    body = processor[process_start:process_end]
    if "juce::AudioBuffer<float> dry" in body or "setSize(" in body or "makeCopyOf(" in body:
        errors.append("processBlock must not allocate or resize AudioBuffer storage")
    bypass_pos = body.find("if (bypassed)")
    delta_pos = body.find("else if (deltaAudition)")
    dsp_pos = body.find("dsp.process")
    if min(bypass_pos, delta_pos, dsp_pos) < 0 or not (dsp_pos < bypass_pos < delta_pos):
        errors.append("processBlock must process first, then apply bypass before delta")
    if "dryBuffer.copyFrom" not in body or "buffer.applyGain(channel, 0, samplesToCopy, -1.0f)" not in body or "buffer.addFrom(channel, 0, dryBuffer" not in body:
        errors.append("processBlock must use reusable dryBuffer for dry/bypass and dry-minus-processed delta output")

if "juce::AudioBuffer<float> dryBuffer" not in processor_h:
    errors.append("PluginProcessor must own a reusable dryBuffer member")
if "dryBuffer.setSize" not in processor:
    errors.append("dryBuffer must be allocated in prepareToPlay")
if "deltaAudition" in dsp_cpp or "deltaAudition" in dsp_h:
    errors.append("VinylMasterDSP must not apply or know about delta audition")
limiter_pos = dsp_cpp.find("limiter.process")
output_peak_pos = dsp_cpp.find("blockOutputPeak", limiter_pos)
if limiter_pos < 0 or output_peak_pos < limiter_pos:
    errors.append("Output metering must be based on the final limiter output")
if 'setParameter("bassWidth", 0.0f);' not in editor:
    errors.append("AUTO SAFE Low Width must be 0%")
if '{ -1.0f, 28.0f, 145.0f, 15.0f, 5800.0f, -22.0f, 6.5f, 22.0f, -1.2f, 0.0f }' not in editor:
    errors.append("Extreme Metal Low Width must be exactly 15%")
if "juce::TooltipWindow tooltipWindow" not in (ROOT / "Source/PluginEditor.h").read_text(encoding="utf-8"):
    errors.append("PluginEditor must own a TooltipWindow for tooltips")


created_ids = set(re.findall(r'ParamIDs::([A-Za-z0-9_]+)', processor))
string_ids = set(re.findall(r'setParameter\("([A-Za-z0-9_]+)"', editor))
attachment_ids = set(re.findall(r'configureKnob\([^,]+,\s*"([A-Za-z0-9_]+)"', editor))
expected_strings = {
    "inputGain", "subsonic", "bassMono", "bassWidth", "deEssFreq",
    "deEssThreshold", "deEssAmount", "softClip", "outputCeiling",
    "outputGain", "bypass", "delta",
}

for param_id in sorted(string_ids | attachment_ids | {"bypass"}):
    if param_id not in expected_strings:
        errors.append(f"Unexpected UI parameter id: {param_id}")

for path in ROOT.rglob("*.cpp"):
    text = path.read_text(encoding="utf-8")
    if text.count("{") != text.count("}"):
        errors.append(f"Unbalanced braces: {path.relative_to(ROOT)}")

cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
if 'PRODUCT_NAME "GURU Vynil Master Suite"' not in cmake:
    errors.append("Product name is not configured correctly")
if "GIT_TAG 8.0.15" not in cmake:
    errors.append("JUCE release is not pinned to 8.0.15")

workflow = (ROOT / ".github/workflows/build-windows-installer.yml").read_text(encoding="utf-8")
installer = (ROOT / "installer/GuruVynilMasterSuite.iss").read_text(encoding="utf-8")
if "runs-on: windows-2022" not in workflow:
    errors.append("Cloud workflow is not pinned to the Windows 2022 runner")
if "workflow_dispatch:" not in workflow:
    errors.append("Cloud workflow cannot be run manually")
if "{commoncf64}\\VST3" not in installer:
    errors.append("Installer does not target the standard 64-bit VST3 directory")

if errors:
    print("Validation failed:")
    for error in errors:
        print(f" - {error}")
    sys.exit(1)

print("Source package validation passed.")
print(f"Checked {len(required)} required project files.")

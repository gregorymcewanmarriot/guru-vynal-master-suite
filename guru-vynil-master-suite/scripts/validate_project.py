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

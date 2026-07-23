# Changelog

## 0.1.1 — Cloud installer build

- Added a GitHub Actions workflow that builds the Windows x64 VST3 and standalone application on a hosted Windows 2022 runner.
- Added an Inno Setup definition that produces a normal Windows installer.
- Added installation of the VST3 bundle into the standard Common Files VST3 directory.
- Added standalone application installation, Start Menu integration and optional desktop shortcut.
- Added automatic SHA-256 checksum generation.
- Added downloadable packed-installer and unpacked-plugin workflow artifacts.
- Added browser-only build instructions requiring no local compiler tools.
- Added explicit UTF-8 source compilation for MSVC.
- Updated the displayed and project version to 0.1.1.
- Clarified that the initial installer is unsigned.

## 0.1.0 — Initial prototype

- Created the GURU Vynil Master Suite JUCE project.
- Added VST3 and standalone targets, with AU enabled on macOS.
- Added stereo input and output gain controls.
- Added cascaded subsonic high-pass filtering.
- Added Mid/Side low-frequency width control.
- Added linked stereo dynamic high-frequency control.
- Added adjustable soft clipping and output limiting.
- Added cut-readiness risk scoring.
- Added input peak, output peak, correlation, low-side and HF-reduction meters.
- Added a resizable dark GURU interface.
- Added nine factory starting presets.
- Added Windows and macOS build scripts.

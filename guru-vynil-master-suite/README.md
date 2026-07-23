# GURU Vynil Master Suite

A transparent vinyl pre-mastering processor for preparing stereo masters before lacquer or DMM cutting.

> Product spelling intentionally follows the requested brand name: **Vynil**.

## Version

Prototype **v0.1.1**

## Included processing

- 24 dB/octave subsonic high-pass filtering
- Frequency-selective low-bass stereo narrowing using Mid/Side processing
- Linked stereo high-frequency dynamic control
- Adjustable soft clipping
- Output safety limiter
- Input/output peak, correlation, low-side energy and HF-reduction metering
- Real-time cut-readiness estimate
- Nine starting presets
- Resizable dark interface
- VST3 and standalone builds on Windows
- VST3, AU and standalone builds on macOS

## Important mastering note

This is a **pre-mastering** processor. It does not apply an RIAA cutting curve and does not simulate surface noise, wow, flutter or record wear. The readiness display is diagnostic guidance, not a guarantee that a particular cutting engineer or lathe can cut the material unchanged.

## No-tools Windows installer build

The project now includes a GitHub Actions cloud-build workflow. It compiles the Windows x64 VST3 and standalone application on a hosted Windows machine, then creates a normal installer using Inno Setup.

See:

```text
CLOUD-BUILD-INSTRUCTIONS.md
```

This route does not require Visual Studio, CMake or Git to be installed on your computer.

## Local Windows build requirements

Local compilation remains available for developers who have:

- Windows 10 or 11, 64-bit
- Visual Studio 2022 with **Desktop development with C++**
- CMake 3.22 or newer
- Git

JUCE 8.0.15 is downloaded automatically during CMake configuration.

## Local build on Windows

Open a Developer Command Prompt in this folder and run:

```bat
build-windows.bat
```

The VST3 bundle will normally be generated under:

```text
build\GuruVynilMasterSuite_artefacts\Release\VST3\
```

## Current limitations

- The safety limiter is not yet a true-peak oversampled limiter.
- Loudness history, LUFS integration and offline programme analysis are planned for a later version.
- Cut-readiness scoring is intentionally conservative and still needs calibration against reference masters and cutting-engineer feedback.
- Bypass is host-automatable, but it is not yet click-free or level-matched.
- The generated Windows installer is unsigned until a commercial code-signing certificate is added.

## Planned milestones

### v0.2

- True-peak oversampled limiter
- Level-matched bypass and delta audition
- Frequency-banded correlation display
- Improved de-esser with selectable split-band and broadband modes
- Parameter smoothing throughout

### v0.3

- Integrated LUFS, short-term LUFS and crest-factor history
- Side-length and playback-speed target profiles
- Exportable mastering report
- Preset management

### v1.0

- Validated Windows and macOS installers
- DAW compatibility testing
- Signed binaries
- Calibrated cut-risk engine
- User manual and factory preset library

## Licensing

The source code in this prototype is copyright GURU. JUCE is a separate dependency with its own licensing terms. Review and obtain the appropriate JUCE licence before closed-source commercial distribution, or comply with the applicable open-source terms.

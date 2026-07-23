cmake_minimum_required(VERSION 3.22)

project(GuruVynilMasterSuite VERSION 0.1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# JUCE can be supplied in one of three ways:
# 1. Place it in ./JUCE
# 2. Configure with -DJUCE_PATH=/absolute/path/to/JUCE
# 3. Allow CMake FetchContent to download the pinned release
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE/CMakeLists.txt")
    add_subdirectory(JUCE)
elseif(DEFINED JUCE_PATH AND EXISTS "${JUCE_PATH}/CMakeLists.txt")
    add_subdirectory("${JUCE_PATH}" "${CMAKE_BINARY_DIR}/JUCE")
else()
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.15
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

set(GURU_FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND GURU_FORMATS AU)
endif()

juce_add_plugin(GuruVynilMasterSuite
    COMPANY_NAME "GURU"
    BUNDLE_ID "co.za.guru.vynilmastersuite"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Guru
    PLUGIN_CODE Gyms
    FORMATS ${GURU_FORMATS}
    PLUGIN_NAME "GURU Vynil Master Suite"
    PRODUCT_NAME "GURU Vynil Master Suite"
    DESCRIPTION "Transparent vinyl pre-mastering and cut-readiness processor"
    VST3_CATEGORIES Fx Mastering
    AU_MAIN_TYPE kAudioUnitType_Effect
)

target_sources(GuruVynilMasterSuite
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/GuruLookAndFeel.cpp
        Source/GuruLookAndFeel.h
        Source/DSP/VinylMasterDSP.cpp
        Source/DSP/VinylMasterDSP.h
)

target_compile_options(GuruVynilMasterSuite
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
)

target_compile_definitions(GuruVynilMasterSuite
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(GuruVynilMasterSuite
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

juce_generate_juce_header(GuruVynilMasterSuite)

cmake_minimum_required(VERSION 3.22)
project(VOCALFIRE VERSION 0.1.0)

include(FetchContent)
FetchContent_Declare(juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(juce)

juce_add_plugin(VOCALFIRE
    COMPANY_NAME "VOCALFIRE Audio"
    BUNDLE_ID "audio.vocalfire.plugin"
    PLUGIN_MANUFACTURER_CODE VcFr
    PLUGIN_CODE VcF1
    FORMATS VST3 Standalone
    PRODUCT_NAME "VOCALFIRE"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)

juce_generate_juce_header(VOCALFIRE)
target_sources(VOCALFIRE PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)
target_compile_features(VOCALFIRE PRIVATE cxx_std_17)
target_compile_definitions(VOCALFIRE PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(VOCALFIRE PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags
           juce::juce_recommended_lto_flags
           juce::juce_recommended_warning_flags)

cmake_minimum_required(VERSION 3.22)
project(trucCompBus VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_OSX_ARCHITECTURES "arm64")
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")

# Allow override: cmake -DJUCE_PATH=...
if(NOT DEFINED JUCE_PATH)
    set(JUCE_PATH "$ENV{HOME}/Desktop/JUCE")
endif()

add_subdirectory(${JUCE_PATH} JUCE EXCLUDE_FROM_ALL)

juce_add_plugin(trucCompBus
    COMPANY_NAME               "Truc"
    PLUGIN_MANUFACTURER_CODE   "Truc"
    PLUGIN_CODE                "TrCB"
    FORMATS                    VST3 AU Standalone
    PRODUCT_NAME               "trucComp@Bus"
    IS_SYNTH                   FALSE
    NEEDS_MIDI_INPUT           FALSE
    NEEDS_MIDI_OUTPUT          FALSE
    IS_MIDI_EFFECT             FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES            "Fx" "Dynamics"
    AU_MAIN_TYPE               "kAudioUnitType_Effect"
    COPY_PLUGIN_AFTER_BUILD    FALSE
)

target_sources(trucCompBus PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

juce_generate_juce_header(trucCompBus)

target_include_directories(trucCompBus PRIVATE Source)

target_compile_definitions(trucCompBus PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(trucCompBus PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

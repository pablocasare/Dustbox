cmake_minimum_required(VERSION 3.22)

if(APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "")
  set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "")
endif()

project(Serpent VERSION 1.0.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE: use a local copy if you pass -DSERPENT_JUCE_PATH=..., otherwise download it
set(SERPENT_JUCE_PATH "" CACHE PATH "Path to a local JUCE checkout")
if(SERPENT_JUCE_PATH)
  add_subdirectory(${SERPENT_JUCE_PATH} JUCE)
else()
  include(FetchContent)
  FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 9.0.4 GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(JUCE)
endif()

set(SERPENT_FORMATS VST3 Standalone)
if(APPLE)
  list(APPEND SERPENT_FORMATS AU)
endif()

juce_add_plugin(Serpent
  COMPANY_NAME             "Synthetik"
  BUNDLE_ID                "com.synthetik.serpent"
  PLUGIN_MANUFACTURER_CODE Synk
  PLUGIN_CODE              Srpt
  PRODUCT_NAME             "Serpent"
  FORMATS                  ${SERPENT_FORMATS}
  IS_SYNTH                 TRUE
  NEEDS_MIDI_INPUT         TRUE
  NEEDS_MIDI_OUTPUT        FALSE
  IS_MIDI_EFFECT           FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS TRUE
  COPY_PLUGIN_AFTER_BUILD  TRUE
  VST3_CATEGORIES          "Instrument" "Synth"
  AU_MAIN_TYPE             "kAudioUnitType_MusicDevice")

target_sources(Serpent PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_definitions(Serpent PUBLIC
  JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0
  JUCE_DISPLAY_SPLASH_SCREEN=0 JUCE_REPORT_APP_USAGE=0)
target_link_libraries(Serpent
  PRIVATE juce::juce_audio_utils juce::juce_dsp
  PUBLIC  juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

option(SERPENT_BUILD_TESTS "Build the offline render test" OFF)
if(SERPENT_BUILD_TESTS)
  juce_add_console_app(SerpentRenderTest PRODUCT_NAME "SerpentRenderTest")
  target_sources(SerpentRenderTest PRIVATE tests/RenderTest.cpp Source/PluginProcessor.cpp Source/PluginEditor.cpp)
  target_compile_definitions(SerpentRenderTest PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JucePlugin_Name="Serpent")
  target_link_libraries(SerpentRenderTest PRIVATE juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags)
endif()

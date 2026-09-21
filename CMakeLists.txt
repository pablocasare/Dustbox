cmake_minimum_required(VERSION 3.22)
project(DustBox VERSION 2.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "")
  set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "")   # universal: Intel + Apple Silicon
endif()

# Point this at an existing JUCE checkout to skip the download:
#   cmake -B build -DDUSTBOX_JUCE_PATH=/path/to/JUCE
set(DUSTBOX_JUCE_PATH "" CACHE PATH "Local JUCE checkout (optional)")
if(DUSTBOX_JUCE_PATH)
  add_subdirectory(${DUSTBOX_JUCE_PATH} ${CMAKE_BINARY_DIR}/JUCE)
else()
  include(FetchContent)
  FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE)
  FetchContent_MakeAvailable(JUCE)
endif()

if(APPLE)
  set(DUSTBOX_FORMATS VST3 AU Standalone)
else()
  set(DUSTBOX_FORMATS VST3 Standalone)
endif()

juce_add_plugin(DustBox
  COMPANY_NAME             "Your Name"
  PLUGIN_MANUFACTURER_CODE Ynme          # 4 chars, at least one uppercase — change before sharing
  PLUGIN_CODE              Dsb2          # 4 chars, at least one uppercase
  FORMATS                  ${DUSTBOX_FORMATS}
  PRODUCT_NAME             "Dust Box"
  IS_SYNTH                 TRUE
  NEEDS_MIDI_INPUT         TRUE
  NEEDS_MIDI_OUTPUT        FALSE
  IS_MIDI_EFFECT           FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS TRUE
  COPY_PLUGIN_AFTER_BUILD  TRUE
  VST3_CATEGORIES          "Instrument" "Synth"
  AU_MAIN_TYPE             "kAudioUnitType_MusicDevice")

target_sources(DustBox PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)

target_compile_definitions(DustBox PUBLIC
  JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0
  JUCE_DISPLAY_SPLASH_SCREEN=0 JUCE_REPORT_APP_USAGE=0)

target_link_libraries(DustBox
  PRIVATE juce::juce_audio_utils juce::juce_dsp
  PUBLIC  juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

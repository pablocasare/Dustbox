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
  COMPANY_NAME             "DustBox"
  BUNDLE_ID                com.dustbox.synth
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

# Find the code wherever it was put: in a Source folder, at the top level, or
# one folder down. The build folder is skipped, because the JUCE download inside
# it has example files with the same names.
set(DUSTBOX_FILES PluginProcessor.cpp PluginEditor.cpp PluginProcessor.h PluginEditor.h
                  Params.h Presets.h Matcher.h SynthEngine.h)
file(GLOB_RECURSE _found RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*PluginProcessor.cpp")
list(FILTER _found EXCLUDE REGEX "(^|/)(build[^/]*|_deps|JUCE[^/]*)/")
list(FILTER _found INCLUDE REGEX "(^|/)PluginProcessor\\.cpp$")
if(NOT _found)
  message(FATAL_ERROR "\nDust Box: can't find PluginProcessor.cpp.\n"
                      "Upload all of these files to the same place: ${DUSTBOX_FILES}\n")
endif()
list(GET _found 0 _first)
get_filename_component(DUSTBOX_SRC "${CMAKE_CURRENT_SOURCE_DIR}/${_first}" DIRECTORY)
set(_missing "")
foreach(f IN LISTS DUSTBOX_FILES)
  if(NOT EXISTS "${DUSTBOX_SRC}/${f}")
    list(APPEND _missing ${f})
  endif()
endforeach()
if(_missing)
  message(FATAL_ERROR "\nDust Box: found the code in ${DUSTBOX_SRC}\n"
                      "but these files are missing from that folder: ${_missing}\n"
                      "Upload them next to PluginProcessor.cpp and run the build again.\n")
endif()
message(STATUS "Dust Box: building from ${DUSTBOX_SRC}")
target_sources(DustBox PRIVATE "${DUSTBOX_SRC}/PluginProcessor.cpp" "${DUSTBOX_SRC}/PluginEditor.cpp")
target_include_directories(DustBox PRIVATE "${DUSTBOX_SRC}")

target_compile_definitions(DustBox PUBLIC
  JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0
  JUCE_DISPLAY_SPLASH_SCREEN=0 JUCE_REPORT_APP_USAGE=0)

target_link_libraries(DustBox
  PRIVATE juce::juce_audio_utils juce::juce_dsp
  PUBLIC  juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)

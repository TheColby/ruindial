cmake_minimum_required(VERSION 3.22)

project(RuinDial VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(DESTROYER_FETCH_JUCE "Download JUCE with FetchContent when it is not found" ON)
set(JUCE_SOURCE_DIR "" CACHE PATH "Path to a local JUCE checkout")

if(JUCE_SOURCE_DIR)
  add_subdirectory("${JUCE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE")
else()
  find_package(JUCE CONFIG QUIET)
  if(NOT JUCE_FOUND)
    if(NOT DESTROYER_FETCH_JUCE)
      message(FATAL_ERROR "JUCE was not found. Set JUCE_SOURCE_DIR or enable DESTROYER_FETCH_JUCE.")
    endif()

    include(FetchContent)
    FetchContent_Declare(
      JUCE
      GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
      GIT_TAG 8.0.6
    )
    FetchContent_MakeAvailable(JUCE)
  endif()
endif()

juce_add_plugin(RuinDial
  COMPANY_NAME "UglySoundGenerator Labs"
  BUNDLE_ID "dev.uglysoundgenerator.ruindial"
  PLUGIN_MANUFACTURER_CODE "UsgL"
  PLUGIN_CODE "RuDl"
  FORMATS AU VST3 Standalone
  PRODUCT_NAME "RuinDial"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(RuinDial)

target_sources(RuinDial
  PRIVATE
    Source/DestroyerDsp.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
)

target_compile_definitions(RuinDial
  PUBLIC
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

target_link_libraries(RuinDial
  PRIVATE
    juce::juce_audio_utils
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

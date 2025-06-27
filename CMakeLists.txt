cmake_minimum_required(VERSION 3.15)
project(DIY_AV_Entrainment_Fast VERSION 1.0 LANGUAGES CXX)

#--------------------------------------------------
# C++ standard
#--------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#--------------------------------------------------
# Paths
#--------------------------------------------------
# Root directory for your audio + UI code
set(AUDIO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp_audio)

#--------------------------------------------------
# Disable JUCE components we don’t need
#--------------------------------------------------
set(JUCE_BUILD_EXTRAS    OFF CACHE BOOL "Disable JUCE extras"   FORCE)
set(JUCE_BUILD_EXAMPLES  OFF CACHE BOOL "Disable JUCE examples" FORCE)
set(JUCE_BUILD_TESTS     OFF CACHE BOOL "Disable JUCE tests"    FORCE)
set(JUCE_BUILD_TOOLS     OFF CACHE BOOL "Disable JUCE tools"    FORCE)

#--------------------------------------------------
# Bring in JUCE as a subproject
#--------------------------------------------------
# Place JUCE’s build artifacts under the main build directory
add_subdirectory(
    "${AUDIO_DIR}/JUCE"
    "${CMAKE_BINARY_DIR}/juce-build"
    EXCLUDE_FROM_ALL
)#--------------------------------------------------
# 2) List all your .cpp source files
#--------------------------------------------------
set(SOURCES
    # Core
    ${AUDIO_DIR}/main.cpp
    ${AUDIO_DIR}/AudioUtils.cpp
    ${AUDIO_DIR}/Common.cpp
    ${AUDIO_DIR}/StepPreviewer.cpp
    ${AUDIO_DIR}/Track.cpp

    # Models
    ${AUDIO_DIR}/models/StepModel.cpp
    ${AUDIO_DIR}/models/VoiceModel.cpp

    # Presets
    ${AUDIO_DIR}/presets/NoiseParams.cpp
    ${AUDIO_DIR}/presets/VoicePreset.cpp

    # Synths
    ${AUDIO_DIR}/synths/BinauralBeat.cpp
    ${AUDIO_DIR}/synths/HybridQamMonauralBeat.cpp
    ${AUDIO_DIR}/synths/IsochronicTone.cpp
    ${AUDIO_DIR}/synths/MonauralBeatStereoAmps.cpp
    ${AUDIO_DIR}/synths/NoiseFlanger.cpp
    ${AUDIO_DIR}/synths/QamBeat.cpp
    ${AUDIO_DIR}/synths/RhythmicWaveshaping.cpp
    ${AUDIO_DIR}/synths/SpatialAngleModulation.cpp
    ${AUDIO_DIR}/synths/StereoAMIndependent.cpp
    ${AUDIO_DIR}/synths/Subliminals.cpp
    ${AUDIO_DIR}/synths/WaveShapeStereoAm.cpp

    # UI Components
    ${AUDIO_DIR}/ui/CollapsibleBox.cpp
    ${AUDIO_DIR}/ui/DefaultVoiceDialog.cpp
    ${AUDIO_DIR}/ui/FrequencyTesterDialog.cpp
    ${AUDIO_DIR}/ui/GlobalSettingsComponent.cpp
    ${AUDIO_DIR}/ui/NoiseGeneratorDialog.cpp
    ${AUDIO_DIR}/ui/OverlayClipDialog.cpp
    ${AUDIO_DIR}/ui/OverlayClipPanel.cpp
    ${AUDIO_DIR}/ui/PreferencesDialog.cpp
    ${AUDIO_DIR}/ui/Simulator.cpp
    ${AUDIO_DIR}/ui/StepConfigPanel.cpp
    ${AUDIO_DIR}/ui/StepListPanel.cpp
    ${AUDIO_DIR}/ui/StepPreviewComponent.cpp
    ${AUDIO_DIR}/ui/SubliminalDialog.cpp
    ${AUDIO_DIR}/ui/Themes.cpp
    ${AUDIO_DIR}/ui/VoiceEditorDialog.cpp
)

#--------------------------------------------------
# 3) Create the executable
#--------------------------------------------------
# Build a GUI executable. The WIN32 keyword hides the console window on
# Windows. On Linux it has no effect but is harmless, so we only use it
# when running on Windows.
if(WIN32)
    add_executable(AudioApp WIN32 ${SOURCES})
    target_compile_definitions(AudioApp PRIVATE PLATFORM_WINDOWS=1)
elseif(UNIX AND NOT APPLE)
    add_executable(AudioApp ${SOURCES})
    target_compile_definitions(AudioApp PRIVATE PLATFORM_LINUX=1)
else()
    add_executable(AudioApp ${SOURCES})
endif()

# Include headers from your code tree
target_include_directories(AudioApp
    PRIVATE
      ${AUDIO_DIR}
)

#--------------------------------------------------
# 4) Link against JUCE modules
#--------------------------------------------------
target_link_libraries(AudioApp
    PRIVATE
      juce::juce_core
      juce::juce_events
      juce::juce_audio_utils
                        juce::juce_audio_basics
                        juce::juce_audio_processors
                        juce::juce_audio_devices
                        juce::juce_gui_basics
                        juce::juce_dsp
                        juce::juce_events
      # add additional juce::<module> as needed
)

# Enable modal loops for JUCE dialogs so that runModal and runModalLoop
# are available for our UI components like OverlayClipDialog and
# PreferencesDialog.
# Enable modal loops so that the traditional synchronous FileChooser
# convenience methods (browseForFileToOpen, etc.) are available.
target_compile_definitions(AudioApp PRIVATE JUCE_MODAL_LOOPS_PERMITTED=1)

# (Optional) Set output directory for the binary
# set_target_properties(AudioApp PROPERTIES
#   RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
# )


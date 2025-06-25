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
# Disable JUCE extras/examples/tests
#--------------------------------------------------
set(JUCE_BUILD_EXTRAS    OFF CACHE BOOL "Disable JUCE extras" FORCE)
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "Disable JUCE examples" FORCE)
set(JUCE_BUILD_TESTS    OFF CACHE BOOL "Disable JUCE tests" FORCE)

#--------------------------------------------------
# 1) Bring in JUCE as a subproject
#--------------------------------------------------
add_subdirectory(${AUDIO_DIR}/JUCE juce-build)

#--------------------------------------------------
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
add_executable(AudioApp ${SOURCES})

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
			juce::juce_audio_device
			juce::juce_gui_basics
			juce::juce_dsp
			juce::juce_events
      # add additional juce::<module> as needed
)

# (Optional) Set output directory for the binary
# set_target_properties(AudioApp PROPERTIES
#   RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
# )



# ----------------------------------------
# 1) Bring in JUCE as a subproject
# ----------------------------------------
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/JUCE juce-build)

# ----------------------------------------
# 2) List all source files
# ----------------------------------------
set(SOURCES
    main.cpp
    AudioUtils.cpp
    Common.cpp
    StepPreviewer.cpp
    Track.cpp

    # Models
    models/StepModel.cpp
    models/VoiceModel.cpp

    # Presets
    presets/NoiseParams.cpp
    presets/VoicePreset.cpp

    # Synths
    synths/BinauralBeat.cpp
    synths/HybridQamMonauralBeat.cpp
    synths/IsochronicTone.cpp
    synths/MonauralBeatStereoAmps.cpp
    synths/NoiseFlanger.cpp
    synths/QamBeat.cpp
    synths/RhythmicWaveshaping.cpp
    synths/SpatialAngleModulation.cpp
    synths/StereoAMIndependent.cpp
    synths/Subliminals.cpp
    synths/WaveShapeStereoAm.cpp

    # UI Components
    ui/CollapsibleBox.cpp
    ui/DefaultVoiceDialog.cpp
    ui/FrequencyTesterDialog.cpp
    ui/GlobalSettingsComponent.cpp
    ui/NoiseGeneratorDialog.cpp
    ui/OverlayClipDialog.cpp
    ui/OverlayClipPanel.cpp
    ui/PreferencesDialog.cpp
    ui/Simulator.cpp
    ui/StepConfigPanel.cpp
    ui/StepListPanel.cpp
    ui/StepPreviewComponent.cpp
    ui/SubliminalDialog.cpp
    ui/Themes.cpp
    ui/VoiceEditorDialog.cpp
)

# ----------------------------------------
# 3) Define the executable target
# ----------------------------------------
add_executable(AudioApp ${SOURCES})

target_include_directories(AudioApp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# ----------------------------------------
# 4) Link JUCE modules
# ----------------------------------------
target_link_libraries(AudioApp
    PRIVATE
        juce::juce_core
                                juce::juce_audio_basics
                                juce::juce_audio_formats
                                juce::juce_audio_devices
        juce::juce_audio_utils
                                juce::juce_data_structures
                                juce::juce_gui_basics
                                juce::juce_gui_extra
                                juce::juce_dsp
                                juce::juce_events
        # add other juce::<module> targets as needed
)

# Ensure modal loop APIs like runModal and runModalLoop are available
# for our dialogs when building this standalone target.
target_compile_definitions(AudioApp PRIVATE JUCE_MODAL_LOOPS_PERMITTED=1)



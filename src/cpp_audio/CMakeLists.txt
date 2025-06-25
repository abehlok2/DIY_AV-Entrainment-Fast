cmake_minimum_required(VERSION 3.14)

project(DIY_AV_Audio_CPP)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/JUCE" juce-build)
add_executable(AudioApp
    main.cpp
    AudioUtils.cpp
    Common.cpp
    StepPreviewer.cpp
    Track.cpp
    BinauralBeat.cpp
    HybridQamMonauralBeat.cpp
    IsochronicTone.cpp
    MonauralBeatStereoAmps.cpp
    NoiseFlanger.cpp
    QamBeat.cpp
    RhythmicWaveshaping.cpp
    StereoAMIndependent.cpp
    Subliminals.cpp
    WaveShapeStereoAm.cpp
    NoiseParams.cpp
    VoicePreset.cpp
    StepModel.cpp
    VoiceModel.cpp
)

juce_add_console_app(diy_av_audio_cpp
    PRODUCT_NAME "DIY AV Audio CPP"
    BUNDLE_ID com.example.diyavcpp
    VERSION 0.1
    SOURCES
        main.cpp
        Track.h
        Track.cpp
        SynthFunctions.h
        synths/BinauralBeat.cpp
        synths/IsochronicTone.cpp
        synths/RhythmicWaveshaping.cpp
        synths/StereoAMIndependent.cpp
        synths/WaveShapeStereoAm.cpp
        synths/MonauralBeatStereoAmps.cpp
        synths/QamBeat.cpp
        synths/HybridQamMonauralBeat.cpp
        synths/SpatialAngleModulation.cpp
        synths/NoiseFlanger.cpp
        synths/Subliminals.cpp
        presets/VoicePreset.h
        presets/VoicePreset.cpp
        presets/NoiseParams.h
        presets/NoiseParams.cpp
        models/StepModel.h
        models/StepModel.cpp
        models/VoiceModel.h
        models/VoiceModel.cpp
        Common.h
        Common.cpp
        AudioUtils.h
        AudioUtils.cpp
        BufferAudioSource.h
        StepPreviewer.h
        StepPreviewer.cpp
)

juce_generate_juce_header(diy_av_audio_cpp)

target_link_libraries(diy_av_audio_cpp
    PRIVATE
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_events
        juce::juce_dsp
        juce::juce_audio_formats
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_osc
        juce::juce_graphics
)

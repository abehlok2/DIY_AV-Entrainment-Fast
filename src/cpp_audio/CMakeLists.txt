cmake_minimum_required(VERSION 3.14)

project(DIY_AV_Audio_CPP)

find_package(JUCE CONFIG REQUIRED)

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
)

juce_generate_juce_header(diy_av_audio_cpp)

target_link_libraries(diy_av_audio_cpp
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_audio_formats
)

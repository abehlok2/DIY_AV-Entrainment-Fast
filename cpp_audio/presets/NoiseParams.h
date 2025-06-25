#pragma once
#include <juce_core/juce_core.h>

constexpr const char* NOISE_FILE_EXTENSION = ".noise";

struct NoiseParams
{
    double durationSeconds{60.0};
    int sampleRate{44100};
    juce::String noiseType{"pink"};
    juce::String lfoWaveform{"sine"};
    bool transition{false};
    double lfoFreq{1.0 / 12.0};
    double startLfoFreq{1.0 / 12.0};
    double endLfoFreq{1.0 / 12.0};
    juce::var sweeps; // array of sweeps
    int startLfoPhaseOffsetDeg{0};
    int endLfoPhaseOffsetDeg{0};
    int startIntraPhaseOffsetDeg{0};
    int endIntraPhaseOffsetDeg{0};
    double initialOffset{0.0};
    double postOffset{0.0};
    juce::String inputAudioPath;
    double startTime{0.0};
    double fadeIn{0.0};
    double fadeOut{0.0};
    juce::var ampEnvelope; // array of envelope points
};

bool saveNoiseParams(const NoiseParams& params, const juce::File& file);
NoiseParams loadNoiseParams(const juce::File& file);

#pragma once
#include <juce_core/juce_core.h>

struct Preferences
{
    juce::String fontFamily;
    int fontSize { 10 };
    juce::String theme { "Dark" };
    juce::String exportDir;
    int sampleRate { 44100 };
    double testStepDuration { 30.0 };
    bool trackMetadata { false };
    double targetOutputAmplitude { 0.25 };
    juce::String crossfadeCurve { "linear" };
    juce::String amplitudeDisplayMode { "absolute" }; // or "dB"
    bool applyTargetAmplitude { true };
    juce::var defaultVoice;
};

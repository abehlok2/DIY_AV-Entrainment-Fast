#pragma once
#include <juce_core/juce_core.h>

constexpr const char* VOICE_FILE_EXTENSION = ".voice";

struct VoicePreset
{
    juce::String synthFunctionName;
    bool isTransition{false};
    juce::NamedValueSet params;
    juce::var volumeEnvelope; // may be undefined
    juce::String description;
};

bool saveVoicePreset(const VoicePreset& preset, const juce::File& file);
VoicePreset loadVoicePreset(const juce::File& file);

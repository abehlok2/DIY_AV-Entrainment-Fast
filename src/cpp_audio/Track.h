#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include <vector>
#include <string>

struct Voice
{
    std::string synthFunction;
    juce::NamedValueSet params;
    bool isTransition { false };
    juce::String description;
};

struct Step
{
    double durationSeconds { 0.0 };
    std::vector<Voice> voices;
    juce::String description;
};

struct GlobalSettings
{
    double sampleRate { 44100.0 };
    double crossfadeDuration { 1.0 };
    juce::String crossfadeCurve { "linear" };
};

struct Track
{
    GlobalSettings settings;
    std::vector<Step> steps;
};

Track loadTrackFromJson(const juce::File& file);
bool writeWavFile(const juce::File& file, const juce::AudioBuffer<float>& buffer, double sampleRate);


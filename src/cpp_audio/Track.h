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

struct BackgroundNoise
{
    juce::String filePath;
    double amp { 0.0 };
    double pan { 0.0 };
    double startTime { 0.0 };
    double fadeIn { 0.0 };
    double fadeOut { 0.0 };
    std::vector<std::pair<double, double>> ampEnvelope;
};

struct Clip
{
    juce::String filePath;
    juce::String description;
    double start { 0.0 };
    double duration { 0.0 };
    double amp { 1.0 };
    double pan { 0.0 };
    double fadeIn { 0.0 };
    double fadeOut { 0.0 };
};

struct Track
{
    GlobalSettings settings;
    BackgroundNoise backgroundNoise;
    std::vector<Clip> clips;
    std::vector<Step> steps;
};

Track loadTrackFromJson(const juce::File& file);
bool writeWavFile(const juce::File& file, const juce::AudioBuffer<float>& buffer, double sampleRate);
juce::AudioBuffer<float> assembleTrack(const Track& track);
/** Loads steps from a JSON file containing a top-level "steps" array and
    appends them to the provided vector.
    @return number of steps successfully loaded. */
int loadExternalStepsFromJson(const juce::File& file, std::vector<Step>& steps);


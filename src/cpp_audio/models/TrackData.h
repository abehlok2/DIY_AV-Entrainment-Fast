#pragma once

#include <juce_core/juce_core.h>
#include <vector>
#include <string>

/** Data structures representing a track configuration.
    These mirror the Python JSON schema used by the editor and
    offline generator to ensure full compatibility with track files.
*/

struct Voice
{
    std::string synthFunction;            // "synth_function_name" in JSON
    juce::NamedValueSet params;           // Parameter dictionary
    bool isTransition { false };          // "is_transition"
    juce::String description;             // Optional voice description
};

struct Step
{
    double durationSeconds { 0.0 };       // "duration"
    std::vector<Voice> voices;            // "voices"
    juce::String description;             // "description"
};

struct GlobalSettings
{
    double sampleRate { 44100.0 };        // "sample_rate"
    double crossfadeDuration { 1.0 };     // "crossfade_duration"
    juce::String crossfadeCurve { "linear" }; // "crossfade_curve"
    juce::String outputFilename { "my_track.wav" }; // "output_filename"
};

struct BackgroundNoise
{
    juce::String filePath;                // "file_path"
    double amp { 0.0 };                   // "amp"
    double pan { 0.0 };                   // "pan"
    double startTime { 0.0 };             // "start_time"
    double fadeIn { 0.0 };                // "fade_in"
    double fadeOut { 0.0 };               // "fade_out"
    std::vector<std::pair<double,double>> ampEnvelope; // "amp_envelope" array
};

struct Clip
{
    juce::String filePath;                // "file_path"
    juce::String description;             // "description"
    double start { 0.0 };                 // "start" / "start_time"
    double duration { 0.0 };              // "duration"
    double amp { 1.0 };                   // "amp" / "gain"
    double pan { 0.0 };                   // "pan"
    double fadeIn { 0.0 };                // "fade_in"
    double fadeOut { 0.0 };               // "fade_out"
};

struct Track
{
    GlobalSettings settings;
    BackgroundNoise backgroundNoise;
    std::vector<Clip> clips;
    std::vector<Step> steps;
};


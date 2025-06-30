#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>
#include <vector>
#include <string>
#include "../models/TrackData.h"


Track loadTrackFromJson(const juce::File& file);
/** Saves the given track structure to a JSON file. The file extension will
    be forced to ".json" if not already present.
    @return true on success. */
bool saveTrackToJson(const Track& track, const juce::File& file);
bool writeWavFile(const juce::File& file, const juce::AudioBuffer<float>& buffer, double sampleRate);
juce::AudioBuffer<float> assembleTrack(const Track& track);
/** Loads steps from a JSON file containing a top-level "steps" array and
    appends them to the provided vector.
    @return number of steps successfully loaded. */
int loadExternalStepsFromJson(const juce::File& file, std::vector<Step>& steps);

/** Returns a list of all available synth function names. */
std::vector<juce::String> getAvailableSynthNames();


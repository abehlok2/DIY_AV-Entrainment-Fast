#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

juce::AudioBuffer<float> qamBeat(double duration, double sampleRate,
                                 const juce::NamedValueSet& params);

juce::AudioBuffer<float> qamBeatTransition(double duration, double sampleRate,
                                           const juce::NamedValueSet& params);

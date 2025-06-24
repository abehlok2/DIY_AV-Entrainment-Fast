#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include "Track.h"

juce::AudioBuffer<float> binauralBeat(double duration, double sampleRate, const juce::NamedValueSet& params);
juce::AudioBuffer<float> isochronicTone(double duration, double sampleRate, const juce::NamedValueSet& params);


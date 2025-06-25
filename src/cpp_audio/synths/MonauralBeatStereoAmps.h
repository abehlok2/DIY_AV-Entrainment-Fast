#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

juce::AudioBuffer<float> monauralBeatStereoAmps(double duration, double sampleRate,
                                                const juce::NamedValueSet& params);

juce::AudioBuffer<float> monauralBeatStereoAmpsTransition(double duration, double sampleRate,
                                                         const juce::NamedValueSet& params);

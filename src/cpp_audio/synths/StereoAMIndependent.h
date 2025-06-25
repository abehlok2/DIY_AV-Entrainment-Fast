#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

juce::AudioBuffer<float> stereoAMIndependent(double duration, double sampleRate,
                                             const juce::NamedValueSet& params);

juce::AudioBuffer<float> stereoAMIndependentTransition(double duration, double sampleRate,
                                                      const juce::NamedValueSet& params);

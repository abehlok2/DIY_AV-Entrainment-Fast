#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

juce::AudioBuffer<float> waveShapeStereoAm(double duration, double sampleRate,
                                           const juce::NamedValueSet& params);

juce::AudioBuffer<float> waveShapeStereoAmTransition(double duration, double sampleRate,
                                                    const juce::NamedValueSet& params);

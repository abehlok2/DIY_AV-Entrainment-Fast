#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

juce::AudioBuffer<float> subliminalEncode(double duration, double sampleRate,
                                          const juce::NamedValueSet& params);

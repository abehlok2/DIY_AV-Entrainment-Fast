#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

juce::AudioBuffer<float> rhythmicWaveshaping(double duration, double sampleRate,
                                             const juce::NamedValueSet& params);

juce::AudioBuffer<float> rhythmicWaveshapingTransition(double duration, double sampleRate,
                                                      const juce::NamedValueSet& params);

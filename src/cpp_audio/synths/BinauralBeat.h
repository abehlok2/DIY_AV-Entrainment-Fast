#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

juce::AudioBuffer<float> binauralBeat(double duration, double sampleRate,
                                      const juce::NamedValueSet& params);

juce::AudioBuffer<float> binauralBeatTransition(double duration, double sampleRate,
                                                const juce::NamedValueSet& params);

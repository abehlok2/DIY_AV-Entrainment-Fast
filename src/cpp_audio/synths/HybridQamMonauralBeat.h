#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

juce::AudioBuffer<float> hybridQamMonauralBeat(double duration, double sampleRate,
                                               const juce::NamedValueSet& params);

juce::AudioBuffer<float> hybridQamMonauralBeatTransition(double duration, double sampleRate,
                                                        const juce::NamedValueSet& params);

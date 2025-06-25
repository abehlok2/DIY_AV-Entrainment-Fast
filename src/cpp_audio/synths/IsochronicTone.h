#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

juce::AudioBuffer<float> isochronicTone(double duration, double sampleRate,
                                        const juce::NamedValueSet& params);

juce::AudioBuffer<float> isochronicToneTransition(double duration, double sampleRate,
                                                  const juce::NamedValueSet& params);

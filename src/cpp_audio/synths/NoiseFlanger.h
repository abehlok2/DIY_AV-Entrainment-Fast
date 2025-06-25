#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

juce::AudioBuffer<float> generateSweptNotchPinkSound(double duration, double sampleRate,
                                                    const juce::NamedValueSet& params);

juce::AudioBuffer<float> generateSweptNotchPinkSoundTransition(double duration, double sampleRate,
                                                             const juce::NamedValueSet& params);

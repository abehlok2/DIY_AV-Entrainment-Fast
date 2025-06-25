#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

juce::AudioBuffer<float> spatialAngleModulation(double duration, double sampleRate,
                                               const juce::NamedValueSet& params);

juce::AudioBuffer<float> spatialAngleModulationTransition(double duration, double sampleRate,
                                                         const juce::NamedValueSet& params);

juce::AudioBuffer<float> spatialAngleModulationMonauralBeat(double duration, double sampleRate,
                                                           const juce::NamedValueSet& params);

juce::AudioBuffer<float> spatialAngleModulationMonauralBeatTransition(double duration, double sampleRate,
                                                                     const juce::NamedValueSet& params);

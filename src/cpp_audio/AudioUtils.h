#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

juce::AudioBuffer<float> generateSine(double freq, double amp, double duration, double sampleRate);
juce::AudioBuffer<float> crossfade(const juce::AudioBuffer<float>& a, const juce::AudioBuffer<float>& b, double duration, double sampleRate, juce::String curve);

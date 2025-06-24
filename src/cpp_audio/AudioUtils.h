#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <utility>
#include <vector>

juce::AudioBuffer<float> generateSine(double freq, double amp, double duration, double sampleRate);
juce::AudioBuffer<float> crossfade(const juce::AudioBuffer<float>& a,
                                   const juce::AudioBuffer<float>& b,
                                   double duration,
                                   double sampleRate,
                                   juce::String curve);

std::pair<float, float> getPanGains(double pan);
std::vector<double> calculateTransitionAlpha(double totalDuration,
                                             double sampleRate,
                                             double initialOffset = 0.0,
                                             double postOffset = 0.0,
                                             juce::String curve = "linear");

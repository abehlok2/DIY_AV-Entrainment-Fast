#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include "Track.h"

juce::AudioBuffer<float> binauralBeat(double duration, double sampleRate, const juce::NamedValueSet& params);
juce::AudioBuffer<float> binauralBeatTransition(double duration, double sampleRate, const juce::NamedValueSet& params);

juce::AudioBuffer<float> isochronicTone(double duration, double sampleRate, const juce::NamedValueSet& params);
juce::AudioBuffer<float> isochronicToneTransition(double duration, double sampleRate, const juce::NamedValueSet& params);

juce::AudioBuffer<float> rhythmicWaveshaping(double duration, double sampleRate, const juce::NamedValueSet& params);
juce::AudioBuffer<float> rhythmicWaveshapingTransition(double duration, double sampleRate, const juce::NamedValueSet& params);

juce::AudioBuffer<float> stereoAMIndependent(double duration, double sampleRate, const juce::NamedValueSet& params);
juce::AudioBuffer<float> stereoAMIndependentTransition(double duration, double sampleRate, const juce::NamedValueSet& params);

juce::AudioBuffer<float> waveShapeStereoAm(double duration, double sampleRate, const juce::NamedValueSet& params);
juce::AudioBuffer<float> waveShapeStereoAmTransition(double duration, double sampleRate, const juce::NamedValueSet& params);

juce::AudioBuffer<float> monauralBeatStereoAmps(double duration, double sampleRate, const juce::NamedValueSet& params);
juce::AudioBuffer<float> monauralBeatStereoAmpsTransition(double duration, double sampleRate, const juce::NamedValueSet& params);

juce::AudioBuffer<float> qamBeat(double duration, double sampleRate, const juce::NamedValueSet& params);
juce::AudioBuffer<float> qamBeatTransition(double duration, double sampleRate, const juce::NamedValueSet& params);

juce::AudioBuffer<float> hybridQamMonauralBeat(double duration, double sampleRate, const juce::NamedValueSet& params);
juce::AudioBuffer<float> hybridQamMonauralBeatTransition(double duration, double sampleRate, const juce::NamedValueSet& params);

juce::AudioBuffer<float> spatialAngleModulation(double duration, double sampleRate, const juce::NamedValueSet& params);
juce::AudioBuffer<float> spatialAngleModulationTransition(double duration, double sampleRate, const juce::NamedValueSet& params);
juce::AudioBuffer<float> spatialAngleModulationMonauralBeat(double duration, double sampleRate, const juce::NamedValueSet& params);
juce::AudioBuffer<float> spatialAngleModulationMonauralBeatTransition(double duration, double sampleRate, const juce::NamedValueSet& params);

juce::AudioBuffer<float> generateSweptNotchPinkSound(double duration, double sampleRate, const juce::NamedValueSet& params);
juce::AudioBuffer<float> generateSweptNotchPinkSoundTransition(double duration, double sampleRate, const juce::NamedValueSet& params);

juce::AudioBuffer<float> subliminalEncode(double duration, double sampleRate, const juce::NamedValueSet& params);

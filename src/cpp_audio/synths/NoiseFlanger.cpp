#include "SynthFunctions.h"
#include "AudioUtils.h"

using namespace juce;

AudioBuffer<float> generateSweptNotchPinkSound(double duration, double sampleRate, const NamedValueSet& params)
{
    // TODO: implement noise flanger effect
    AudioBuffer<float> buf(2, static_cast<int>(duration * sampleRate));
    buf.clear();
    return buf;
}

AudioBuffer<float> generateSweptNotchPinkSoundTransition(double duration, double sampleRate, const NamedValueSet& params)
{
    // TODO: implement transition version
    AudioBuffer<float> buf(2, static_cast<int>(duration * sampleRate));
    buf.clear();
    return buf;
}

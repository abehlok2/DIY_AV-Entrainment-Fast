#include "SynthFunctions.h"
#include "AudioUtils.h"

using namespace juce;

AudioBuffer<float> monauralBeatStereoAmps(double duration, double sampleRate, const NamedValueSet& params)
{
    // TODO: full implementation pending
    AudioBuffer<float> buf(2, static_cast<int>(duration * sampleRate));
    buf.clear();
    return buf;
}

AudioBuffer<float> monauralBeatStereoAmpsTransition(double duration, double sampleRate, const NamedValueSet& params)
{
    // TODO: full implementation pending
    AudioBuffer<float> buf(2, static_cast<int>(duration * sampleRate));
    buf.clear();
    return buf;
}

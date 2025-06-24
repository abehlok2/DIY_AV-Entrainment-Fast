#include "SynthFunctions.h"
#include "AudioUtils.h"

using namespace juce;

AudioBuffer<float> qamBeat(double duration, double sampleRate, const NamedValueSet& params)
{
    // TODO: implement QAM beat synthesis
    AudioBuffer<float> buf(2, static_cast<int>(duration * sampleRate));
    buf.clear();
    return buf;
}

AudioBuffer<float> qamBeatTransition(double duration, double sampleRate, const NamedValueSet& params)
{
    // TODO: implement transition variant
    AudioBuffer<float> buf(2, static_cast<int>(duration * sampleRate));
    buf.clear();
    return buf;
}

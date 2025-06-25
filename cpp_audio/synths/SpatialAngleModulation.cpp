#include "SynthFunctions.h"
#include "AudioUtils.h"

using namespace juce;

AudioBuffer<float> spatialAngleModulation(double duration, double sampleRate, const NamedValueSet& params)
{
    // TODO: implement SAM voice
    AudioBuffer<float> buf(2, static_cast<int>(duration * sampleRate));
    buf.clear();
    return buf;
}

AudioBuffer<float> spatialAngleModulationTransition(double duration, double sampleRate, const NamedValueSet& params)
{
    // TODO: implement SAM transition
    AudioBuffer<float> buf(2, static_cast<int>(duration * sampleRate));
    buf.clear();
    return buf;
}

AudioBuffer<float> spatialAngleModulationMonauralBeat(double duration, double sampleRate, const NamedValueSet& params)
{
    // TODO: implement SAM monaural beat
    AudioBuffer<float> buf(2, static_cast<int>(duration * sampleRate));
    buf.clear();
    return buf;
}

AudioBuffer<float> spatialAngleModulationMonauralBeatTransition(double duration, double sampleRate, const NamedValueSet& params)
{
    // TODO: implement SAM monaural beat transition
    AudioBuffer<float> buf(2, static_cast<int>(duration * sampleRate));
    buf.clear();
    return buf;
}

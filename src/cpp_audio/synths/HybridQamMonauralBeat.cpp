#include "SynthFunctions.h"
#include "AudioUtils.h"

using namespace juce;

AudioBuffer<float> hybridQamMonauralBeat(double duration, double sampleRate, const NamedValueSet& params)
{
    // TODO: implement hybrid qam monaural beat
    AudioBuffer<float> buf(2, static_cast<int>(duration * sampleRate));
    buf.clear();
    return buf;
}

AudioBuffer<float> hybridQamMonauralBeatTransition(double duration, double sampleRate, const NamedValueSet& params)
{
    // TODO: implement transition
    AudioBuffer<float> buf(2, static_cast<int>(duration * sampleRate));
    buf.clear();
    return buf;
}

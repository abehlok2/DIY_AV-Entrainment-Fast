#include "SynthFunctions.h"
#include "AudioUtils.h"

using namespace juce;

AudioBuffer<float> subliminalEncode(double duration, double sampleRate, const NamedValueSet& params)
{
    // TODO: implement subliminal encoding
    AudioBuffer<float> buf(2, static_cast<int>(duration * sampleRate));
    buf.clear();
    return buf;
}

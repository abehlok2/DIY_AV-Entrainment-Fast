#include "SynthFunctions.h"
#include "AudioUtils.h"
#include <cmath>

using namespace juce;

AudioBuffer<float> isochronicTone(double duration, double sampleRate, const NamedValueSet& params)
{
    double amp = params.getWithDefault("amp", 0.5);
    double baseFreq = params.getWithDefault("baseFreq", 200.0);
    double beatFreq = params.getWithDefault("beatFreq", 4.0);
    double rampPercent = params.getWithDefault("rampPercent", 0.2);
    double gapPercent = params.getWithDefault("gapPercent", 0.15);

    int totalSamples = static_cast<int>(duration * sampleRate);
    AudioBuffer<float> buffer(2, totalSamples);

    double phase = 0.0;
    double dt = 1.0 / sampleRate;
    for (int i = 0; i < totalSamples; ++i)
    {
        double cycle = 1.0 / beatFreq;
        double tInCycle = std::fmod(i * dt, cycle);
        double env = 1.0;
        double onTime = cycle * (1.0 - gapPercent);
        double rampTime = onTime * rampPercent;
        if (tInCycle > onTime)
            env = 0.0;
        else if (tInCycle < rampTime)
            env = tInCycle / rampTime;
        else if (tInCycle > (onTime - rampTime))
            env = (onTime - tInCycle) / rampTime;

        float sample = static_cast<float>(std::sin(phase) * amp * env);
        buffer.setSample(0, i, sample);
        buffer.setSample(1, i, sample);
        phase += MathConstants<double>::twoPi * baseFreq * dt;
    }
    return buffer;
}

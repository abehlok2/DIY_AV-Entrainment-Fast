#include "SynthFunctions.h"
#include "AudioUtils.h"
#include <cmath>

juce::AudioBuffer<float> binauralBeat(double duration, double sampleRate, const juce::NamedValueSet& params)
{
    double ampL = params.getWithDefault("ampL", 0.5);
    double ampR = params.getWithDefault("ampR", 0.5);
    double baseFreq = params.getWithDefault("baseFreq", 200.0);
    double beatFreq = params.getWithDefault("beatFreq", 4.0);

    auto left = generateSine(baseFreq - beatFreq/2.0, ampL, duration, sampleRate);
    auto right = generateSine(baseFreq + beatFreq/2.0, ampR, duration, sampleRate);

    juce::AudioBuffer<float> result(2, left.getNumSamples());
    result.copyFrom(0, 0, left, 0, 0, left.getNumSamples());
    result.copyFrom(1, 0, right, 1, 0, right.getNumSamples());
    return result;
}

juce::AudioBuffer<float> isochronicTone(double duration, double sampleRate, const juce::NamedValueSet& params)
{
    double amp = params.getWithDefault("amp", 0.5);
    double baseFreq = params.getWithDefault("baseFreq", 200.0);
    double beatFreq = params.getWithDefault("beatFreq", 4.0);
    double rampPercent = params.getWithDefault("rampPercent", 0.2);
    double gapPercent = params.getWithDefault("gapPercent", 0.15);

    int totalSamples = static_cast<int>(duration * sampleRate);
    juce::AudioBuffer<float> buffer(2, totalSamples);

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
        phase += juce::MathConstants<double>::twoPi * baseFreq * dt;
    }
    return buffer;
}


#include "AudioUtils.h"
#include <cmath>

juce::AudioBuffer<float> generateSine(double freq, double amp, double duration, double sampleRate)
{
    auto n = static_cast<int>(duration * sampleRate);
    juce::AudioBuffer<float> buffer(2, n);
    for (int ch = 0; ch < 2; ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        double phase = 0.0;
        double increment = juce::MathConstants<double>::twoPi * freq / sampleRate;
        for (int i = 0; i < n; ++i)
        {
            data[i] = static_cast<float>(std::sin(phase) * amp);
            phase += increment;
        }
    }
    return buffer;
}

juce::AudioBuffer<float> crossfade(const juce::AudioBuffer<float>& a, const juce::AudioBuffer<float>& b, double duration, double sampleRate, juce::String curve)
{
    int n = static_cast<int>(duration * sampleRate);
    n = std::min({ n, a.getNumSamples(), b.getNumSamples() });
    juce::AudioBuffer<float> result(2, n);

    for (int ch = 0; ch < 2; ++ch)
    {
        const float* pa = a.getReadPointer(ch);
        const float* pb = b.getReadPointer(ch);
        float* pr = result.getWritePointer(ch);
        for (int i = 0; i < n; ++i)
        {
            double alpha = static_cast<double>(i) / static_cast<double>(n);
            if (curve == "equal_power")
                alpha = std::sin(alpha * juce::MathConstants<double>::halfPi);
            pr[i] = pa[i] * (1.0 - alpha) + pb[i] * alpha;
        }
    }
    return result;
}


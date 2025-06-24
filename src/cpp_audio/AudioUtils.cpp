#include "AudioUtils.h"
#include <cmath>
#include <algorithm>

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

std::pair<float, float> getPanGains(double pan)
{
    pan = std::clamp(pan, -1.0, 1.0);
    double angle = (pan + 1.0) * juce::MathConstants<double>::pi / 4.0;
    float left = static_cast<float>(std::cos(angle));
    float right = static_cast<float>(std::sin(angle));
    return { left, right };
}

std::vector<double> calculateTransitionAlpha(double totalDuration,
                                             double sampleRate,
                                             double initialOffset,
                                             double postOffset,
                                             juce::String curve)
{
    int N = static_cast<int>(totalDuration * sampleRate);
    std::vector<double> alpha(N, 0.0);
    if (N <= 0)
        return alpha;

    double startT = std::min(initialOffset, totalDuration);
    double endT = std::max(startT, totalDuration - postOffset);
    double transTime = endT - startT;

    for (int i = 0; i < N; ++i)
    {
        double t = static_cast<double>(i) / sampleRate;
        double a = 0.0;
        if (transTime > 0.0)
        {
            a = (t - startT) / transTime;
            a = std::clamp(a, 0.0, 1.0);
        }

        if (curve == "logarithmic")
            a = 1.0 - std::pow(1.0 - a, 2.0);
        else if (curve == "exponential")
            a = std::pow(a, 2.0);

        alpha[i] = a;
    }
    return alpha;
}


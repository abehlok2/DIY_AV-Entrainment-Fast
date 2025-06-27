#include "IsochronicTone.h"
#include "../utils/AudioUtils.h"
#include <cmath>
#include <algorithm>

using namespace juce;

static double trapezoidEnv(double t, double cycleLen, double rampPercent, double gapPercent)
{
    if (cycleLen <= 0.0)
        return 0.0;

    double audibleLen = (1.0 - gapPercent) * cycleLen;
    double rampTotal = std::clamp(audibleLen * rampPercent * 2.0, 0.0, audibleLen);
    double rampUpLen = rampTotal * 0.5;
    double stableLen = audibleLen - rampTotal;
    double stableEnd = rampUpLen + stableLen;

    if (t >= audibleLen)
        return 0.0;
    if (t < rampUpLen)
        return rampUpLen > 0.0 ? t / rampUpLen : 0.0;
    if (t >= stableEnd)
        return rampUpLen > 0.0 ? (audibleLen - t) / rampUpLen : 0.0;
    return 1.0;
}

AudioBuffer<float> isochronicTone(double duration, double sampleRate, const NamedValueSet& params)
{
    int totalSamples = static_cast<int>(duration * sampleRate);
    AudioBuffer<float> buffer(2, std::max(0, totalSamples));
    if (totalSamples <= 0)
        return buffer;

    double amp        = params.getWithDefault("amp", 0.5);
    double baseFreq   = std::max(0.0, static_cast<double>(params.getWithDefault("baseFreq", 200.0)));
    double beatFreq   = std::max(0.0, static_cast<double>(params.getWithDefault("beatFreq", 4.0)));
    double rampPct    = params.getWithDefault("rampPercent", 0.2);
    double gapPct     = params.getWithDefault("gapPercent", 0.15);
    double pan        = params.getWithDefault("pan", 0.0);

    auto gains = getPanGains(pan);

    double dt = 1.0 / sampleRate;
    double phase = 0.0;
    double cycle = beatFreq > 0.0 ? 1.0 / beatFreq : 0.0;
    double onTime = cycle * (1.0 - gapPct);
    double rampTime = onTime * rampPct;

    for (int i = 0; i < totalSamples; ++i)
    {
        double tInCycle = beatFreq > 0.0 ? std::fmod(i * dt, cycle) : 0.0;
        double env = trapezoidEnv(tInCycle, cycle, rampPct, gapPct);

        float s = static_cast<float>(std::sin(phase) * amp * env);
        buffer.setSample(0, i, s * gains.first);
        buffer.setSample(1, i, s * gains.second);

        phase += MathConstants<double>::twoPi * baseFreq * dt;
    }
    return buffer;
}

AudioBuffer<float> isochronicToneTransition(double duration, double sampleRate, const NamedValueSet& params)
{
    int totalSamples = static_cast<int>(duration * sampleRate);
    AudioBuffer<float> buffer(2, std::max(0, totalSamples));
    if (totalSamples <= 0)
        return buffer;

    double amp          = params.getWithDefault("amp", 0.5);
    double startBaseF    = params.getWithDefault("startBaseFreq", params.getWithDefault("baseFreq", 200.0));
    double endBaseF      = params.getWithDefault("endBaseFreq", startBaseF);
    double startBeatF    = params.getWithDefault("startBeatFreq", params.getWithDefault("beatFreq", 4.0));
    double endBeatF      = params.getWithDefault("endBeatFreq", startBeatF);
    double rampPct       = params.getWithDefault("rampPercent", 0.2);
    double gapPct        = params.getWithDefault("gapPercent", 0.15);
    double pan           = params.getWithDefault("pan", 0.0);
    double initialOffset = params.getWithDefault("initial_offset", 0.0);
    double postOffset    = params.getWithDefault("post_offset", 0.0);
    String curve         = params.getWithDefault("transition_curve", "linear");

    auto alpha = calculateTransitionAlpha(duration, sampleRate, initialOffset, postOffset, curve);
    auto gains = getPanGains(pan);

    double dt = 1.0 / sampleRate;
    double phase = 0.0;
    double beatPhase = 0.0;

    for (int i = 0; i < totalSamples; ++i)
    {
        double a = alpha.empty() ? static_cast<double>(i) / (totalSamples > 1 ? totalSamples - 1 : 1)
                                 : alpha[i];

        double baseF = std::max(0.0, startBaseF + (endBaseF - startBaseF) * a);
        double beatF = std::max(0.0, startBeatF + (endBeatF - startBeatF) * a);
        double cycle = beatF > 0.0 ? 1.0 / beatF : 0.0;

        beatPhase += beatF * dt;
        double tInCycle = beatF > 0.0 ? std::fmod(beatPhase, 1.0) * cycle : 0.0;
        double env = trapezoidEnv(tInCycle, cycle, rampPct, gapPct);

        float s = static_cast<float>(std::sin(phase) * amp * env);
        buffer.setSample(0, i, s * gains.first);
        buffer.setSample(1, i, s * gains.second);

        phase += MathConstants<double>::twoPi * baseF * dt;
    }

    return buffer;
}

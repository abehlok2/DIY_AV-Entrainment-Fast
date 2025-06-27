#include "Common.h"
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <algorithm>
#include <numeric>
#include <random>
#include <array>

std::vector<double> sineWave(double freq, const std::vector<double>& t, double phase)
{
    std::vector<double> out(t.size(), 0.0);
    freq = std::max(freq, 1e-9);
    for (size_t i = 0; i < t.size(); ++i)
        out[i] = std::sin(2.0 * M_PI * freq * t[i] + phase);
    return out;
}

std::vector<double> sineWaveVarying(const std::vector<double>& freqArray,
                                    const std::vector<double>& t,
                                    double /*sampleRate*/)
{
    size_t N = std::min(freqArray.size(), t.size());
    std::vector<double> out(N, 0.0);
    if (N <= 1)
        return out;

    std::vector<double> phase(N, 0.0);
    for (size_t i = 1; i < N; ++i)
        phase[i] = phase[i-1] + 2.0 * M_PI * std::max(freqArray[i-1], 1e-9) * (t[i]-t[i-1]);

    double initial = 0.0;
    if (!t.empty())
        initial = 2.0 * M_PI * std::max(freqArray[0], 1e-9) * t[0];

    for (size_t i = 0; i < N; ++i)
        out[i] = std::sin(initial + phase[i]);
    return out;
}

std::vector<double> adsrEnvelope(const std::vector<double>& t,
                                 double attack,
                                 double decay,
                                 double sustainLevel,
                                 double release)
{
    size_t N = t.size();
    if (N <= 1)
        return std::vector<double>(N, 0.0);

    double duration = t.back() - t.front();
    if (N > 1)
        duration += (t[1]-t[0]);
    double sr = (duration > 0.0) ? static_cast<double>(N) / duration : 44100.0;

    attack = std::max(0.0, attack);
    decay = std::max(0.0, decay);
    release = std::max(0.0, release);
    sustainLevel = std::clamp(sustainLevel, 0.0, 1.0);

    size_t attackSamples = static_cast<size_t>(attack * sr);
    size_t decaySamples = static_cast<size_t>(decay * sr);
    size_t releaseSamples = static_cast<size_t>(release * sr);

    attackSamples = std::min(attackSamples, N);
    decaySamples = std::min(decaySamples, N);
    releaseSamples = std::min(releaseSamples, N);

    size_t totalADSR = attackSamples + decaySamples + releaseSamples;
    if (totalADSR > N && totalADSR > 0)
    {
        double scale = static_cast<double>(N) / static_cast<double>(totalADSR);
        attackSamples = static_cast<size_t>(attackSamples * scale);
        decaySamples = static_cast<size_t>(decaySamples * scale);
        releaseSamples = N - attackSamples - decaySamples;
    }

    size_t sustainSamples = N - (attackSamples + decaySamples + releaseSamples);

    std::vector<double> env;
    env.reserve(N);

    for (size_t i = 0; i < attackSamples; ++i)
        env.push_back(static_cast<double>(i) / std::max<size_t>(1, attackSamples));
    for (size_t i = 0; i < decaySamples; ++i)
    {
        double alpha = static_cast<double>(i) / std::max<size_t>(1, decaySamples);
        env.push_back(1.0 + alpha * (sustainLevel - 1.0));
    }
    for (size_t i = 0; i < sustainSamples; ++i)
        env.push_back(sustainLevel);
    for (size_t i = 0; i < releaseSamples; ++i)
    {
        double alpha = static_cast<double>(i) / std::max<size_t>(1, releaseSamples);
        env.push_back((1.0 - alpha) * sustainLevel);
    }

    env.resize(N, releaseSamples > 0 ? 0.0 : sustainLevel);
    return env;
}

std::vector<double> createLinearFadeEnvelope(double totalDuration,
                                             double sampleRate,
                                             double fadeDuration,
                                             double startAmp,
                                             double endAmp,
                                             const std::string& fadeType)
{
    int totalSamples = static_cast<int>(totalDuration * sampleRate);
    int fadeSamples  = std::min(static_cast<int>(fadeDuration * sampleRate), totalSamples);
    if (totalSamples <= 0)
        return {};

    std::vector<double> env(totalSamples, 1.0);
    if (fadeSamples <= 0)
    {
        std::fill(env.begin(), env.end(), fadeType == "in" ? endAmp : startAmp);
        return env;
    }

    if (fadeType == "in")
    {
        for (int i = 0; i < fadeSamples; ++i)
            env[i] = startAmp + (endAmp - startAmp) * (static_cast<double>(i) / static_cast<double>(fadeSamples));
        std::fill(env.begin() + fadeSamples, env.end(), endAmp);
    }
    else
    {
        int sustain = totalSamples - fadeSamples;
        std::fill(env.begin(), env.begin() + sustain, startAmp);
        for (int i = 0; i < fadeSamples; ++i)
            env[sustain + i] = startAmp + (endAmp - startAmp) * (static_cast<double>(i) / static_cast<double>(fadeSamples));
    }
    return env;
}

std::vector<double> linenEnvelope(const std::vector<double>& t,
                                  double attack,
                                  double release)
{
    size_t N = t.size();
    if (N <= 1)
        return std::vector<double>(N, 0.0);

    double duration = t.back() - t.front();
    if (N > 1)
        duration += (t[1]-t[0]);
    double sr = (duration > 0.0) ? static_cast<double>(N) / duration : 44100.0;

    attack = std::max(0.0, attack);
    release = std::max(0.0, release);

    size_t attackSamples = static_cast<size_t>(attack * sr);
    size_t releaseSamples = static_cast<size_t>(release * sr);
    attackSamples = std::min(attackSamples, N);
    releaseSamples = std::min(releaseSamples, N);

    size_t totalAR = attackSamples + releaseSamples;
    if (totalAR > N && totalAR > 0)
    {
        double scale = static_cast<double>(N) / static_cast<double>(totalAR);
        attackSamples = static_cast<size_t>(attackSamples * scale);
        releaseSamples = N - attackSamples;
    }

    size_t sustainSamples = N - (attackSamples + releaseSamples);
    std::vector<double> env;
    env.reserve(N);

    for (size_t i = 0; i < attackSamples; ++i)
        env.push_back(static_cast<double>(i) / std::max<size_t>(1, attackSamples));
    for (size_t i = 0; i < sustainSamples; ++i)
        env.push_back(1.0);
    for (size_t i = 0; i < releaseSamples; ++i)
        env.push_back(1.0 - static_cast<double>(i) / std::max<size_t>(1, releaseSamples));

    env.resize(N, releaseSamples > 0 ? 0.0 : 1.0);
    return env;
}

std::pair<std::vector<double>, std::vector<double>> pan2(const std::vector<double>& signal,
                                                         double pan)
{
    pan = std::clamp(pan, -1.0, 1.0);
    double angle = (pan + 1.0) * M_PI / 4.0;
    double leftGain = std::cos(angle);
    double rightGain = std::sin(angle);

    std::vector<double> left(signal.size());
    std::vector<double> right(signal.size());
    for (size_t i = 0; i < signal.size(); ++i)
    {
        left[i] = signal[i] * leftGain;
        right[i] = signal[i] * rightGain;
    }
    return {left, right};
}

static std::vector<double> applyIIR(const std::vector<double>& data,
                                    double center,
                                    double Q,
                                    double fs,
                                    bool bandpass)
{
    if (data.empty() || fs <= 0.0)
        return data;

    std::vector<double> out(data.size(), 0.0);

    double omega = 2.0 * M_PI * center / fs;
    double alpha = std::sin(omega) / (2.0 * Q);
    double cosw = std::cos(omega);

    double b0, b1, b2, a0, a1, a2;
    if (bandpass)
    {
        b0 = alpha;
        b1 = 0.0;
        b2 = -alpha;
    }
    else
    {
        b0 = 1.0;
        b1 = -2.0 * cosw;
        b2 = 1.0;
    }
    a0 = 1.0 + alpha;
    a1 = -2.0 * cosw;
    a2 = 1.0 - alpha;

    b0 /= a0; b1 /= a0; b2 /= a0; a1 /= a0; a2 /= a0;

    double x1 = 0.0, x2 = 0.0;
    double y1 = 0.0, y2 = 0.0;
    for (size_t i = 0; i < data.size(); ++i)
    {
        double x0 = data[i];
        double y0 = b0*x0 + b1*x1 + b2*x2 - a1*y1 - a2*y2;
        out[i] = y0;
        x2 = x1; x1 = x0; y2 = y1; y1 = y0;
    }
    return out;
}

std::vector<double> bandpassFilter(const std::vector<double>& data,
                                   double center,
                                   double Q,
                                   double fs)
{
    center = std::max(center, 1e-6);
    Q = std::max(Q, 0.1);
    return applyIIR(data, center, Q, fs, true);
}

std::vector<double> bandrejectFilter(const std::vector<double>& data,
                                     double center,
                                     double Q,
                                     double fs)
{
    center = std::max(center, 1e-6);
    Q = std::max(Q, 0.1);
    return applyIIR(data, center, Q, fs, false);
}

std::vector<double> lowpassFilter(const std::vector<double>& data,
                                  double cutoff,
                                  double fs)
{
    if (data.empty() || fs <= 0.0)
        return data;

    double omega = 2.0 * M_PI * cutoff / fs;
    double cosw = std::cos(omega);
    double sinw = std::sin(omega);
    double alpha = sinw / std::sqrt(2.0);

    double b0 = (1.0 - cosw)/2.0;
    double b1 = 1.0 - cosw;
    double b2 = (1.0 - cosw)/2.0;
    double a0 = 1.0 + alpha;
    double a1 = -2.0*cosw;
    double a2 = 1.0 - alpha;

    b0 /= a0; b1 /= a0; b2 /= a0; a1 /= a0; a2 /= a0;

    std::vector<double> out(data.size(), 0.0);
    double x1 = 0.0, x2 = 0.0;
    double y1 = 0.0, y2 = 0.0;
    for (size_t i = 0; i < data.size(); ++i)
    {
        double x0 = data[i];
        double y0 = b0*x0 + b1*x1 + b2*x2 - a1*y1 - a2*y2;
        out[i] = y0;
        x2 = x1; x1 = x0; y2 = y1; y1 = y0;
    }
    return out;
}

std::vector<double> pinkNoise(int n)
{
    if (n <= 0)
        return {};
    std::vector<double> out(n, 0.0);
    std::array<double, 7> b{};
    std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    for (int i = 0; i < n; ++i)
    {
        double white = dist(rng);
        b[0] = 0.99886 * b[0] + white * 0.0555179;
        b[1] = 0.99332 * b[1] + white * 0.0750759;
        b[2] = 0.96900 * b[2] + white * 0.1538520;
        b[3] = 0.86650 * b[3] + white * 0.3104856;
        b[4] = 0.55000 * b[4] + white * 0.5329522;
        b[5] = -0.7616 * b[5] - white * 0.0168980;
        out[i] = b[0] + b[1] + b[2] + b[3] + b[4] + b[5] + b[6] + white * 0.5362;
        out[i] *= 0.11; // normalization factor
        b[6] = white * 0.115926;
    }
    return out;
}

std::vector<double> brownNoise(int n)
{
    if (n <= 0)
        return {};
    std::vector<double> out(n, 0.0);
    std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    double sum = 0.0;
    for (int i = 0; i < n; ++i)
    {
        sum += dist(rng);
        out[i] = sum;
    }
    double maxAbs = 0.0;
    for (double v : out)
        maxAbs = std::max(maxAbs, std::abs(v));
    if (maxAbs > 1e-9)
        for (double& v : out) v /= maxAbs;
    return out;
}

std::vector<double> trapezoidEnvelopeVectorized(const std::vector<double>& tInCycle,
                                                const std::vector<double>& cycleLen,
                                                const std::vector<double>& rampPercent,
                                                const std::vector<double>& gapPercent)
{
    size_t N = tInCycle.size();
    std::vector<double> env(N, 0.0);
    for (size_t i = 0; i < N; ++i)
    {
        if (cycleLen[i] <= 0.0)
        {
            env[i] = 0.0;
            continue;
        }
        double audibleLen = (1.0 - gapPercent[i]) * cycleLen[i];
        double rampTotal = std::clamp(audibleLen * rampPercent[i] * 2.0, 0.0, audibleLen);
        double rampUpLen = rampTotal * 0.5;
        double stableLen = audibleLen - rampTotal;
        double stableEnd = rampUpLen + stableLen;
        double pos = tInCycle[i];
        if (pos >= audibleLen)
            env[i] = 0.0;
        else if (pos < rampUpLen && rampUpLen > 0.0)
            env[i] = pos / rampUpLen;
        else if (pos >= stableEnd && rampUpLen > 0.0)
            env[i] = 1.0 - (pos - stableEnd) / rampUpLen;
        else
            env[i] = 1.0;
        env[i] = std::clamp(env[i], 0.0, 1.0);
    }
    return env;
}

std::vector<double> applyFilters(const std::vector<double>& signalSegment,
                                 double fs)
{
    if (signalSegment.empty())
        return signalSegment;
    if (fs <= 0.0)
        return signalSegment;

    double hpCutoff = 30.0;
    double lpCutoff = fs * 0.5 * 0.9;

    std::vector<double> result = signalSegment;
    if (hpCutoff > 0.0 && hpCutoff < fs * 0.5)
        result = bandpassFilter(result, hpCutoff, hpCutoff / 2.0, fs); // simple high-pass using bandpass implementation
    if (lpCutoff > 0.0 && lpCutoff < fs * 0.5)
        result = lowpassFilter(result, lpCutoff, fs);
    return result;
}


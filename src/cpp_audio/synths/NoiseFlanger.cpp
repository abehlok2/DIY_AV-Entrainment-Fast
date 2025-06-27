#include "NoiseFlanger.h"
#include "AudioUtils.h"
#include "../core/VarUtils.h"
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <algorithm>
#include <cmath>

// The implementation here is a simplified C++ version of the Python
// ``noise_flanger`` module.  It generates pink or brown noise and applies one
// or more dynamically sweeping notch filters driven by an LFO.  The goal is not
// to perfectly reproduce every feature of the Python code but to provide the
// same overall effect for the C++ tool chain.

using namespace juce;

namespace
{
//------------------------------------------------------------------------------
// Helper utilities

static std::vector<float> generatePinkNoise(int n)
{
    Random rng;
    std::vector<float> out(n);

    float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f, b3 = 0.0f, b4 = 0.0f, b5 = 0.0f;
    for (int i = 0; i < n; ++i)
    {
        float w = rng.nextFloat() * 2.0f - 1.0f;
        b0 = 0.99886f * b0 + w * 0.0555179f;
        b1 = 0.99332f * b1 + w * 0.0750759f;
        b2 = 0.96900f * b2 + w * 0.1538520f;
        b3 = 0.86650f * b3 + w * 0.3104856f;
        b4 = 0.55000f * b4 + w * 0.5329522f;
        b5 = -0.7616f * b5 - w * 0.0168980f;

        out[i] = (b0 + b1 + b2 + b3 + b4 + b5 + w * 0.5362f) * 0.11f;
    }
    return out;
}

static std::vector<float> generateBrownNoise(int n)
{
    Random rng;
    std::vector<float> out(n);
    float accum = 0.0f;
    for (int i = 0; i < n; ++i)
    {
        accum += rng.nextFloat() * 2.0f - 1.0f;
        out[i] = accum;
    }

    float maxAbs = 1e-9f;
    for (float v : out)
        maxAbs = std::max(maxAbs, std::abs(v));
    for (float& v : out)
        v /= maxAbs;
    return out;
}

static float computeRms(const std::vector<float>& data)
{
    double sum = 0.0;
    for (float v : data)
        sum += static_cast<double>(v) * static_cast<double>(v);
    return static_cast<float>(std::sqrt(sum / std::max<size_t>(1, data.size())));
}

static std::vector<float> applyNotchSweep(const std::vector<float>& in,
                                          double sampleRate,
                                          double lfoFreq,
                                          double phaseOffset,
                                          double intraPhaseOffset,
                                          double minFreq,
                                          double maxFreq,
                                          double q,
                                          int    cascades,
                                          String lfoWaveform)
{
    int n = static_cast<int>(in.size());
    std::vector<float> out(n);

    std::vector<dsp::IIR::Filter<float>> filters(cascades);
    for (auto& f : filters)
        f.reset();

    auto triangle = [](double phase)
    {
        double frac = phase / MathConstants<double>::twoPi;
        frac = frac - std::floor(frac);
        return 2.0 * std::abs(2.0 * frac - 1.0) - 1.0;
    };

    double phase = phaseOffset;
    double phase2 = phaseOffset + intraPhaseOffset;
    double dt = 1.0 / sampleRate;

    for (int i = 0; i < n; ++i)
    {
        double lfo = (lfoWaveform == "triangle") ? triangle(phase)
                                                   : std::cos(phase);
        double lfo2 = (lfoWaveform == "triangle") ? triangle(phase2)
                                                   : std::cos(phase2);

        double freq1 = minFreq + (maxFreq - minFreq) * (lfo + 1.0) * 0.5;
        double freq2 = minFreq + (maxFreq - minFreq) * (lfo2 + 1.0) * 0.5;

        auto coeff1 = dsp::IIR::Coefficients<float>::makeNotch(sampleRate, freq1, q);
        auto coeff2 = dsp::IIR::Coefficients<float>::makeNotch(sampleRate, freq2, q);

        float sample = in[i];
        for (auto& f : filters)
        {
            f.coefficients = coeff1;
            sample = f.processSample(sample);
        }
        for (auto& f : filters)
        {
            f.coefficients = coeff2;
            sample = f.processSample(sample);
        }

        out[i] = sample;

        phase += MathConstants<double>::twoPi * lfoFreq * dt;
        phase2 += MathConstants<double>::twoPi * lfoFreq * dt;
    }

    return out;
}
} // namespace

//------------------------------------------------------------------------------

AudioBuffer<float> generateSweptNotchPinkSound(double duration,
                                               double sampleRate,
                                               const NamedValueSet& params)
{
    int totalSamples = static_cast<int>(duration * sampleRate);
    AudioBuffer<float> buffer(2, totalSamples);
    if (totalSamples <= 0)
        return buffer;

    double lfoFreq = params.getWithDefault("lfo_freq", 1.0 / 12.0);
    double notchQ  = params.getWithDefault("notch_q", 25.0);
    int    casc    = static_cast<int>(params.getWithDefault("cascade_count", 10));
    double phaseOffsetDeg = params.getWithDefault("lfo_phase_offset_deg", 90.0);
    double intraPhaseDeg  = params.getWithDefault("intra_phase_offset_deg", 0.0);
    String noiseType      = params.getWithDefault("noise_type", "pink").toString();
    String lfoWave        = params.getWithDefault("lfo_waveform", "sine").toString();

    // Parse filter_sweeps parameter (only first entry is used in this simplified
    // implementation).  Expect [[min, max], ...] or a dictionary with keys
    // start_min/start_max.
    double minFreq = 1000.0;
    double maxFreq = 10000.0;
    if (params.contains("filter_sweeps"))
    {
        if (auto* arr = params["filter_sweeps"].getArray())
        {
            if (! arr->isEmpty())
            {
                auto e = (*arr)[0];
                if (auto* pair = e.getArray())
                {
                    if (pair->size() >= 2)
                    {
                        minFreq = pair->getUnchecked(0);
                        maxFreq = pair->getUnchecked(1);
                    }
                }
                else if (auto* obj = e.getDynamicObject())
                {
                    minFreq = getPropertyWithDefault(obj, "start_min", getPropertyWithDefault(obj, "min", minFreq));
                    maxFreq = getPropertyWithDefault(obj, "start_max", getPropertyWithDefault(obj, "max", maxFreq));
                }
            }
        }
    }

    std::vector<float> noise = (noiseType == "brown") ? generateBrownNoise(totalSamples)
                                                       : generatePinkNoise(totalSamples);

    // Pre-filter for warmth / HPF to roughly match python processing
    dsp::IIR::Filter<float> lowPass;
    dsp::IIR::Filter<float> highPass;
    lowPass.coefficients  = dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 10000.0);
    highPass.coefficients = dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 50.0);

    for (float& s : noise)
        s = highPass.processSample(lowPass.processSample(s));

    float rmsIn = computeRms(noise);
    if (rmsIn < 1e-8f)
        rmsIn = 1e-8f;

    double phaseOffsetRad  = MathConstants<double>::pi * phaseOffsetDeg / 180.0;
    double intraPhaseRad   = MathConstants<double>::pi * intraPhaseDeg / 180.0;

    auto left  = applyNotchSweep(noise, sampleRate, lfoFreq, 0.0, intraPhaseRad, minFreq, maxFreq, notchQ, casc, lfoWave);
    auto right = applyNotchSweep(noise, sampleRate, lfoFreq, phaseOffsetRad, intraPhaseRad, minFreq, maxFreq, notchQ, casc, lfoWave);

    float rmsL = computeRms(left);
    float rmsR = computeRms(right);
    if (rmsL > 1e-8f)
    {
        float g = rmsIn / rmsL;
        for (auto& s : left)
            s *= g;
    }
    if (rmsR > 1e-8f)
    {
        float g = rmsIn / rmsR;
        for (auto& s : right)
            s *= g;
    }

    float maxAbs = 0.0f;
    for (int i = 0; i < totalSamples; ++i)
        maxAbs = std::max({ maxAbs, std::abs(left[i]), std::abs(right[i]) });
    if (maxAbs > 0.95f)
    {
        float g = 0.95f / maxAbs;
        for (auto& s : left)
            s *= g;
        for (auto& s : right)
            s *= g;
    }

    buffer.clear();
    for (int i = 0; i < totalSamples; ++i)
    {
        buffer.setSample(0, i, left[i]);
        buffer.setSample(1, i, right[i]);
    }

    return buffer;
}

//------------------------------------------------------------------------------

AudioBuffer<float> generateSweptNotchPinkSoundTransition(double duration,
                                                         double sampleRate,
                                                         const NamedValueSet& params)
{
    int totalSamples = static_cast<int>(duration * sampleRate);
    AudioBuffer<float> buffer(2, totalSamples);
    if (totalSamples <= 0)
        return buffer;

    double startFreq = params.getWithDefault("start_lfo_freq", 1.0 / 12.0);
    double endFreq   = params.getWithDefault("end_lfo_freq", 1.0 / 12.0);
    double startQ    = params.getWithDefault("start_notch_q", 25.0);
    double endQ      = params.getWithDefault("end_notch_q", 25.0);
    int    startCasc = static_cast<int>(params.getWithDefault("start_cascade_count", 10));
    int    endCasc   = static_cast<int>(params.getWithDefault("end_cascade_count", 10));
    double startPhaseDeg = params.getWithDefault("start_lfo_phase_offset_deg", 90.0);
    double endPhaseDeg   = params.getWithDefault("end_lfo_phase_offset_deg", 90.0);
    double startIntraDeg = params.getWithDefault("start_intra_phase_offset_deg", 0.0);
    double endIntraDeg   = params.getWithDefault("end_intra_phase_offset_deg", 0.0);
    String noiseType     = params.getWithDefault("noise_type", "pink").toString();
    String lfoWave       = params.getWithDefault("lfo_waveform", "sine").toString();
    double initialOffset = params.getWithDefault("initial_offset", 0.0);
    double postOffset    = params.getWithDefault("post_offset", 0.0);
    String curve         = params.getWithDefault("transition_curve", "linear").toString();

    double minFreqStart = 1000.0, maxFreqStart = 10000.0;
    double minFreqEnd   = 1000.0, maxFreqEnd   = 10000.0;

    if (params.contains("start_filter_sweeps"))
    {
        if (auto* arr = params["start_filter_sweeps"].getArray())
        {
            if (! arr->isEmpty())
            {
                auto e = (*arr)[0];
                if (auto* pair = e.getArray())
                {
                    if (pair->size() >= 2)
                    {
                        minFreqStart = pair->getUnchecked(0);
                        maxFreqStart = pair->getUnchecked(1);
                    }
                }
                else if (auto* obj = e.getDynamicObject())
                {
                    minFreqStart = getPropertyWithDefault(obj, "start_min", getPropertyWithDefault(obj, "min", minFreqStart));
                    maxFreqStart = getPropertyWithDefault(obj, "start_max", getPropertyWithDefault(obj, "max", maxFreqStart));
                }
            }
        }
    }

    if (params.contains("end_filter_sweeps"))
    {
        if (auto* arr = params["end_filter_sweeps"].getArray())
        {
            if (! arr->isEmpty())
            {
                auto e = (*arr)[0];
                if (auto* pair = e.getArray())
                {
                    if (pair->size() >= 2)
                    {
                        minFreqEnd = pair->getUnchecked(0);
                        maxFreqEnd = pair->getUnchecked(1);
                    }
                }
                else if (auto* obj = e.getDynamicObject())
                {
                    minFreqEnd = getPropertyWithDefault(obj, "end_min", getPropertyWithDefault(obj, "min", minFreqEnd));
                    maxFreqEnd = getPropertyWithDefault(obj, "end_max", getPropertyWithDefault(obj, "max", maxFreqEnd));
                }
            }
        }
    }

    std::vector<float> noise = (noiseType == "brown") ? generateBrownNoise(totalSamples)
                                                       : generatePinkNoise(totalSamples);

    dsp::IIR::Filter<float> lowPass;
    dsp::IIR::Filter<float> highPass;
    lowPass.coefficients  = dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 10000.0);
    highPass.coefficients = dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 50.0);

    for (float& s : noise)
        s = highPass.processSample(lowPass.processSample(s));

    float rmsIn = computeRms(noise);
    if (rmsIn < 1e-8f)
        rmsIn = 1e-8f;

    auto alpha = calculateTransitionAlpha(duration, sampleRate, initialOffset, postOffset, curve);
    if (alpha.size() != static_cast<size_t>(totalSamples))
    {
        alpha.resize(totalSamples, alpha.empty() ? 0.0 : alpha.back());
    }

    std::vector<float> left(totalSamples);
    std::vector<float> right(totalSamples);

    std::vector<dsp::IIR::Filter<float>> leftFilters(std::max(startCasc, endCasc));
    std::vector<dsp::IIR::Filter<float>> rightFilters(std::max(startCasc, endCasc));
    for (auto& f : leftFilters)
        f.reset();
    for (auto& f : rightFilters)
        f.reset();

    double phaseL = 0.0;
    double phaseR = MathConstants<double>::pi * startPhaseDeg / 180.0;
    double phaseL2 = MathConstants<double>::pi * startIntraDeg / 180.0;
    double phaseR2 = MathConstants<double>::pi * (startPhaseDeg + startIntraDeg) / 180.0;

    for (int i = 0; i < totalSamples; ++i)
    {
        double a = alpha.empty() ? static_cast<double>(i) / (totalSamples > 1 ? totalSamples - 1 : 1)
                                 : alpha[(size_t) i];

        double curFreq  = startFreq + (endFreq - startFreq) * a;
        double curMin   = minFreqStart + (minFreqEnd - minFreqStart) * a;
        double curMax   = maxFreqStart + (maxFreqEnd - maxFreqStart) * a;
        double curQ     = startQ + (endQ - startQ) * a;
        int    curCasc  = static_cast<int>(std::round(startCasc + (endCasc - startCasc) * a));
        curCasc        = std::max(1, curCasc);

        double lfoL  = (lfoWave == "triangle") ? (2.0 * std::abs(2.0 * std::fmod(phaseL / MathConstants<double>::twoPi, 1.0) - 1.0) - 1.0)
                                               : std::cos(phaseL);
        double lfoR  = (lfoWave == "triangle") ? (2.0 * std::abs(2.0 * std::fmod(phaseR / MathConstants<double>::twoPi, 1.0) - 1.0) - 1.0)
                                               : std::cos(phaseR);
        double lfoL2 = (lfoWave == "triangle") ? (2.0 * std::abs(2.0 * std::fmod(phaseL2 / MathConstants<double>::twoPi, 1.0) - 1.0) - 1.0)
                                               : std::cos(phaseL2);
        double lfoR2 = (lfoWave == "triangle") ? (2.0 * std::abs(2.0 * std::fmod(phaseR2 / MathConstants<double>::twoPi, 1.0) - 1.0) - 1.0)
                                               : std::cos(phaseR2);

        double freqL  = curMin + (curMax - curMin) * (lfoL + 1.0) * 0.5;
        double freqR  = curMin + (curMax - curMin) * (lfoR + 1.0) * 0.5;
        double freqL2 = curMin + (curMax - curMin) * (lfoL2 + 1.0) * 0.5;
        double freqR2 = curMin + (curMax - curMin) * (lfoR2 + 1.0) * 0.5;

        auto coeffL  = dsp::IIR::Coefficients<float>::makeNotch(sampleRate, freqL, curQ);
        auto coeffR  = dsp::IIR::Coefficients<float>::makeNotch(sampleRate, freqR, curQ);
        auto coeffL2 = dsp::IIR::Coefficients<float>::makeNotch(sampleRate, freqL2, curQ);
        auto coeffR2 = dsp::IIR::Coefficients<float>::makeNotch(sampleRate, freqR2, curQ);

        float sampleL = noise[i];
        float sampleR = noise[i];

        for (int c = 0; c < curCasc; ++c)
        {
            leftFilters[c].coefficients = coeffL;
            sampleL = leftFilters[c].processSample(sampleL);
            rightFilters[c].coefficients = coeffR;
            sampleR = rightFilters[c].processSample(sampleR);
        }
        for (int c = 0; c < curCasc; ++c)
        {
            leftFilters[c].coefficients = coeffL2;
            sampleL = leftFilters[c].processSample(sampleL);
            rightFilters[c].coefficients = coeffR2;
            sampleR = rightFilters[c].processSample(sampleR);
        }

        left[i]  = sampleL;
        right[i] = sampleR;

        double inc = MathConstants<double>::twoPi * curFreq / sampleRate;
        phaseL += inc;
        phaseR += inc;
        phaseL2 += inc;
        phaseR2 += inc;
    }

    float rmsL = computeRms(left);
    float rmsR = computeRms(right);
    if (rmsL > 1e-8f)
    {
        float g = rmsIn / rmsL;
        for (auto& s : left)
            s *= g;
    }
    if (rmsR > 1e-8f)
    {
        float g = rmsIn / rmsR;
        for (auto& s : right)
            s *= g;
    }

    float maxAbs = 0.0f;
    for (int i = 0; i < totalSamples; ++i)
        maxAbs = std::max({ maxAbs, std::abs(left[i]), std::abs(right[i]) });
    if (maxAbs > 0.95f)
    {
        float g = 0.95f / maxAbs;
        for (auto& s : left)
            s *= g;
        for (auto& s : right)
            s *= g;
    }

    buffer.clear();
    for (int i = 0; i < totalSamples; ++i)
    {
        buffer.setSample(0, i, left[i]);
        buffer.setSample(1, i, right[i]);
    }

    return buffer;
}

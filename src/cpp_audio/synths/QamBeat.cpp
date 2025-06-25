#include "QamBeat.h"
#include "AudioUtils.h"

using namespace juce;

// -----------------------------------------------------------------------------
// Basic QAM Beat synthesis. This is a C++ counterpart to the more elaborate
// Python implementation found in src/audio/synth_functions/qam_beat.py.  The
// goal is functional parity with the existing Python version so the C++ build
// can be used as a drop-in replacement.  The code intentionally mirrors the
// structure of BinauralBeat.cpp for consistency.
// -----------------------------------------------------------------------------

static inline double shapedCos(double phase, double shape)
{
    double c = std::cos(phase);
    if (shape == 1.0)
        return c;
    double sign = (c >= 0.0) ? 1.0 : -1.0;
    return sign * std::pow(std::abs(c), 1.0 / std::max(1e-6, shape));
}

AudioBuffer<float> qamBeat(double duration, double sampleRate, const NamedValueSet& params)
{
    int N = static_cast<int>(duration * sampleRate);
    AudioBuffer<float> buffer(2, std::max(0, N));
    if (N <= 0)
        return buffer;

    // Carrier/amp parameters
    double ampL = params.getWithDefault("ampL", 0.5);
    double ampR = params.getWithDefault("ampR", 0.5);
    double baseFreqL = params.getWithDefault("baseFreqL", 200.0);
    double baseFreqR = params.getWithDefault("baseFreqR", 204.0);

    // Primary QAM modulation
    double qamAmFreqL = params.getWithDefault("qamAmFreqL", 4.0);
    double qamAmDepthL = params.getWithDefault("qamAmDepthL", 0.5);
    double qamAmPhaseOffsetL = params.getWithDefault("qamAmPhaseOffsetL", 0.0);
    double qamAmFreqR = params.getWithDefault("qamAmFreqR", 4.0);
    double qamAmDepthR = params.getWithDefault("qamAmDepthR", 0.5);
    double qamAmPhaseOffsetR = params.getWithDefault("qamAmPhaseOffsetR", 0.0);

    // Secondary modulation
    double qamAm2FreqL = params.getWithDefault("qamAm2FreqL", 0.0);
    double qamAm2DepthL = params.getWithDefault("qamAm2DepthL", 0.0);
    double qamAm2PhaseOffsetL = params.getWithDefault("qamAm2PhaseOffsetL", 0.0);
    double qamAm2FreqR = params.getWithDefault("qamAm2FreqR", 0.0);
    double qamAm2DepthR = params.getWithDefault("qamAm2DepthR", 0.0);
    double qamAm2PhaseOffsetR = params.getWithDefault("qamAm2PhaseOffsetR", 0.0);

    // Modulation shape and coupling
    double modShapeL = params.getWithDefault("modShapeL", 1.0);
    double modShapeR = params.getWithDefault("modShapeR", 1.0);
    double crossModDepth = params.getWithDefault("crossModDepth", 0.0);
    double crossModDelay = params.getWithDefault("crossModDelay", 0.0);

    // Harmonics / sidebands
    double harmonicDepth = params.getWithDefault("harmonicDepth", 0.0);
    double harmonicRatio = params.getWithDefault("harmonicRatio", 2.0);
    double subHarmonicFreq = params.getWithDefault("subHarmonicFreq", 0.0);
    double subHarmonicDepth = params.getWithDefault("subHarmonicDepth", 0.0);

    // Phase and additional modulation
    double startPhaseL = params.getWithDefault("startPhaseL", 0.0);
    double startPhaseR = params.getWithDefault("startPhaseR", 0.0);
    double phaseOscFreq = params.getWithDefault("phaseOscFreq", 0.0);
    double phaseOscRange = params.getWithDefault("phaseOscRange", 0.0);
    double phaseOscPhaseOffset = params.getWithDefault("phaseOscPhaseOffset", 0.0);

    bool beatingSidebands = params.getWithDefault("beatingSidebands", false);
    double sidebandOffset = params.getWithDefault("sidebandOffset", 1.0);
    double sidebandDepth = params.getWithDefault("sidebandDepth", 0.1);
    double attackTime = params.getWithDefault("attackTime", 0.0);
    double releaseTime = params.getWithDefault("releaseTime", 0.0);

    std::vector<double> t(N);
    std::vector<double> envL(N, 1.0), envR(N, 1.0);
    std::vector<double> phaseL(N), phaseR(N);

    double dt = duration / static_cast<double>(N);
    for (int i = 0; i < N; ++i)
        t[i] = i * dt;

    // Primary modulation
    for (int i = 0; i < N; ++i)
    {
        if (qamAmFreqL != 0.0 && qamAmDepthL != 0.0)
        {
            double ph = 2.0 * MathConstants<double>::pi * qamAmFreqL * t[i] + qamAmPhaseOffsetL;
            envL[i] *= 1.0 + qamAmDepthL * shapedCos(ph, modShapeL);
        }
        if (qamAmFreqR != 0.0 && qamAmDepthR != 0.0)
        {
            double ph = 2.0 * MathConstants<double>::pi * qamAmFreqR * t[i] + qamAmPhaseOffsetR;
            envR[i] *= 1.0 + qamAmDepthR * shapedCos(ph, modShapeR);
        }
    }

    // Secondary modulation
    if (qamAm2FreqL != 0.0 && qamAm2DepthL != 0.0)
        for (int i = 0; i < N; ++i)
        {
            double ph = 2.0 * MathConstants<double>::pi * qamAm2FreqL * t[i] + qamAm2PhaseOffsetL;
            envL[i] *= 1.0 + qamAm2DepthL * std::cos(ph);
        }

    if (qamAm2FreqR != 0.0 && qamAm2DepthR != 0.0)
        for (int i = 0; i < N; ++i)
        {
            double ph = 2.0 * MathConstants<double>::pi * qamAm2FreqR * t[i] + qamAm2PhaseOffsetR;
            envR[i] *= 1.0 + qamAm2DepthR * std::cos(ph);
        }

    // Cross-channel coupling
    if (crossModDepth != 0.0 && crossModDelay > 0.0)
    {
        std::vector<double> envLCopy = envL;
        std::vector<double> envRCopy = envR;
        int delaySamp = static_cast<int>(crossModDelay * sampleRate);
        for (int i = delaySamp; i < N; ++i)
        {
            envL[i] *= 1.0 + crossModDepth * (envRCopy[i - delaySamp] - 1.0);
            envR[i] *= 1.0 + crossModDepth * (envLCopy[i - delaySamp] - 1.0);
        }
    }

    // Sub-harmonic modulation
    if (subHarmonicFreq != 0.0 && subHarmonicDepth != 0.0)
        for (int i = 0; i < N; ++i)
        {
            double subMod = std::cos(2.0 * MathConstants<double>::pi * subHarmonicFreq * t[i]);
            double m = 1.0 + subHarmonicDepth * subMod;
            envL[i] *= m;
            envR[i] *= m;
        }

    // Carrier phases
    double curL = startPhaseL;
    double curR = startPhaseR;
    for (int i = 0; i < N; ++i)
    {
        phaseL[i] = curL;
        phaseR[i] = curR;
        curL += MathConstants<double>::twoPi * baseFreqL * dt;
        curR += MathConstants<double>::twoPi * baseFreqR * dt;
    }

    // Phase oscillation
    if (phaseOscFreq != 0.0 || phaseOscRange != 0.0)
    {
        for (int i = 0; i < N; ++i)
        {
            double dphi = (phaseOscRange * 0.5) *
                           std::sin(2.0 * MathConstants<double>::pi * phaseOscFreq * t[i] +
                                    phaseOscPhaseOffset);
            phaseL[i] -= dphi;
            phaseR[i] += dphi;
        }
    }

    // Generate output
    for (int i = 0; i < N; ++i)
    {
        double time = t[i];
        double envMul = 1.0;
        if (attackTime > 0.0 && time < attackTime)
            envMul *= time / attackTime;
        if (releaseTime > 0.0 && time > (duration - releaseTime))
            envMul *= (duration - time) / releaseTime;

        double sigL = envL[i] * std::cos(phaseL[i]);
        double sigR = envR[i] * std::cos(phaseR[i]);

        if (harmonicDepth != 0.0)
        {
            sigL += harmonicDepth * envL[i] * std::cos(harmonicRatio * phaseL[i]);
            sigR += harmonicDepth * envR[i] * std::cos(harmonicRatio * phaseR[i]);
        }

        if (beatingSidebands && sidebandDepth != 0.0)
        {
            double side = 2.0 * MathConstants<double>::pi * sidebandOffset * time;
            sigL += sidebandDepth * envL[i] * std::cos(phaseL[i] - side);
            sigR += sidebandDepth * envR[i] * std::cos(phaseR[i] - side);
            sigL += sidebandDepth * envL[i] * std::cos(phaseL[i] + side);
            sigR += sidebandDepth * envR[i] * std::cos(phaseR[i] + side);
        }

        buffer.setSample(0, i, static_cast<float>(sigL * ampL * envMul));
        buffer.setSample(1, i, static_cast<float>(sigR * ampR * envMul));
    }

    return buffer;
}

// -----------------------------------------------------------------------------
// Parameter transitioning version of qamBeat. Parameters that have a "start"
// and "end" prefix will be interpolated over the duration of the segment. This
// mirrors the behaviour of qam_beat_transition in the Python implementation.
// -----------------------------------------------------------------------------

AudioBuffer<float> qamBeatTransition(double duration, double sampleRate, const NamedValueSet& params)
{
    int N = static_cast<int>(duration * sampleRate);
    AudioBuffer<float> buffer(2, std::max(0, N));
    if (N <= 0)
        return buffer;

    // Start/end parameters
    double startAmpL = params.getWithDefault("startAmpL", params.getWithDefault("ampL", 0.5));
    double endAmpL   = params.getWithDefault("endAmpL", startAmpL);
    double startAmpR = params.getWithDefault("startAmpR", params.getWithDefault("ampR", 0.5));
    double endAmpR   = params.getWithDefault("endAmpR", startAmpR);

    double startBaseFreqL = params.getWithDefault("startBaseFreqL", params.getWithDefault("baseFreqL", 200.0));
    double endBaseFreqL   = params.getWithDefault("endBaseFreqL", startBaseFreqL);
    double startBaseFreqR = params.getWithDefault("startBaseFreqR", params.getWithDefault("baseFreqR", 204.0));
    double endBaseFreqR   = params.getWithDefault("endBaseFreqR", startBaseFreqR);

    double startQamAmFreqL = params.getWithDefault("startQamAmFreqL", params.getWithDefault("qamAmFreqL", 4.0));
    double endQamAmFreqL   = params.getWithDefault("endQamAmFreqL", startQamAmFreqL);
    double startQamAmDepthL = params.getWithDefault("startQamAmDepthL", params.getWithDefault("qamAmDepthL", 0.5));
    double endQamAmDepthL   = params.getWithDefault("endQamAmDepthL", startQamAmDepthL);
    double startQamAmPhaseOffsetL = params.getWithDefault("startQamAmPhaseOffsetL", params.getWithDefault("qamAmPhaseOffsetL", 0.0));
    double endQamAmPhaseOffsetL   = params.getWithDefault("endQamAmPhaseOffsetL", startQamAmPhaseOffsetL);

    double startQamAmFreqR = params.getWithDefault("startQamAmFreqR", params.getWithDefault("qamAmFreqR", 4.0));
    double endQamAmFreqR   = params.getWithDefault("endQamAmFreqR", startQamAmFreqR);
    double startQamAmDepthR = params.getWithDefault("startQamAmDepthR", params.getWithDefault("qamAmDepthR", 0.5));
    double endQamAmDepthR   = params.getWithDefault("endQamAmDepthR", startQamAmDepthR);
    double startQamAmPhaseOffsetR = params.getWithDefault("startQamAmPhaseOffsetR", params.getWithDefault("qamAmPhaseOffsetR", 0.0));
    double endQamAmPhaseOffsetR   = params.getWithDefault("endQamAmPhaseOffsetR", startQamAmPhaseOffsetR);

    double startQamAm2FreqL = params.getWithDefault("startQamAm2FreqL", params.getWithDefault("qamAm2FreqL", 0.0));
    double endQamAm2FreqL   = params.getWithDefault("endQamAm2FreqL", startQamAm2FreqL);
    double startQamAm2DepthL = params.getWithDefault("startQamAm2DepthL", params.getWithDefault("qamAm2DepthL", 0.0));
    double endQamAm2DepthL   = params.getWithDefault("endQamAm2DepthL", startQamAm2DepthL);
    double startQamAm2PhaseOffsetL = params.getWithDefault("startQamAm2PhaseOffsetL", params.getWithDefault("qamAm2PhaseOffsetL", 0.0));
    double endQamAm2PhaseOffsetL   = params.getWithDefault("endQamAm2PhaseOffsetL", startQamAm2PhaseOffsetL);

    double startQamAm2FreqR = params.getWithDefault("startQamAm2FreqR", params.getWithDefault("qamAm2FreqR", 0.0));
    double endQamAm2FreqR   = params.getWithDefault("endQamAm2FreqR", startQamAm2FreqR);
    double startQamAm2DepthR = params.getWithDefault("startQamAm2DepthR", params.getWithDefault("qamAm2DepthR", 0.0));
    double endQamAm2DepthR   = params.getWithDefault("endQamAm2DepthR", startQamAm2DepthR);
    double startQamAm2PhaseOffsetR = params.getWithDefault("startQamAm2PhaseOffsetR", params.getWithDefault("qamAm2PhaseOffsetR", 0.0));
    double endQamAm2PhaseOffsetR   = params.getWithDefault("endQamAm2PhaseOffsetR", startQamAm2PhaseOffsetR);

    double startModShapeL = params.getWithDefault("startModShapeL", params.getWithDefault("modShapeL", 1.0));
    double endModShapeL   = params.getWithDefault("endModShapeL", startModShapeL);
    double startModShapeR = params.getWithDefault("startModShapeR", params.getWithDefault("modShapeR", 1.0));
    double endModShapeR   = params.getWithDefault("endModShapeR", startModShapeR);

    double startCrossModDepth = params.getWithDefault("startCrossModDepth", params.getWithDefault("crossModDepth", 0.0));
    double endCrossModDepth   = params.getWithDefault("endCrossModDepth", startCrossModDepth);

    double startHarmonicDepth = params.getWithDefault("startHarmonicDepth", params.getWithDefault("harmonicDepth", 0.0));
    double endHarmonicDepth   = params.getWithDefault("endHarmonicDepth", startHarmonicDepth);

    double startSubHarmonicFreq = params.getWithDefault("startSubHarmonicFreq", params.getWithDefault("subHarmonicFreq", 0.0));
    double endSubHarmonicFreq   = params.getWithDefault("endSubHarmonicFreq", startSubHarmonicFreq);
    double startSubHarmonicDepth = params.getWithDefault("startSubHarmonicDepth", params.getWithDefault("subHarmonicDepth", 0.0));
    double endSubHarmonicDepth   = params.getWithDefault("endSubHarmonicDepth", startSubHarmonicDepth);

    double startPhaseOscFreq = params.getWithDefault("startPhaseOscFreq", params.getWithDefault("phaseOscFreq", 0.0));
    double endPhaseOscFreq   = params.getWithDefault("endPhaseOscFreq", startPhaseOscFreq);
    double startPhaseOscRange = params.getWithDefault("startPhaseOscRange", params.getWithDefault("phaseOscRange", 0.0));
    double endPhaseOscRange   = params.getWithDefault("endPhaseOscRange", startPhaseOscRange);

    double startStartPhaseL = params.getWithDefault("startStartPhaseL", params.getWithDefault("startPhaseL", 0.0));
    double endStartPhaseL   = params.getWithDefault("endStartPhaseL", startStartPhaseL);
    double startStartPhaseR = params.getWithDefault("startStartPhaseR", params.getWithDefault("startPhaseR", 0.0));
    double endStartPhaseR   = params.getWithDefault("endStartPhaseR", startStartPhaseR);

    // Static parameters
    double crossModDelay = params.getWithDefault("crossModDelay", 0.0);
    double harmonicRatio = params.getWithDefault("harmonicRatio", 2.0);
    double phaseOscPhaseOffset = params.getWithDefault("phaseOscPhaseOffset", 0.0);
    bool beatingSidebands = params.getWithDefault("beatingSidebands", false);
    double sidebandOffset = params.getWithDefault("sidebandOffset", 1.0);
    double sidebandDepth = params.getWithDefault("sidebandDepth", 0.1);
    double attackTime = params.getWithDefault("attackTime", 0.0);
    double releaseTime = params.getWithDefault("releaseTime", 0.0);

    double initialOffset = params.getWithDefault("initial_offset", 0.0);
    double postOffset = params.getWithDefault("post_offset", 0.0);
    String curve = params.getWithDefault("transition_curve", "linear");

    auto alpha = calculateTransitionAlpha(duration, sampleRate, initialOffset, postOffset, curve);

    std::vector<double> t(N);
    for (int i = 0; i < N; ++i)
        t[i] = i * (duration / static_cast<double>(N));

    // Parameter arrays
    std::vector<double> ampLArr(N), ampRArr(N);
    std::vector<double> baseLArr(N), baseRArr(N);
    std::vector<double> amFreqLArr(N), amDepthLArr(N), amPhaseOffsetLArr(N);
    std::vector<double> amFreqRArr(N), amDepthRArr(N), amPhaseOffsetRArr(N);
    std::vector<double> am2FreqLArr(N), am2DepthLArr(N), am2PhaseOffsetLArr(N);
    std::vector<double> am2FreqRArr(N), am2DepthRArr(N), am2PhaseOffsetRArr(N);
    std::vector<double> modShapeLArr(N), modShapeRArr(N);
    std::vector<double> crossDepthArr(N);
    std::vector<double> harmonicDepthArr(N);
    std::vector<double> subFreqArr(N), subDepthArr(N);
    std::vector<double> phaseOscFreqArr(N), phaseOscRangeArr(N);

    for (int i = 0; i < N; ++i)
    {
        double a = alpha.empty() ? static_cast<double>(i) / (N > 1 ? N - 1 : 1) : alpha[i];

        ampLArr[i] = startAmpL + (endAmpL - startAmpL) * a;
        ampRArr[i] = startAmpR + (endAmpR - startAmpR) * a;
        baseLArr[i] = startBaseFreqL + (endBaseFreqL - startBaseFreqL) * a;
        baseRArr[i] = startBaseFreqR + (endBaseFreqR - startBaseFreqR) * a;

        amFreqLArr[i] = startQamAmFreqL + (endQamAmFreqL - startQamAmFreqL) * a;
        amDepthLArr[i] = startQamAmDepthL + (endQamAmDepthL - startQamAmDepthL) * a;
        amPhaseOffsetLArr[i] = startQamAmPhaseOffsetL + (endQamAmPhaseOffsetL - startQamAmPhaseOffsetL) * a;
        amFreqRArr[i] = startQamAmFreqR + (endQamAmFreqR - startQamAmFreqR) * a;
        amDepthRArr[i] = startQamAmDepthR + (endQamAmDepthR - startQamAmDepthR) * a;
        amPhaseOffsetRArr[i] = startQamAmPhaseOffsetR + (endQamAmPhaseOffsetR - startQamAmPhaseOffsetR) * a;

        am2FreqLArr[i] = startQamAm2FreqL + (endQamAm2FreqL - startQamAm2FreqL) * a;
        am2DepthLArr[i] = startQamAm2DepthL + (endQamAm2DepthL - startQamAm2DepthL) * a;
        am2PhaseOffsetLArr[i] = startQamAm2PhaseOffsetL + (endQamAm2PhaseOffsetL - startQamAm2PhaseOffsetL) * a;
        am2FreqRArr[i] = startQamAm2FreqR + (endQamAm2FreqR - startQamAm2FreqR) * a;
        am2DepthRArr[i] = startQamAm2DepthR + (endQamAm2DepthR - startQamAm2DepthR) * a;
        am2PhaseOffsetRArr[i] = startQamAm2PhaseOffsetR + (endQamAm2PhaseOffsetR - startQamAm2PhaseOffsetR) * a;

        modShapeLArr[i] = startModShapeL + (endModShapeL - startModShapeL) * a;
        modShapeRArr[i] = startModShapeR + (endModShapeR - startModShapeR) * a;
        crossDepthArr[i] = startCrossModDepth + (endCrossModDepth - startCrossModDepth) * a;
        harmonicDepthArr[i] = startHarmonicDepth + (endHarmonicDepth - startHarmonicDepth) * a;
        subFreqArr[i] = startSubHarmonicFreq + (endSubHarmonicFreq - startSubHarmonicFreq) * a;
        subDepthArr[i] = startSubHarmonicDepth + (endSubHarmonicDepth - startSubHarmonicDepth) * a;
        phaseOscFreqArr[i] = startPhaseOscFreq + (endPhaseOscFreq - startPhaseOscFreq) * a;
        phaseOscRangeArr[i] = startPhaseOscRange + (endPhaseOscRange - startPhaseOscRange) * a;
    }

    std::vector<double> envL(N, 1.0), envR(N, 1.0);

    for (int i = 0; i < N; ++i)
    {
        if (amFreqLArr[i] != 0.0 && amDepthLArr[i] != 0.0)
        {
            double ph = 2.0 * MathConstants<double>::pi * amFreqLArr[i] * t[i] + amPhaseOffsetLArr[i];
            envL[i] *= 1.0 + amDepthLArr[i] * shapedCos(ph, modShapeLArr[i]);
        }
        if (amFreqRArr[i] != 0.0 && amDepthRArr[i] != 0.0)
        {
            double ph = 2.0 * MathConstants<double>::pi * amFreqRArr[i] * t[i] + amPhaseOffsetRArr[i];
            envR[i] *= 1.0 + amDepthRArr[i] * shapedCos(ph, modShapeRArr[i]);
        }

        if (am2FreqLArr[i] != 0.0 && am2DepthLArr[i] != 0.0)
        {
            double ph = 2.0 * MathConstants<double>::pi * am2FreqLArr[i] * t[i] + am2PhaseOffsetLArr[i];
            envL[i] *= 1.0 + am2DepthLArr[i] * std::cos(ph);
        }
        if (am2FreqRArr[i] != 0.0 && am2DepthRArr[i] != 0.0)
        {
            double ph = 2.0 * MathConstants<double>::pi * am2FreqRArr[i] * t[i] + am2PhaseOffsetRArr[i];
            envR[i] *= 1.0 + am2DepthRArr[i] * std::cos(ph);
        }

        if (subFreqArr[i] != 0.0 && subDepthArr[i] != 0.0)
        {
            double ph = 2.0 * MathConstants<double>::pi * subFreqArr[i] * t[i];
            double m = 1.0 + subDepthArr[i] * std::cos(ph);
            envL[i] *= m;
            envR[i] *= m;
        }
    }

    if (crossModDelay > 0.0)
    {
        std::vector<double> preL = envL;
        std::vector<double> preR = envR;
        int delaySamp = static_cast<int>(crossModDelay * sampleRate);
        for (int i = delaySamp; i < N; ++i)
        {
            if (crossDepthArr[i] != 0.0)
            {
                envL[i] *= 1.0 + crossDepthArr[i] * (preR[i - delaySamp] - 1.0);
                envR[i] *= 1.0 + crossDepthArr[i] * (preL[i - delaySamp] - 1.0);
            }
        }
    }

    std::vector<double> phaseL(N), phaseR(N);
    double curL = startStartPhaseL;
    double curR = startStartPhaseR;
    double dt = duration / static_cast<double>(N);
    for (int i = 0; i < N; ++i)
    {
        phaseL[i] = curL;
        phaseR[i] = curR;
        curL += MathConstants<double>::twoPi * baseLArr[i] * dt;
        curR += MathConstants<double>::twoPi * baseRArr[i] * dt;
    }

    for (int i = 0; i < N; ++i)
    {
        if (phaseOscFreqArr[i] != 0.0 || phaseOscRangeArr[i] != 0.0)
        {
            double dphi = (phaseOscRangeArr[i] * 0.5) *
                           std::sin(2.0 * MathConstants<double>::pi * phaseOscFreqArr[i] * t[i] +
                                    phaseOscPhaseOffset);
            phaseL[i] -= dphi;
            phaseR[i] += dphi;
        }
    }

    for (int i = 0; i < N; ++i)
    {
        double time = t[i];
        double envMul = 1.0;
        if (attackTime > 0.0 && time < attackTime)
            envMul *= time / attackTime;
        if (releaseTime > 0.0 && time > (duration - releaseTime))
            envMul *= (duration - time) / releaseTime;

        double sigL = envL[i] * std::cos(phaseL[i]);
        double sigR = envR[i] * std::cos(phaseR[i]);

        if (harmonicDepthArr[i] != 0.0)
        {
            sigL += harmonicDepthArr[i] * envL[i] * std::cos(harmonicRatio * phaseL[i]);
            sigR += harmonicDepthArr[i] * envR[i] * std::cos(harmonicRatio * phaseR[i]);
        }

        if (beatingSidebands && sidebandDepth != 0.0)
        {
            double side = 2.0 * MathConstants<double>::pi * sidebandOffset * time;
            sigL += sidebandDepth * envL[i] * std::cos(phaseL[i] - side);
            sigR += sidebandDepth * envR[i] * std::cos(phaseR[i] - side);
            sigL += sidebandDepth * envL[i] * std::cos(phaseL[i] + side);
            sigR += sidebandDepth * envR[i] * std::cos(phaseR[i] + side);
        }

        buffer.setSample(0, i, static_cast<float>(sigL * ampLArr[i] * envMul));
        buffer.setSample(1, i, static_cast<float>(sigR * ampRArr[i] * envMul));
    }

    return buffer;
}


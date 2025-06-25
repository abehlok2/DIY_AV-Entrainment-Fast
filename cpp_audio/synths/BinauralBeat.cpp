#include "SynthFunctions.h"
#include "AudioUtils.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>
#include <juce_dsp/juce_dsp.h>

using namespace juce;

AudioBuffer<float> binauralBeat(double duration, double sampleRate, const NamedValueSet& params)
{
    int N = static_cast<int>(duration * sampleRate);
    AudioBuffer<float> buffer(2, std::max(0, N));
    if (N <= 0)
        return buffer;

    double ampL  = params.getWithDefault("ampL", 0.5);
    double ampR  = params.getWithDefault("ampR", 0.5);
    double baseF = params.getWithDefault("baseFreq", 200.0);
    double beatF = params.getWithDefault("beatFreq", 4.0);
    bool   forceMono = params.getWithDefault("forceMono", false);
    double startL = params.getWithDefault("startPhaseL", 0.0);
    double startR = params.getWithDefault("startPhaseR", 0.0);
    double aODL  = params.getWithDefault("ampOscDepthL", 0.0);
    double aOFL  = params.getWithDefault("ampOscFreqL", 0.0);
    double aODR  = params.getWithDefault("ampOscDepthR", 0.0);
    double aOFR  = params.getWithDefault("ampOscFreqR", 0.0);
    double fORL  = params.getWithDefault("freqOscRangeL", 0.0);
    double fOFL  = params.getWithDefault("freqOscFreqL", 0.0);
    double fORR  = params.getWithDefault("freqOscRangeR", 0.0);
    double fOFR  = params.getWithDefault("freqOscFreqR", 0.0);
    double ampOscPhaseOffsetL = params.getWithDefault("ampOscPhaseOffsetL", 0.0);
    double ampOscPhaseOffsetR = params.getWithDefault("ampOscPhaseOffsetR", 0.0);
    double pOF  = params.getWithDefault("phaseOscFreq", 0.0);
    double pOR  = params.getWithDefault("phaseOscRange", 0.0);

    double glitchInterval   = params.getWithDefault("glitchInterval", 0.0);
    double glitchDur        = params.getWithDefault("glitchDur", 0.0);
    double glitchNoiseLevel = params.getWithDefault("glitchNoiseLevel", 0.0);
    double glitchFocusWidth = params.getWithDefault("glitchFocusWidth", 0.0);
    double glitchFocusExp   = params.getWithDefault("glitchFocusExp", 0.0);

    std::vector<double> t(N);
    std::vector<double> instL(N), instR(N);
    std::vector<double> phaseL(N), phaseR(N);
    std::vector<double> envL(N), envR(N);

    double dt = duration / static_cast<double>(N);
    for (int i = 0; i < N; ++i)
        t[i] = i * dt;

    double halfB = beatF * 0.5;
    double fLbase = baseF - halfB;
    double fRbase = baseF + halfB;
    for (int i = 0; i < N; ++i)
    {
        double vibL = (fORL * 0.5) * std::sin(2.0 * MathConstants<double>::pi * fOFL * t[i]);
        double vibR = (fORR * 0.5) * std::sin(2.0 * MathConstants<double>::pi * fOFR * t[i]);
        instL[i] = std::max(0.0, fLbase + vibL);
        instR[i] = std::max(0.0, fRbase + vibR);
    }

    if (forceMono || beatF == 0.0)
        for (int i = 0; i < N; ++i)
            instL[i] = instR[i] = std::max(0.0, baseF);

    double curL = startL;
    double curR = startR;
    for (int i = 0; i < N; ++i)
    {
        curL += 2.0 * MathConstants<double>::pi * instL[i] * dt;
        curR += 2.0 * MathConstants<double>::pi * instR[i] * dt;
        phaseL[i] = curL;
        phaseR[i] = curR;
    }

    if (pOF != 0.0 || pOR != 0.0)
    {
        for (int i = 0; i < N; ++i)
        {
            double dphi = (pOR * 0.5) * std::sin(2.0 * MathConstants<double>::pi * pOF * t[i]);
            phaseL[i] -= dphi;
            phaseR[i] += dphi;
        }
    }

    for (int i = 0; i < N; ++i)
    {
        envL[i] = 1.0 - aODL * (0.5 * (1.0 + std::sin(2.0 * MathConstants<double>::pi * aOFL * t[i] + ampOscPhaseOffsetL)));
        envR[i] = 1.0 - aODR * (0.5 * (1.0 + std::sin(2.0 * MathConstants<double>::pi * aOFR * t[i] + ampOscPhaseOffsetR)));
    }

    std::vector<int> glitchPos;
    std::vector<float> glitchBuf;
    if (glitchInterval > 0.0 && glitchDur > 0.0 && glitchNoiseLevel > 0.0 && N > 0)
    {
        int fullN = static_cast<int>(glitchDur * sampleRate);
        if (fullN > 0)
        {
            int repeats = glitchInterval > 0.0 ? static_cast<int>(duration / glitchInterval) : 0;
            std::mt19937 rng(std::random_device{}());
            std::normal_distribution<float> dist(0.0f, 1.0f);

            juce::dsp::IIR::Filter<float> bpFilter;
            if (glitchFocusWidth > 0.0)
            {
                double q = baseF / std::max(1e-6, glitchFocusWidth);
                bpFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, baseF, q);
            }

            for (int k = 1; k <= repeats; ++k)
            {
                double tEnd = k * glitchInterval;
                double tStart = std::max(0.0, tEnd - glitchDur);
                int i0 = static_cast<int>(tStart * sampleRate);
                int i1 = i0 + fullN;
                if (i1 > N)
                    continue;

                std::vector<float> noise(fullN);
                for (int i = 0; i < fullN; ++i)
                    noise[i] = dist(rng);

                if (glitchFocusWidth > 0.0)
                {
                    bpFilter.reset();
                    for (int i = 0; i < fullN; ++i)
                        noise[i] = bpFilter.processSample(noise[i]);
                }

                float maxAbs = 0.0f;
                for (float s : noise)
                    maxAbs = std::max(maxAbs, std::abs(s));
                if (maxAbs < 1e-6f)
                    maxAbs = 1.0f;

                for (int i = 0; i < fullN; ++i)
                {
                    float ramp = static_cast<float>(i) / static_cast<float>(fullN);
                    glitchBuf.push_back((noise[i] / maxAbs) * ramp * static_cast<float>(glitchNoiseLevel));
                }
                glitchPos.push_back(i0);
            }
        }
    }

    for (int i = 0; i < N; ++i)
    {
        float outL = static_cast<float>(std::sin(phaseL[i]) * envL[i] * ampL);
        float outR = static_cast<float>(std::sin(phaseR[i]) * envR[i] * ampR);
        buffer.setSample(0, i, outL);
        buffer.setSample(1, i, outR);
    }

    if (!glitchBuf.empty() && glitchBuf.size() % glitchPos.size() == 0)
    {
        int segLen = static_cast<int>(glitchBuf.size() / glitchPos.size());
        size_t idx = 0;
        for (size_t b = 0; b < glitchPos.size(); ++b)
        {
            int start = glitchPos[b];
            for (int j = 0; j < segLen; ++j)
            {
                int p = start + j;
                if (p < N)
                {
                    float val = glitchBuf[idx + j];
                    buffer.addSample(0, p, val);
                    buffer.addSample(1, p, val);
                }
            }
            idx += segLen;
        }
    }

    return buffer;
}

AudioBuffer<float> binauralBeatTransition(double duration, double sampleRate, const NamedValueSet& params)
{
    int N = static_cast<int>(duration * sampleRate);
    AudioBuffer<float> buffer(2, std::max(0, N));
    if (N <= 0)
        return buffer;

    double startAmpL = params.getWithDefault("startAmpL", params.getWithDefault("ampL", 0.5));
    double endAmpL   = params.getWithDefault("endAmpL", startAmpL);
    double startAmpR = params.getWithDefault("startAmpR", params.getWithDefault("ampR", 0.5));
    double endAmpR   = params.getWithDefault("endAmpR", startAmpR);
    double startBaseF = params.getWithDefault("startBaseFreq", params.getWithDefault("baseFreq", 200.0));
    double endBaseF   = params.getWithDefault("endBaseFreq", startBaseF);
    double startBeatF = params.getWithDefault("startBeatFreq", params.getWithDefault("beatFreq", 4.0));
    double endBeatF   = params.getWithDefault("endBeatFreq", startBeatF);
    double startForceMono = params.getWithDefault("startForceMono", params.getWithDefault("forceMono", 0.0));
    double endForceMono   = params.getWithDefault("endForceMono", startForceMono);
    double startStartPhaseL = params.getWithDefault("startStartPhaseL", params.getWithDefault("startPhaseL", 0.0));
    double endStartPhaseL   = params.getWithDefault("endStartPhaseL", startStartPhaseL);
    double startStartPhaseR = params.getWithDefault("startStartPhaseR", params.getWithDefault("startPhaseR", 0.0));
    double endStartPhaseR   = params.getWithDefault("endStartPhaseR", startStartPhaseR);
    double startPOF = params.getWithDefault("startPhaseOscFreq", params.getWithDefault("phaseOscFreq", 0.0));
    double endPOF   = params.getWithDefault("endPhaseOscFreq", startPOF);
    double startPOR = params.getWithDefault("startPhaseOscRange", params.getWithDefault("phaseOscRange", 0.0));
    double endPOR   = params.getWithDefault("endPhaseOscRange", startPOR);
    double startAODL = params.getWithDefault("startAmpOscDepthL", params.getWithDefault("ampOscDepthL", 0.0));
    double endAODL   = params.getWithDefault("endAmpOscDepthL", startAODL);
    double startAOFL = params.getWithDefault("startAmpOscFreqL", params.getWithDefault("ampOscFreqL", 0.0));
    double endAOFL   = params.getWithDefault("endAmpOscFreqL", startAOFL);
    double startAODR = params.getWithDefault("startAmpOscDepthR", params.getWithDefault("ampOscDepthR", 0.0));
    double endAODR   = params.getWithDefault("endAmpOscDepthR", startAODR);
    double startAOFR = params.getWithDefault("startAmpOscFreqR", params.getWithDefault("ampOscFreqR", 0.0));
    double endAOFR   = params.getWithDefault("endAmpOscFreqR", startAOFR);
    double startAmpOscPhaseOffsetL = params.getWithDefault("startAmpOscPhaseOffsetL", params.getWithDefault("ampOscPhaseOffsetL", 0.0));
    double endAmpOscPhaseOffsetL   = params.getWithDefault("endAmpOscPhaseOffsetL", startAmpOscPhaseOffsetL);
    double startAmpOscPhaseOffsetR = params.getWithDefault("startAmpOscPhaseOffsetR", params.getWithDefault("ampOscPhaseOffsetR", 0.0));
    double endAmpOscPhaseOffsetR   = params.getWithDefault("endAmpOscPhaseOffsetR", startAmpOscPhaseOffsetR);
    double startFORL = params.getWithDefault("startFreqOscRangeL", params.getWithDefault("freqOscRangeL", 0.0));
    double endFORL   = params.getWithDefault("endFreqOscRangeL", startFORL);
    double startFOFL = params.getWithDefault("startFreqOscFreqL", params.getWithDefault("freqOscFreqL", 0.0));
    double endFOFL   = params.getWithDefault("endFreqOscFreqL", startFOFL);
    double startFORR = params.getWithDefault("startFreqOscRangeR", params.getWithDefault("freqOscRangeR", 0.0));
    double endFORR   = params.getWithDefault("endFreqOscRangeR", startFORR);
    double startFOFR = params.getWithDefault("startFreqOscFreqR", params.getWithDefault("freqOscFreqR", 0.0));
    double endFOFR   = params.getWithDefault("endFreqOscFreqR", startFOFR);

    double sGlitchInterval = params.getWithDefault("startGlitchInterval", params.getWithDefault("glitchInterval", 0.0));
    double eGlitchInterval = params.getWithDefault("endGlitchInterval", sGlitchInterval);
    double avgGlitchInterval = (sGlitchInterval + eGlitchInterval) / 2.0;

    double sGlitchDur = params.getWithDefault("startGlitchDur", params.getWithDefault("glitchDur", 0.0));
    double eGlitchDur = params.getWithDefault("endGlitchDur", sGlitchDur);
    double avgGlitchDur = (sGlitchDur + eGlitchDur) / 2.0;

    double sGlitchNoiseLevel = params.getWithDefault("startGlitchNoiseLevel", params.getWithDefault("glitchNoiseLevel", 0.0));
    double eGlitchNoiseLevel = params.getWithDefault("endGlitchNoiseLevel", sGlitchNoiseLevel);
    double avgGlitchNoiseLevel = (sGlitchNoiseLevel + eGlitchNoiseLevel) / 2.0;

    double sGlitchFocusWidth = params.getWithDefault("startGlitchFocusWidth", params.getWithDefault("glitchFocusWidth", 0.0));
    double eGlitchFocusWidth = params.getWithDefault("endGlitchFocusWidth", sGlitchFocusWidth);
    double avgGlitchFocusWidth = (sGlitchFocusWidth + eGlitchFocusWidth) / 2.0;

    double sGlitchFocusExp = params.getWithDefault("startGlitchFocusExp", params.getWithDefault("glitchFocusExp", 0.0));
    double eGlitchFocusExp = params.getWithDefault("endGlitchFocusExp", sGlitchFocusExp);
    double avgGlitchFocusExp = (sGlitchFocusExp + eGlitchFocusExp) / 2.0;
    double initialOffset = params.getWithDefault("initial_offset", 0.0);
    double postOffset    = params.getWithDefault("post_offset", 0.0);
    String curve = params.getWithDefault("transition_curve", "linear");

    auto alpha = calculateTransitionAlpha(duration, sampleRate, initialOffset, postOffset, curve);
    std::vector<double> t(N);
    for (int i = 0; i < N; ++i)
        t[i] = i * (duration / static_cast<double>(N));

    std::vector<double> instL(N), instR(N), phaseL(N), phaseR(N), envL(N), envR(N);
    std::vector<double> outAmpL(N), outAmpR(N);

    std::vector<int> glitchPos;
    std::vector<float> glitchBuf;
    if (avgGlitchInterval > 0.0 && avgGlitchDur > 0.0 && avgGlitchNoiseLevel > 0.0 && N > 0)
    {
        int gSamples = static_cast<int>(avgGlitchDur * sampleRate);
        if (gSamples > 0)
        {
            int repeats = avgGlitchInterval > 0.0 ? static_cast<int>(duration / avgGlitchInterval) : 0;
            std::mt19937 rng(std::random_device{}());
            std::normal_distribution<float> dist(0.0f, 1.0f);
            double shapingFreq = (startBaseF + endBaseF) * 0.5;
            juce::dsp::IIR::Filter<float> bpFilter;
            if (avgGlitchFocusWidth > 0.0)
            {
                double q = shapingFreq / std::max(1e-6, avgGlitchFocusWidth);
                bpFilter.coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, shapingFreq, q);
            }

            for (int k = 1; k <= repeats; ++k)
            {
                double tEnd = k * avgGlitchInterval;
                double tStart = std::max(0.0, tEnd - avgGlitchDur);
                int i0 = static_cast<int>(tStart * sampleRate);
                int i1 = i0 + gSamples;
                if (i1 > N)
                    continue;

                std::vector<float> noise(gSamples);
                for (int i = 0; i < gSamples; ++i)
                    noise[i] = dist(rng);

                if (avgGlitchFocusWidth > 0.0)
                {
                    bpFilter.reset();
                    for (int i = 0; i < gSamples; ++i)
                        noise[i] = bpFilter.processSample(noise[i]);
                }

                float maxAbs = 0.0f;
                for (float s : noise)
                    maxAbs = std::max(maxAbs, std::abs(s));
                if (maxAbs < 1e-6f)
                    maxAbs = 1.0f;

                for (int i = 0; i < gSamples; ++i)
                {
                    float ramp = static_cast<float>(i) / static_cast<float>(gSamples);
                    float val = (noise[i] / maxAbs) * ramp * static_cast<float>(avgGlitchNoiseLevel);
                    glitchBuf.push_back(val);
                }
                glitchPos.push_back(i0);
            }
        }
    }

    for (int i = 0; i < N; ++i)
    {
        double a = alpha.empty() ? static_cast<double>(i) / (N > 1 ? N - 1 : 1) : alpha[i];
        outAmpL[i] = startAmpL + (endAmpL - startAmpL) * a;
        outAmpR[i] = startAmpR + (endAmpR - startAmpR) * a;
        double baseF = startBaseF + (endBaseF - startBaseF) * a;
        double beatF = startBeatF + (endBeatF - startBeatF) * a;
        double forceM = startForceMono + (endForceMono - startForceMono) * a;
        double forL = startFORL + (endFORL - startFORL) * a;
        double fofL = startFOFL + (endFOFL - startFOFL) * a;
        double forR = startFORR + (endFORR - startFORR) * a;
        double fofR = startFOFR + (endFOFR - startFOFR) * a;

        double halfB = beatF * 0.5;
        double fLbase = baseF - halfB;
        double fRbase = baseF + halfB;
        double vibL = (forL * 0.5) * std::sin(2.0 * MathConstants<double>::pi * fofL * t[i]);
        double vibR = (forR * 0.5) * std::sin(2.0 * MathConstants<double>::pi * fofR * t[i]);
        instL[i] = (forceM > 0.5 || beatF == 0.0) ? std::max(0.0, baseF) : std::max(0.0, fLbase + vibL);
        instR[i] = (forceM > 0.5 || beatF == 0.0) ? std::max(0.0, baseF) : std::max(0.0, fRbase + vibR);

        envL[i] = 1.0 - (startAODL + (endAODL - startAODL) * a) *
                        (0.5 * (1.0 + std::sin(2.0 * MathConstants<double>::pi * (startAOFL + (endAOFL - startAOFL) * a) * t[i] +
                                            (startAmpOscPhaseOffsetL + (endAmpOscPhaseOffsetL - startAmpOscPhaseOffsetL) * a))));
        envR[i] = 1.0 - (startAODR + (endAODR - startAODR) * a) *
                        (0.5 * (1.0 + std::sin(2.0 * MathConstants<double>::pi * (startAOFR + (endAOFR - startAOFR) * a) * t[i] +
                                            (startAmpOscPhaseOffsetR + (endAmpOscPhaseOffsetR - startAmpOscPhaseOffsetR) * a))));
    }

    double curL = startStartPhaseL;
    double curR = startStartPhaseR;
    double dt = duration / static_cast<double>(N);
    for (int i = 0; i < N; ++i)
    {
        curL += 2.0 * MathConstants<double>::pi * instL[i] * dt;
        curR += 2.0 * MathConstants<double>::pi * instR[i] * dt;
        phaseL[i] = curL;
        phaseR[i] = curR;
    }

    for (int i = 0; i < N; ++i)
    {
        double a = alpha.empty() ? static_cast<double>(i) / (N > 1 ? N - 1 : 1) : alpha[i];
        double pOFv = startPOF + (endPOF - startPOF) * a;
        double pORv = startPOR + (endPOR - startPOR) * a;
        double dphi = (pORv * 0.5) * std::sin(2.0 * MathConstants<double>::pi * pOFv * t[i]);
        phaseL[i] -= dphi;
        phaseR[i] += dphi;
    }

    for (int i = 0; i < N; ++i)
    {
        float outL = static_cast<float>(std::sin(phaseL[i]) * envL[i] * outAmpL[i]);
        float outR = static_cast<float>(std::sin(phaseR[i]) * envR[i] * outAmpR[i]);
        buffer.setSample(0, i, outL);
        buffer.setSample(1, i, outR);
    }

    if (!glitchBuf.empty() && glitchBuf.size() % glitchPos.size() == 0)
    {
        int segLen = static_cast<int>(glitchBuf.size() / glitchPos.size());
        size_t idx = 0;
        for (size_t b = 0; b < glitchPos.size(); ++b)
        {
            int start = glitchPos[b];
            for (int j = 0; j < segLen; ++j)
            {
                int p = start + j;
                if (p < N)
                {
                    float val = glitchBuf[idx + j];
                    buffer.addSample(0, p, val);
                    buffer.addSample(1, p, val);
                }
            }
            idx += segLen;
        }
    }

    return buffer;
}


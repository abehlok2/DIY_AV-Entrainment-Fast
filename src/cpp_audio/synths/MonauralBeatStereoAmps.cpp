#include "MonauralBeatStereoAmps.h"
#include "AudioUtils.h"
#include <cmath>
#include <algorithm>

using namespace juce;

AudioBuffer<float> monauralBeatStereoAmps(double duration, double sampleRate, const NamedValueSet& params)
{
    int N = static_cast<int>(duration * sampleRate);
    AudioBuffer<float> buffer(2, std::max(0, N));
    if (N <= 0)
        return buffer;

    double ampLL = params.getWithDefault("amp_lower_L", 0.5);
    double ampUL = params.getWithDefault("amp_upper_L", 0.5);
    double ampLR = params.getWithDefault("amp_lower_R", 0.5);
    double ampUR = params.getWithDefault("amp_upper_R", 0.5);
    double baseF = std::max(0.0, static_cast<double>(params.getWithDefault("baseFreq", 200.0)));
    double beatF = std::max(0.0, static_cast<double>(params.getWithDefault("beatFreq", 4.0)));
    double startL = params.getWithDefault("startPhaseL", 0.0);
    double startU = params.getWithDefault("startPhaseR", 0.0);
    double phiF = params.getWithDefault("phaseOscFreq", 0.0);
    double phiR = params.getWithDefault("phaseOscRange", 0.0);
    double aOD = params.getWithDefault("ampOscDepth", 0.0);
    double aOF = params.getWithDefault("ampOscFreq", 0.0);
    double aOP = params.getWithDefault("ampOscPhaseOffset", 0.0);

    double dt = 1.0 / sampleRate;
    double halfB = beatF * 0.5;
    double fLower = std::max(0.0, baseF - halfB);
    double fUpper = std::max(0.0, baseF + halfB);

    double phaseL = startL;
    double phaseU = startU;

    for (int i = 0; i < N; ++i)
    {
        double t = i * dt;

        phaseL += MathConstants<double>::twoPi * fLower * dt;
        phaseU += MathConstants<double>::twoPi * fUpper * dt;

        double dphi = (phiF != 0.0 || phiR != 0.0)
                           ? (phiR * 0.5) * std::sin(MathConstants<double>::twoPi * phiF * t)
                           : 0.0;

        double sLower = std::sin(phaseL - dphi);
        double sUpper = std::sin(phaseU + dphi);

        double outL = sLower * ampLL + sUpper * ampUL;
        double outR = sLower * ampLR + sUpper * ampUR;

        double depth = std::clamp(aOD, 0.0, 2.0);
        if (depth != 0.0 && aOF != 0.0)
        {
            double mod = (1.0 - depth * 0.5) + (depth * 0.5) * std::sin(MathConstants<double>::twoPi * aOF * t + aOP);
            outL *= mod;
            outR *= mod;
        }

        outL = std::clamp(outL, -1.0, 1.0);
        outR = std::clamp(outR, -1.0, 1.0);

        buffer.setSample(0, i, static_cast<float>(outL));
        buffer.setSample(1, i, static_cast<float>(outR));
    }

    return buffer;
}

AudioBuffer<float> monauralBeatStereoAmpsTransition(double duration, double sampleRate, const NamedValueSet& params)
{
    int N = static_cast<int>(duration * sampleRate);
    AudioBuffer<float> buffer(2, std::max(0, N));
    if (N <= 0)
        return buffer;

    double sLL = params.getWithDefault("start_amp_lower_L", params.getWithDefault("amp_lower_L", 0.5));
    double eLL = params.getWithDefault("end_amp_lower_L", sLL);
    double sUL = params.getWithDefault("start_amp_upper_L", params.getWithDefault("amp_upper_L", 0.5));
    double eUL = params.getWithDefault("end_amp_upper_L", sUL);
    double sLR = params.getWithDefault("start_amp_lower_R", params.getWithDefault("amp_lower_R", 0.5));
    double eLR = params.getWithDefault("end_amp_lower_R", sLR);
    double sUR = params.getWithDefault("start_amp_upper_R", params.getWithDefault("amp_upper_R", 0.5));
    double eUR = params.getWithDefault("end_amp_upper_R", sUR);

    double sBF = params.getWithDefault("startBaseFreq", params.getWithDefault("baseFreq", 200.0));
    double eBF = params.getWithDefault("endBaseFreq", sBF);
    double sBt = params.getWithDefault("startBeatFreq", params.getWithDefault("beatFreq", 4.0));
    double eBt = params.getWithDefault("endBeatFreq", sBt);

    double sPhaseL = params.getWithDefault("startStartPhaseL", params.getWithDefault("startPhaseL", 0.0));
    double ePhaseL = params.getWithDefault("endStartPhaseL", sPhaseL);
    double sPhaseU = params.getWithDefault("startStartPhaseU", params.getWithDefault("startPhaseR", 0.0));
    double ePhaseU = params.getWithDefault("endStartPhaseU", sPhaseU);

    double sPhiF = params.getWithDefault("startPhaseOscFreq", params.getWithDefault("phaseOscFreq", 0.0));
    double ePhiF = params.getWithDefault("endPhaseOscFreq", sPhiF);
    double sPhiR = params.getWithDefault("startPhaseOscRange", params.getWithDefault("phaseOscRange", 0.0));
    double ePhiR = params.getWithDefault("endPhaseOscRange", sPhiR);

    double sAOD = params.getWithDefault("startAmpOscDepth", params.getWithDefault("ampOscDepth", 0.0));
    double eAOD = params.getWithDefault("endAmpOscDepth", sAOD);
    double sAOF = params.getWithDefault("startAmpOscFreq", params.getWithDefault("ampOscFreq", 0.0));
    double eAOF = params.getWithDefault("endAmpOscFreq", sAOF);
    double sAOP = params.getWithDefault("startAmpOscPhaseOffset", params.getWithDefault("ampOscPhaseOffset", 0.0));
    double eAOP = params.getWithDefault("endAmpOscPhaseOffset", sAOP);

    double initialOffset = params.getWithDefault("initial_offset", 0.0);
    double postOffset    = params.getWithDefault("post_offset", 0.0);
    String curve         = params.getWithDefault("transition_curve", "linear");

    auto alpha = calculateTransitionAlpha(duration, sampleRate, initialOffset, postOffset, curve);

    double dt = 1.0 / sampleRate;
    double phaseL = sPhaseL;
    double phaseU = sPhaseU;

    for (int i = 0; i < N; ++i)
    {
        double a = alpha.empty() ? static_cast<double>(i) / (N > 1 ? N - 1 : 1) : alpha[i];

        double ampLL = sLL + (eLL - sLL) * a;
        double ampUL = sUL + (eUL - sUL) * a;
        double ampLR = sLR + (eLR - sLR) * a;
        double ampUR = sUR + (eUR - sUR) * a;

        double baseF = std::max(0.0, sBF + (eBF - sBF) * a);
        double beatF = std::max(0.0, sBt + (eBt - sBt) * a);
        double fLower = std::max(0.0, baseF - beatF * 0.5);
        double fUpper = std::max(0.0, baseF + beatF * 0.5);

        phaseL += MathConstants<double>::twoPi * fLower * dt;
        phaseU += MathConstants<double>::twoPi * fUpper * dt;

        double phiFv = sPhiF + (ePhiF - sPhiF) * a;
        double phiRv = sPhiR + (ePhiR - sPhiR) * a;
        double dphi = (phiFv != 0.0 || phiRv != 0.0)
                           ? (phiRv * 0.5) * std::sin(MathConstants<double>::twoPi * phiFv * (i * dt))
                           : 0.0;

        double sLow = std::sin(phaseL - dphi);
        double sUp  = std::sin(phaseU + dphi);

        double outL = sLow * ampLL + sUp * ampUL;
        double outR = sLow * ampLR + sUp * ampUR;

        double depth = std::clamp(sAOD + (eAOD - sAOD) * a, 0.0, 2.0);
        double aOFv  = sAOF + (eAOF - sAOF) * a;
        double aOPv  = sAOP + (eAOP - sAOP) * a;

        if (depth != 0.0 && aOFv != 0.0)
        {
            double mod = (1.0 - depth * 0.5) + (depth * 0.5) * std::sin(MathConstants<double>::twoPi * aOFv * (i * dt) + aOPv);
            outL *= mod;
            outR *= mod;
        }

        outL = std::clamp(outL, -1.0, 1.0);
        outR = std::clamp(outR, -1.0, 1.0);

        buffer.setSample(0, i, static_cast<float>(outL));
        buffer.setSample(1, i, static_cast<float>(outR));
    }

    return buffer;
}

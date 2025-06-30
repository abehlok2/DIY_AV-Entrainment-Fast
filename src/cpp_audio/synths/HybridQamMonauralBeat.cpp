#include "HybridQamMonauralBeat.h"
#include "AudioUtils.h"

using namespace juce;

AudioBuffer<float> hybridQamMonauralBeat(double duration, double sampleRate, const NamedValueSet& params)
{
    const int N = static_cast<int>(duration * sampleRate);
    AudioBuffer<float> buffer(2, jmax(0, N));
    if (N <= 0)
        return buffer;

    const double ampL = params.getWithDefault("ampL", 0.5);
    const double ampR = params.getWithDefault("ampR", 0.5);

    const double qamCarrierFreqL   = params.getWithDefault("qamCarrierFreqL", 100.0);
    const double qamAmFreqL        = params.getWithDefault("qamAmFreqL", 4.0);
    const double qamAmDepthL       = params.getWithDefault("qamAmDepthL", 0.5);
    const double qamAmPhaseOffsetL = params.getWithDefault("qamAmPhaseOffsetL", 0.0);
    const double qamStartPhaseL    = params.getWithDefault("qamStartPhaseL", 0.0);

    const double monoCarrierFreqR      = params.getWithDefault("monoCarrierFreqR", 100.0);
    const double monoBeatFreqInChannel = params.getWithDefault("monoBeatFreqInChannelR", 4.0);
    const double monoAmDepthR          = params.getWithDefault("monoAmDepthR", 0.0);
    const double monoAmFreqR           = params.getWithDefault("monoAmFreqR", 0.0);
    const double monoAmPhaseOffsetR    = params.getWithDefault("monoAmPhaseOffsetR", 0.0);
    const double monoFmRangeR          = params.getWithDefault("monoFmRangeR", 0.0);
    const double monoFmFreqR           = params.getWithDefault("monoFmFreqR", 0.0);
    const double monoFmPhaseOffsetR    = params.getWithDefault("monoFmPhaseOffsetR", 0.0);
    const double monoStartPhaseTone1R  = params.getWithDefault("monoStartPhaseR_Tone1", 0.0);
    const double monoStartPhaseTone2R  = params.getWithDefault("monoStartPhaseR_Tone2", 0.0);
    const double monoPhaseOscFreqR     = params.getWithDefault("monoPhaseOscFreqR", 0.0);
    const double monoPhaseOscRangeR    = params.getWithDefault("monoPhaseOscRangeR", 0.0);
    const double monoPhaseOscPhaseOffR = params.getWithDefault("monoPhaseOscPhaseOffsetR", 0.0);

    std::vector<double> t(N);
    const double dt = 1.0 / sampleRate;
    for (int i = 0; i < N; ++i)
        t[i] = i * dt;

    // --- Left channel (QAM style modulation) ---
    std::vector<double> phQAM(N);
    double curPhaseQAM = qamStartPhaseL;
    for (int i = 0; i < N; ++i)
    {
        phQAM[i] = curPhaseQAM;
        curPhaseQAM += MathConstants<double>::twoPi * qamCarrierFreqL * dt;
    }

    std::vector<double> envQAM(N, 1.0);
    if (qamAmFreqL != 0.0 && qamAmDepthL != 0.0)
    {
        for (int i = 0; i < N; ++i)
            envQAM[i] = 1.0 + qamAmDepthL * std::cos(MathConstants<double>::twoPi * qamAmFreqL * t[i] + qamAmPhaseOffsetL);
    }

    // --- Right channel (monaural beat) ---
    std::vector<double> carrierInst(N);
    for (int i = 0; i < N; ++i)
    {
        double mod = 0.0;
        if (monoFmFreqR != 0.0 && monoFmRangeR != 0.0)
            mod = (monoFmRangeR * 0.5) * std::sin(MathConstants<double>::twoPi * monoFmFreqR * t[i] + monoFmPhaseOffsetR);
        carrierInst[i] = std::max(0.0, monoCarrierFreqR + mod);
    }

    const double halfBeat = monoBeatFreqInChannel * 0.5;
    std::vector<double> phTone1(N), phTone2(N);
    double cur1 = monoStartPhaseTone1R;
    double cur2 = monoStartPhaseTone2R;
    for (int i = 0; i < N; ++i)
    {
        double f1 = std::max(0.0, carrierInst[i] - halfBeat);
        double f2 = std::max(0.0, carrierInst[i] + halfBeat);
        cur1 += MathConstants<double>::twoPi * f1 * dt;
        cur2 += MathConstants<double>::twoPi * f2 * dt;
        phTone1[i] = cur1;
        phTone2[i] = cur2;
    }

    if (monoPhaseOscFreqR != 0.0 || monoPhaseOscRangeR != 0.0)
    {
        for (int i = 0; i < N; ++i)
        {
            double dphi = (monoPhaseOscRangeR * 0.5) * std::sin(MathConstants<double>::twoPi * monoPhaseOscFreqR * t[i] + monoPhaseOscPhaseOffR);
            phTone1[i] -= dphi;
            phTone2[i] += dphi;
        }
    }

    std::vector<double> envMono(N, 1.0);
    if (monoAmFreqR != 0.0 && monoAmDepthR != 0.0)
    {
        double clamped = std::clamp(monoAmDepthR, 0.0, 1.0);
        for (int i = 0; i < N; ++i)
            envMono[i] = 1.0 - clamped * (0.5 * (1.0 + std::sin(MathConstants<double>::twoPi * monoAmFreqR * t[i] + monoAmPhaseOffsetR)));
    }

    buffer.clear();
    for (int i = 0; i < N; ++i)
    {
        float left = static_cast<float>(std::cos(phQAM[i]) * envQAM[i] * ampL);
        float right = static_cast<float>((std::sin(phTone1[i]) + std::sin(phTone2[i])) * envMono[i] * ampR);
        buffer.setSample(0, i, left);
        buffer.setSample(1, i, right);
    }

    return buffer;
}

AudioBuffer<float> hybridQamMonauralBeatTransition(double duration, double sampleRate, const NamedValueSet& params)
{
    const int N = static_cast<int>(duration * sampleRate);
    AudioBuffer<float> buffer(2, jmax(0, N));
    if (N <= 0)
        return buffer;

    const double initialOffset = params.getWithDefault("initial_offset", 0.0);
    const double postOffset    = params.getWithDefault("post_offset", 0.0);
    const String curve         = params.getWithDefault("transition_curve", "linear");
    auto alpha = calculateTransitionAlpha(duration, sampleRate, initialOffset, postOffset, curve);

    const double s_ampL = params.getWithDefault("startAmpL", params.getWithDefault("ampL", 0.5));
    const double e_ampL = params.getWithDefault("endAmpL", s_ampL);
    const double s_ampR = params.getWithDefault("startAmpR", params.getWithDefault("ampR", 0.5));
    const double e_ampR = params.getWithDefault("endAmpR", s_ampR);

    const double s_qamCarrierFreqL   = params.getWithDefault("startQamCarrierFreqL", params.getWithDefault("qamCarrierFreqL", 100.0));
    const double e_qamCarrierFreqL   = params.getWithDefault("endQamCarrierFreqL", s_qamCarrierFreqL);
    const double s_qamAmFreqL        = params.getWithDefault("startQamAmFreqL", params.getWithDefault("qamAmFreqL", 4.0));
    const double e_qamAmFreqL        = params.getWithDefault("endQamAmFreqL", s_qamAmFreqL);
    const double s_qamAmDepthL       = params.getWithDefault("startQamAmDepthL", params.getWithDefault("qamAmDepthL", 0.5));
    const double e_qamAmDepthL       = params.getWithDefault("endQamAmDepthL", s_qamAmDepthL);
    const double s_qamAmPhaseOffsetL = params.getWithDefault("startQamAmPhaseOffsetL", params.getWithDefault("qamAmPhaseOffsetL", 0.0));
    const double e_qamAmPhaseOffsetL = params.getWithDefault("endQamAmPhaseOffsetL", s_qamAmPhaseOffsetL);
    const double s_qamStartPhaseL    = params.getWithDefault("startQamStartPhaseL", params.getWithDefault("qamStartPhaseL", 0.0));
    const double e_qamStartPhaseL    = params.getWithDefault("endQamStartPhaseL", s_qamStartPhaseL);

    const double s_monoCarrierFreqR      = params.getWithDefault("startMonoCarrierFreqR", params.getWithDefault("monoCarrierFreqR", 100.0));
    const double e_monoCarrierFreqR      = params.getWithDefault("endMonoCarrierFreqR", s_monoCarrierFreqR);
    const double s_monoBeatFreqInChannel = params.getWithDefault("startMonoBeatFreqInChannelR", params.getWithDefault("monoBeatFreqInChannelR", 4.0));
    const double e_monoBeatFreqInChannel = params.getWithDefault("endMonoBeatFreqInChannelR", s_monoBeatFreqInChannel);

    const double s_monoAmDepthR       = params.getWithDefault("startMonoAmDepthR", params.getWithDefault("monoAmDepthR", 0.0));
    const double e_monoAmDepthR       = params.getWithDefault("endMonoAmDepthR", s_monoAmDepthR);
    const double s_monoAmFreqR        = params.getWithDefault("startMonoAmFreqR", params.getWithDefault("monoAmFreqR", 0.0));
    const double e_monoAmFreqR        = params.getWithDefault("endMonoAmFreqR", s_monoAmFreqR);
    const double s_monoAmPhaseOffsetR = params.getWithDefault("startMonoAmPhaseOffsetR", params.getWithDefault("monoAmPhaseOffsetR", 0.0));
    const double e_monoAmPhaseOffsetR = params.getWithDefault("endMonoAmPhaseOffsetR", s_monoAmPhaseOffsetR);

    const double s_monoFmRangeR       = params.getWithDefault("startMonoFmRangeR", params.getWithDefault("monoFmRangeR", 0.0));
    const double e_monoFmRangeR       = params.getWithDefault("endMonoFmRangeR", s_monoFmRangeR);
    const double s_monoFmFreqR        = params.getWithDefault("startMonoFmFreqR", params.getWithDefault("monoFmFreqR", 0.0));
    const double e_monoFmFreqR        = params.getWithDefault("endMonoFmFreqR", s_monoFmFreqR);
    const double s_monoFmPhaseOffsetR = params.getWithDefault("startMonoFmPhaseOffsetR", params.getWithDefault("monoFmPhaseOffsetR", 0.0));
    const double e_monoFmPhaseOffsetR = params.getWithDefault("endMonoFmPhaseOffsetR", s_monoFmPhaseOffsetR);

    const double s_monoStartPhaseTone1R = params.getWithDefault("startMonoStartPhaseR_Tone1", params.getWithDefault("monoStartPhaseR_Tone1", 0.0));
    const double e_monoStartPhaseTone1R = params.getWithDefault("endMonoStartPhaseR_Tone1", s_monoStartPhaseTone1R);
    const double s_monoStartPhaseTone2R = params.getWithDefault("startMonoStartPhaseR_Tone2", params.getWithDefault("monoStartPhaseR_Tone2", 0.0));
    const double e_monoStartPhaseTone2R = params.getWithDefault("endMonoStartPhaseR_Tone2", s_monoStartPhaseTone2R);

    const double s_monoPhaseOscFreqR     = params.getWithDefault("startMonoPhaseOscFreqR", params.getWithDefault("monoPhaseOscFreqR", 0.0));
    const double e_monoPhaseOscFreqR     = params.getWithDefault("endMonoPhaseOscFreqR", s_monoPhaseOscFreqR);
    const double s_monoPhaseOscRangeR    = params.getWithDefault("startMonoPhaseOscRangeR", params.getWithDefault("monoPhaseOscRangeR", 0.0));
    const double e_monoPhaseOscRangeR    = params.getWithDefault("endMonoPhaseOscRangeR", s_monoPhaseOscRangeR);
    const double s_monoPhaseOscPhaseOffR = params.getWithDefault("startMonoPhaseOscPhaseOffsetR", params.getWithDefault("monoPhaseOscPhaseOffsetR", 0.0));
    const double e_monoPhaseOscPhaseOffR = params.getWithDefault("endMonoPhaseOscPhaseOffsetR", s_monoPhaseOscPhaseOffR);

    std::vector<double> t(N);
    const double dt = 1.0 / sampleRate;
    for (int i = 0; i < N; ++i)
        t[i] = i * dt;

    std::vector<double> phQAM(N);
    double curPhaseQAM = s_qamStartPhaseL;
    for (int i = 0; i < N; ++i)
    {
        double a = alpha.empty() ? static_cast<double>(i) / (N > 1 ? N - 1 : 1) : alpha[i];
        double freq = s_qamCarrierFreqL + (e_qamCarrierFreqL - s_qamCarrierFreqL) * a;
        phQAM[i] = curPhaseQAM;
        curPhaseQAM += MathConstants<double>::twoPi * freq * dt;
    }

    std::vector<double> envQAM(N, 1.0);
    for (int i = 0; i < N; ++i)
    {
        double a = alpha.empty() ? static_cast<double>(i) / (N > 1 ? N - 1 : 1) : alpha[i];
        double f = s_qamAmFreqL + (e_qamAmFreqL - s_qamAmFreqL) * a;
        double d = s_qamAmDepthL + (e_qamAmDepthL - s_qamAmDepthL) * a;
        double p = s_qamAmPhaseOffsetL + (e_qamAmPhaseOffsetL - s_qamAmPhaseOffsetL) * a;
        if (f != 0.0 && d != 0.0)
            envQAM[i] = 1.0 + d * std::cos(MathConstants<double>::twoPi * f * t[i] + p);
    }

    // Right channel parameters per sample
    std::vector<double> carrierInst(N);
    std::vector<double> ampEnv(N, 1.0);
    for (int i = 0; i < N; ++i)
    {
        double a = alpha.empty() ? static_cast<double>(i) / (N > 1 ? N - 1 : 1) : alpha[i];
        double base = s_monoCarrierFreqR + (e_monoCarrierFreqR - s_monoCarrierFreqR) * a;
        double range = s_monoFmRangeR + (e_monoFmRangeR - s_monoFmRangeR) * a;
        double freq = s_monoFmFreqR + (e_monoFmFreqR - s_monoFmFreqR) * a;
        double phaseOff = s_monoFmPhaseOffsetR + (e_monoFmPhaseOffsetR - s_monoFmPhaseOffsetR) * a;
        double mod = 0.0;
        if (freq != 0.0 && range != 0.0)
            mod = (range * 0.5) * std::sin(MathConstants<double>::twoPi * freq * t[i] + phaseOff);
        carrierInst[i] = std::max(0.0, base + mod);

        double ampD = s_monoAmDepthR + (e_monoAmDepthR - s_monoAmDepthR) * a;
        double ampF = s_monoAmFreqR + (e_monoAmFreqR - s_monoAmFreqR) * a;
        double ampP = s_monoAmPhaseOffsetR + (e_monoAmPhaseOffsetR - s_monoAmPhaseOffsetR) * a;
        if (ampF != 0.0 && ampD != 0.0)
        {
            double c = std::clamp(ampD, 0.0, 1.0);
            ampEnv[i] = 1.0 - c * (0.5 * (1.0 + std::sin(MathConstants<double>::twoPi * ampF * t[i] + ampP)));
        }
    }

    const double halfBeat = (s_monoBeatFreqInChannel + (e_monoBeatFreqInChannel - s_monoBeatFreqInChannel)) * 0.5; // approximate
    std::vector<double> phTone1(N), phTone2(N);
    double cur1 = s_monoStartPhaseTone1R;
    double cur2 = s_monoStartPhaseTone2R;
    for (int i = 0; i < N; ++i)
    {
        phTone1[i] = cur1;
        phTone2[i] = cur2;
        double a = alpha.empty() ? static_cast<double>(i) / (N > 1 ? N - 1 : 1) : alpha[i];
        double beat = s_monoBeatFreqInChannel + (e_monoBeatFreqInChannel - s_monoBeatFreqInChannel) * a;
        double f1 = std::max(0.0, carrierInst[i] - beat * 0.5);
        double f2 = std::max(0.0, carrierInst[i] + beat * 0.5);
        cur1 += MathConstants<double>::twoPi * f1 * dt;
        cur2 += MathConstants<double>::twoPi * f2 * dt;
    }

    for (int i = 0; i < N; ++i)
    {
        double a = alpha.empty() ? static_cast<double>(i) / (N > 1 ? N - 1 : 1) : alpha[i];
        double oscF   = s_monoPhaseOscFreqR + (e_monoPhaseOscFreqR - s_monoPhaseOscFreqR) * a;
        double oscR   = s_monoPhaseOscRangeR + (e_monoPhaseOscRangeR - s_monoPhaseOscRangeR) * a;
        double oscOff = s_monoPhaseOscPhaseOffR + (e_monoPhaseOscPhaseOffR - s_monoPhaseOscPhaseOffR) * a;
        if (oscF != 0.0 || oscR != 0.0)
        {
            double dphi = (oscR * 0.5) * std::sin(MathConstants<double>::twoPi * oscF * t[i] + oscOff);
            phTone1[i] -= dphi;
            phTone2[i] += dphi;
        }
    }

    buffer.clear();
    for (int i = 0; i < N; ++i)
    {
        double a = alpha.empty() ? static_cast<double>(i) / (N > 1 ? N - 1 : 1) : alpha[i];
        double ampLeft  = s_ampL + (e_ampL - s_ampL) * a;
        double ampRight = s_ampR + (e_ampR - s_ampR) * a;
        float left = static_cast<float>(std::cos(phQAM[i]) * envQAM[i] * ampLeft);
        float right = static_cast<float>((std::sin(phTone1[i]) + std::sin(phTone2[i])) * ampEnv[i] * ampRight);
        buffer.setSample(0, i, left);
        buffer.setSample(1, i, right);
    }

    return buffer;
}

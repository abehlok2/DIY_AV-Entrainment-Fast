#include "SynthFunctions.h"
#include "AudioUtils.h"
#include <cmath>

using namespace juce;

AudioBuffer<float> stereoAMIndependent(double duration, double sampleRate, const NamedValueSet& params)
{
    double amp = params.getWithDefault("amp", 0.25);
    double carrierFreq = params.getWithDefault("carrierFreq", 200.0);
    double modFreqL = params.getWithDefault("modFreqL", 4.0);
    double modDepthL = params.getWithDefault("modDepthL", 0.8);
    double modPhaseL = params.getWithDefault("modPhaseL", 0.0);
    double modFreqR = params.getWithDefault("modFreqR", 4.0);
    double modDepthR = params.getWithDefault("modDepthR", 0.8);
    double modPhaseR = params.getWithDefault("modPhaseR", 0.0);
    double stereoWidthHz = params.getWithDefault("stereo_width_hz", 0.2);

    int totalSamples = static_cast<int>(duration * sampleRate);
    AudioBuffer<float> buffer(2, totalSamples);

    double dt = 1.0 / sampleRate;
    double carrierPhaseL = 0.0;
    double carrierPhaseR = 0.0;
    double lfoPhaseL = modPhaseL;
    double lfoPhaseR = modPhaseR;

    for (int i = 0; i < totalSamples; ++i)
    {
        double carrierL = std::sin(carrierPhaseL);
        double carrierR = std::sin(carrierPhaseR);
        double lfoL = std::sin(lfoPhaseL);
        double lfoR = std::sin(lfoPhaseR);

        double modL = 1.0 - modDepthL * (1.0 - lfoL) * 0.5;
        double modR = 1.0 - modDepthR * (1.0 - lfoR) * 0.5;

        float outL = static_cast<float>(carrierL * modL * amp);
        float outR = static_cast<float>(carrierR * modR * amp);

        buffer.setSample(0, i, outL);
        buffer.setSample(1, i, outR);

        carrierPhaseL += MathConstants<double>::twoPi * (carrierFreq - stereoWidthHz * 0.5) * dt;
        carrierPhaseR += MathConstants<double>::twoPi * (carrierFreq + stereoWidthHz * 0.5) * dt;
        lfoPhaseL += MathConstants<double>::twoPi * modFreqL * dt;
        lfoPhaseR += MathConstants<double>::twoPi * modFreqR * dt;
    }

    return buffer;
}

AudioBuffer<float> stereoAMIndependentTransition(double duration, double sampleRate, const NamedValueSet& params)
{
    double amp = params.getWithDefault("amp", 0.25);
    double startCarrierFreq = params.getWithDefault("startCarrierFreq", 200.0);
    double endCarrierFreq = params.getWithDefault("endCarrierFreq", 250.0);
    double startModFreqL = params.getWithDefault("startModFreqL", 4.0);
    double endModFreqL = params.getWithDefault("endModFreqL", 6.0);
    double startModDepthL = params.getWithDefault("startModDepthL", 0.8);
    double endModDepthL = params.getWithDefault("endModDepthL", 0.8);
    double startModPhaseL = params.getWithDefault("startModPhaseL", 0.0);
    double startModFreqR = params.getWithDefault("startModFreqR", 4.1);
    double endModFreqR = params.getWithDefault("endModFreqR", 5.9);
    double startModDepthR = params.getWithDefault("startModDepthR", 0.8);
    double endModDepthR = params.getWithDefault("endModDepthR", 0.8);
    double startModPhaseR = params.getWithDefault("startModPhaseR", 0.0);
    double startStereoWidthHz = params.getWithDefault("startStereoWidthHz", 0.2);
    double endStereoWidthHz = params.getWithDefault("endStereoWidthHz", 0.2);

    double initialOffset = params.getWithDefault("initial_offset", 0.0);
    double postOffset = params.getWithDefault("post_offset", 0.0);
    String curve = params.getWithDefault("transition_curve", "linear");

    int totalSamples = static_cast<int>(duration * sampleRate);
    AudioBuffer<float> buffer(2, totalSamples);
    auto alpha = calculateTransitionAlpha(duration, sampleRate, initialOffset, postOffset, curve);

    double dt = 1.0 / sampleRate;
    double carrierPhaseL = 0.0;
    double carrierPhaseR = 0.0;
    double lfoPhaseL = startModPhaseL;
    double lfoPhaseR = startModPhaseR;

    for (int i = 0; i < totalSamples; ++i)
    {
        double a = alpha.empty() ? static_cast<double>(i) / (totalSamples > 1 ? totalSamples - 1 : 1)
                                 : alpha[i];

        double carrierFreq = startCarrierFreq + (endCarrierFreq - startCarrierFreq) * a;
        double modFreqL = startModFreqL + (endModFreqL - startModFreqL) * a;
        double modDepthL = startModDepthL + (endModDepthL - startModDepthL) * a;
        double modFreqR = startModFreqR + (endModFreqR - startModFreqR) * a;
        double modDepthR = startModDepthR + (endModDepthR - startModDepthR) * a;
        double stereoWidthHz = startStereoWidthHz + (endStereoWidthHz - startStereoWidthHz) * a;

        double carrierL = std::sin(carrierPhaseL);
        double carrierR = std::sin(carrierPhaseR);
        double lfoL = std::sin(lfoPhaseL);
        double lfoR = std::sin(lfoPhaseR);

        double modL = 1.0 - modDepthL * (1.0 - lfoL) * 0.5;
        double modR = 1.0 - modDepthR * (1.0 - lfoR) * 0.5;

        float outL = static_cast<float>(carrierL * modL * amp);
        float outR = static_cast<float>(carrierR * modR * amp);

        buffer.setSample(0, i, outL);
        buffer.setSample(1, i, outR);

        carrierPhaseL += MathConstants<double>::twoPi * (carrierFreq - stereoWidthHz * 0.5) * dt;
        carrierPhaseR += MathConstants<double>::twoPi * (carrierFreq + stereoWidthHz * 0.5) * dt;
        lfoPhaseL += MathConstants<double>::twoPi * modFreqL * dt;
        lfoPhaseR += MathConstants<double>::twoPi * modFreqR * dt;
    }

    return buffer;
}

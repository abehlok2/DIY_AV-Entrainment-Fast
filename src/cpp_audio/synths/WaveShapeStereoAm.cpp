#include "SynthFunctions.h"
#include "AudioUtils.h"
#include <cmath>

using namespace juce;

AudioBuffer<float> waveShapeStereoAm(double duration, double sampleRate, const NamedValueSet& params)
{
    double amp = params.getWithDefault("amp", 0.15);
    double carrierFreq = params.getWithDefault("carrierFreq", 200.0);
    double shapeModFreq = params.getWithDefault("shapeModFreq", 4.0);
    double shapeModDepth = params.getWithDefault("shapeModDepth", 0.8);
    double shapeAmount = params.getWithDefault("shapeAmount", 0.5);
    double stereoModFreqL = params.getWithDefault("stereoModFreqL", 4.1);
    double stereoModDepthL = params.getWithDefault("stereoModDepthL", 0.8);
    double stereoModPhaseL = params.getWithDefault("stereoModPhaseL", 0.0);
    double stereoModFreqR = params.getWithDefault("stereoModFreqR", 4.0);
    double stereoModDepthR = params.getWithDefault("stereoModDepthR", 0.8);
    double stereoModPhaseR = params.getWithDefault("stereoModPhaseR", MathConstants<double>::halfPi);

    int totalSamples = static_cast<int>(duration * sampleRate);
    AudioBuffer<float> buffer(2, totalSamples);

    double dt = 1.0 / sampleRate;
    double carrierPhase = 0.0;
    double shapePhase = 0.0;
    double stereoPhaseL = stereoModPhaseL;
    double stereoPhaseR = stereoModPhaseR;
    double tanhShape = std::tanh(std::max(1e-6, shapeAmount));

    for (int i = 0; i < totalSamples; ++i)
    {
        double carrier = std::sin(carrierPhase);
        double shapeLFO = std::sin(shapePhase);
        double shapeAmp = 1.0 - shapeModDepth * (1.0 - shapeLFO) * 0.5;
        double shaped = std::tanh(carrier * shapeAmp * shapeAmount) / tanhShape;

        double lfoL = std::sin(stereoPhaseL);
        double lfoR = std::sin(stereoPhaseR);
        double modL = 1.0 - stereoModDepthL * (1.0 - lfoL) * 0.5;
        double modR = 1.0 - stereoModDepthR * (1.0 - lfoR) * 0.5;

        float outL = static_cast<float>(shaped * modL * amp);
        float outR = static_cast<float>(shaped * modR * amp);

        buffer.setSample(0, i, outL);
        buffer.setSample(1, i, outR);

        carrierPhase += MathConstants<double>::twoPi * carrierFreq * dt;
        shapePhase += MathConstants<double>::twoPi * shapeModFreq * dt;
        stereoPhaseL += MathConstants<double>::twoPi * stereoModFreqL * dt;
        stereoPhaseR += MathConstants<double>::twoPi * stereoModFreqR * dt;
    }

    return buffer;
}

AudioBuffer<float> waveShapeStereoAmTransition(double duration, double sampleRate, const NamedValueSet& params)
{
    double amp = params.getWithDefault("amp", 0.15);
    double startCarrierFreq = params.getWithDefault("startCarrierFreq", 200.0);
    double endCarrierFreq = params.getWithDefault("endCarrierFreq", 100.0);
    double startShapeModFreq = params.getWithDefault("startShapeModFreq", 4.0);
    double endShapeModFreq = params.getWithDefault("endShapeModFreq", 8.0);
    double startShapeModDepth = params.getWithDefault("startShapeModDepth", 0.8);
    double endShapeModDepth = params.getWithDefault("endShapeModDepth", 0.8);
    double startShapeAmount = params.getWithDefault("startShapeAmount", 0.5);
    double endShapeAmount = params.getWithDefault("endShapeAmount", 0.5);
    double startStereoModFreqL = params.getWithDefault("startStereoModFreqL", 4.1);
    double endStereoModFreqL = params.getWithDefault("endStereoModFreqL", 6.0);
    double startStereoModDepthL = params.getWithDefault("startStereoModDepthL", 0.8);
    double endStereoModDepthL = params.getWithDefault("endStereoModDepthL", 0.8);
    double startStereoModPhaseL = params.getWithDefault("startStereoModPhaseL", 0.0);
    double startStereoModFreqR = params.getWithDefault("startStereoModFreqR", 4.0);
    double endStereoModFreqR = params.getWithDefault("endStereoModFreqR", 6.1);
    double startStereoModDepthR = params.getWithDefault("startStereoModDepthR", 0.9);
    double endStereoModDepthR = params.getWithDefault("endStereoModDepthR", 0.9);
    double startStereoModPhaseR = params.getWithDefault("startStereoModPhaseR", MathConstants<double>::halfPi);

    double initialOffset = params.getWithDefault("initial_offset", 0.0);
    double postOffset = params.getWithDefault("post_offset", 0.0);
    String curve = params.getWithDefault("transition_curve", "linear");

    int totalSamples = static_cast<int>(duration * sampleRate);
    AudioBuffer<float> buffer(2, totalSamples);
    auto alpha = calculateTransitionAlpha(duration, sampleRate, initialOffset, postOffset, curve);

    double dt = 1.0 / sampleRate;
    double carrierPhase = 0.0;
    double shapePhase = 0.0;
    double stereoPhaseL = startStereoModPhaseL;
    double stereoPhaseR = startStereoModPhaseR;

    for (int i = 0; i < totalSamples; ++i)
    {
        double a = alpha.empty() ? static_cast<double>(i) / (totalSamples > 1 ? totalSamples - 1 : 1)
                                 : alpha[i];

        double carrierFreq = startCarrierFreq + (endCarrierFreq - startCarrierFreq) * a;
        double shapeModFreq = startShapeModFreq + (endShapeModFreq - startShapeModFreq) * a;
        double shapeModDepth = startShapeModDepth + (endShapeModDepth - startShapeModDepth) * a;
        double shapeAmount = startShapeAmount + (endShapeAmount - startShapeAmount) * a;
        double stereoModFreqL = startStereoModFreqL + (endStereoModFreqL - startStereoModFreqL) * a;
        double stereoModDepthL = startStereoModDepthL + (endStereoModDepthL - startStereoModDepthL) * a;
        double stereoModFreqR = startStereoModFreqR + (endStereoModFreqR - startStereoModFreqR) * a;
        double stereoModDepthR = startStereoModDepthR + (endStereoModDepthR - startStereoModDepthR) * a;

        double carrier = std::sin(carrierPhase);
        double shapeLFO = std::sin(shapePhase);
        double shapeAmp = 1.0 - shapeModDepth * (1.0 - shapeLFO) * 0.5;
        double tanhShape = std::tanh(std::max(1e-6, shapeAmount));
        double shaped = std::tanh(carrier * shapeAmp * shapeAmount) / tanhShape;

        double lfoL = std::sin(stereoPhaseL);
        double lfoR = std::sin(stereoPhaseR);
        double modL = 1.0 - stereoModDepthL * (1.0 - lfoL) * 0.5;
        double modR = 1.0 - stereoModDepthR * (1.0 - lfoR) * 0.5;

        float outL = static_cast<float>(shaped * modL * amp);
        float outR = static_cast<float>(shaped * modR * amp);

        buffer.setSample(0, i, outL);
        buffer.setSample(1, i, outR);

        carrierPhase += MathConstants<double>::twoPi * carrierFreq * dt;
        shapePhase += MathConstants<double>::twoPi * shapeModFreq * dt;
        stereoPhaseL += MathConstants<double>::twoPi * stereoModFreqL * dt;
        stereoPhaseR += MathConstants<double>::twoPi * stereoModFreqR * dt;
    }

    return buffer;
}

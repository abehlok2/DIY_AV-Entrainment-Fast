#include "SynthFunctions.h"
#include "AudioUtils.h"
#include <cmath>

using namespace juce;

AudioBuffer<float> rhythmicWaveshaping(double duration, double sampleRate, const NamedValueSet& params)
{
    double amp = params.getWithDefault("amp", 0.25);
    double carrierFreq = params.getWithDefault("carrierFreq", 200.0);
    double modFreq = params.getWithDefault("modFreq", 4.0);
    double modDepth = params.getWithDefault("modDepth", 1.0);
    double shapeAmount = params.getWithDefault("shapeAmount", 5.0);
    double pan = params.getWithDefault("pan", 0.0);

    int totalSamples = static_cast<int>(duration * sampleRate);
    AudioBuffer<float> buffer(2, totalSamples);
    auto gains = getPanGains(pan);

    double dt = 1.0 / sampleRate;
    double carrierPhase = 0.0;
    double modPhase = 0.0;
    double tanhShape = std::tanh(std::max(1e-6, shapeAmount));

    for (int i = 0; i < totalSamples; ++i)
    {
        double carrier = std::sin(carrierPhase);
        double lfo = std::sin(modPhase);
        double shapeLFO = 1.0 - modDepth * (1.0 - lfo) * 0.5;
        double modulated = carrier * shapeLFO;
        double shaped = std::tanh(modulated * shapeAmount) / tanhShape;
        float s = static_cast<float>(shaped * amp);
        buffer.setSample(0, i, s * gains.first);
        buffer.setSample(1, i, s * gains.second);

        carrierPhase += MathConstants<double>::twoPi * carrierFreq * dt;
        modPhase += MathConstants<double>::twoPi * modFreq * dt;
    }

    return buffer;
}

AudioBuffer<float> rhythmicWaveshapingTransition(double duration, double sampleRate, const NamedValueSet& params)
{
    double amp = params.getWithDefault("amp", 0.25);
    double startCarrierFreq = params.getWithDefault("startCarrierFreq", 200.0);
    double endCarrierFreq = params.getWithDefault("endCarrierFreq", 80.0);
    double startModFreq = params.getWithDefault("startModFreq", 12.0);
    double endModFreq = params.getWithDefault("endModFreq", 7.83);
    double startModDepth = params.getWithDefault("startModDepth", 1.0);
    double endModDepth = params.getWithDefault("endModDepth", 1.0);
    double startShapeAmount = params.getWithDefault("startShapeAmount", 5.0);
    double endShapeAmount = params.getWithDefault("endShapeAmount", 5.0);
    double pan = params.getWithDefault("pan", 0.0);

    double initialOffset = params.getWithDefault("initial_offset", 0.0);
    double postOffset = params.getWithDefault("post_offset", 0.0);
    String curve = params.getWithDefault("transition_curve", "linear");

    int totalSamples = static_cast<int>(duration * sampleRate);
    AudioBuffer<float> buffer(2, totalSamples);
    auto alpha = calculateTransitionAlpha(duration, sampleRate, initialOffset, postOffset, curve);
    auto gains = getPanGains(pan);

    double dt = 1.0 / sampleRate;
    double carrierPhase = 0.0;
    double modPhase = 0.0;

    for (int i = 0; i < totalSamples; ++i)
    {
        double a = alpha.empty() ? static_cast<double>(i) / (totalSamples > 1 ? totalSamples - 1 : 1)
                                 : alpha[i];

        double carrierFreq = startCarrierFreq + (endCarrierFreq - startCarrierFreq) * a;
        double modFreq = startModFreq + (endModFreq - startModFreq) * a;
        double modDepth = startModDepth + (endModDepth - startModDepth) * a;
        double shapeAmount = startShapeAmount + (endShapeAmount - startShapeAmount) * a;
        double tanhShape = std::tanh(std::max(1e-6, shapeAmount));

        double carrier = std::sin(carrierPhase);
        double lfo = std::sin(modPhase);
        double shapeLFO = 1.0 - modDepth * (1.0 - lfo) * 0.5;
        double modulated = carrier * shapeLFO;
        double shaped = std::tanh(modulated * shapeAmount) / tanhShape;
        float s = static_cast<float>(shaped * amp);
        buffer.setSample(0, i, s * gains.first);
        buffer.setSample(1, i, s * gains.second);

        carrierPhase += MathConstants<double>::twoPi * carrierFreq * dt;
        modPhase += MathConstants<double>::twoPi * modFreq * dt;
    }

    return buffer;
}

#include "SpatialAngleModulation.h"
#include "AudioUtils.h"

using namespace juce;

AudioBuffer<float> spatialAngleModulation(double duration, double sampleRate, const NamedValueSet& params)
{
    const int N = static_cast<int>(duration * sampleRate);
    AudioBuffer<float> buffer(2, jmax(0, N));
    if (N <= 0)
        return buffer;

    const double amp         = params.getWithDefault("amp", 0.7);
    const double carrierFreq = params.getWithDefault("carrierFreq", 440.0);
    const double beatFreq    = params.getWithDefault("beatFreq", 4.0);
    const double radius      = params.getWithDefault("pathRadius", 1.0);
    const double startDeg    = params.getWithDefault("arcStartDeg", 0.0);
    const double endDeg      = params.getWithDefault("arcEndDeg", 360.0);

    const double dt = 1.0 / sampleRate;
    double phCarrier = 0.0;
    double phBeat = 0.0;
    for (int i = 0; i < N; ++i)
    {
        double t = static_cast<double>(i) / sampleRate;
        double angle = juce::degreesToRadians(startDeg + (endDeg - startDeg) * (t / duration));
        double pan = std::sin(angle) * radius;
        auto gains = getPanGains(pan);

        double leftBeat  = std::sin(phCarrier - phBeat * 0.5);
        double rightBeat = std::sin(phCarrier + phBeat * 0.5);
        double mono = (leftBeat + rightBeat) * 0.5;

        buffer.setSample(0, i, static_cast<float>(mono * amp * gains.first));
        buffer.setSample(1, i, static_cast<float>(mono * amp * gains.second));

        phCarrier += MathConstants<double>::twoPi * carrierFreq * dt;
        phBeat += MathConstants<double>::twoPi * beatFreq * dt;
    }

    return buffer;
}

AudioBuffer<float> spatialAngleModulationTransition(double duration, double sampleRate, const NamedValueSet& params)
{
    const int N = static_cast<int>(duration * sampleRate);
    AudioBuffer<float> buffer(2, jmax(0, N));
    if (N <= 0)
        return buffer;

    const double initialOffset = params.getWithDefault("initial_offset", 0.0);
    const double postOffset    = params.getWithDefault("post_offset", 0.0);
    const String curve         = params.getWithDefault("transition_curve", "linear");
    auto alpha = calculateTransitionAlpha(duration, sampleRate, initialOffset, postOffset, curve);

    const double sAmp         = params.getWithDefault("startAmp", params.getWithDefault("amp", 0.7));
    const double eAmp         = params.getWithDefault("endAmp", sAmp);
    const double sCarrierFreq = params.getWithDefault("startCarrierFreq", params.getWithDefault("carrierFreq", 440.0));
    const double eCarrierFreq = params.getWithDefault("endCarrierFreq", sCarrierFreq);
    const double sBeatFreq    = params.getWithDefault("startBeatFreq", params.getWithDefault("beatFreq", 4.0));
    const double eBeatFreq    = params.getWithDefault("endBeatFreq", sBeatFreq);
    const double sRadius      = params.getWithDefault("startPathRadius", params.getWithDefault("pathRadius", 1.0));
    const double eRadius      = params.getWithDefault("endPathRadius", sRadius);
    const double sDeg         = params.getWithDefault("startArcStartDeg", params.getWithDefault("arcStartDeg", 0.0));
    const double eDeg         = params.getWithDefault("endArcEndDeg", params.getWithDefault("arcEndDeg", 360.0));

    const double dt = 1.0 / sampleRate;
    double phCarrier = 0.0;
    double phBeat = 0.0;
    for (int i = 0; i < N; ++i)
    {
        double a = alpha.empty() ? static_cast<double>(i) / (N > 1 ? N - 1 : 1) : alpha[i];
        double amp    = sAmp + (eAmp - sAmp) * a;
        double cfreq  = sCarrierFreq + (eCarrierFreq - sCarrierFreq) * a;
        double bfreq  = sBeatFreq + (eBeatFreq - sBeatFreq) * a;
        double radius = sRadius + (eRadius - sRadius) * a;
        double deg    = sDeg + (eDeg - sDeg) * a;
        double pan    = std::sin(juce::degreesToRadians(deg)) * radius;
        auto gains = getPanGains(pan);

        double leftBeat  = std::sin(phCarrier - phBeat * 0.5);
        double rightBeat = std::sin(phCarrier + phBeat * 0.5);
        double mono = (leftBeat + rightBeat) * 0.5;

        buffer.setSample(0, i, static_cast<float>(mono * amp * gains.first));
        buffer.setSample(1, i, static_cast<float>(mono * amp * gains.second));

        phCarrier += MathConstants<double>::twoPi * cfreq * dt;
        phBeat += MathConstants<double>::twoPi * bfreq * dt;
    }

    return buffer;
}

AudioBuffer<float> spatialAngleModulationMonauralBeat(double duration, double sampleRate, const NamedValueSet& params)
{
    const int N = static_cast<int>(duration * sampleRate);
    AudioBuffer<float> buffer(2, jmax(0, N));
    if (N <= 0)
        return buffer;

    // Generate monaural beat first
    NamedValueSet beatParams;
    for (const auto& k : {"amp_lower_L","amp_upper_L","amp_lower_R","amp_upper_R","baseFreq","beatFreq","startPhaseL","startPhaseR","phaseOscFreq","phaseOscRange","ampOscDepth","ampOscFreq","ampOscPhaseOffset"})
        if (params.contains(k)) beatParams.set(k, params[k]);
    auto beat = monauralBeatStereoAmps(duration, sampleRate, beatParams);
    std::vector<float> mono(N);
    for (int i = 0; i < N; ++i)
        mono[i] = 0.5f * (beat.getSample(0, i) + beat.getSample(1, i));

    const double aOD = params.getWithDefault("sam_ampOscDepth", 0.0);
    const double aOF = params.getWithDefault("sam_ampOscFreq", 0.0);
    const double aOP = params.getWithDefault("sam_ampOscPhaseOffset", 0.0);
    const double spatialFreq = params.getWithDefault("spatialBeatFreq", params.getWithDefault("beatFreq", 4.0));
    const double radius      = params.getWithDefault("pathRadius", 1.0);

    double phase = 0.0;
    const double dt = 1.0 / sampleRate;
    for (int i = 0; i < N; ++i)
    {
        double env = 1.0;
        if (aOD != 0.0 && aOF != 0.0)
        {
            double depth = std::clamp(aOD, 0.0, 2.0);
            env = (1.0 - depth * 0.5) + (depth * 0.5) * std::sin(MathConstants<double>::twoPi * aOF * i * dt + aOP);
        }
        double pan = std::sin(phase) * radius;
        auto gains = getPanGains(pan);
        float v = mono[i] * static_cast<float>(env);
        buffer.setSample(0, i, v * gains.first);
        buffer.setSample(1, i, v * gains.second);
        phase += MathConstants<double>::twoPi * spatialFreq * dt;
    }

    return buffer;
}

AudioBuffer<float> spatialAngleModulationMonauralBeatTransition(double duration, double sampleRate, const NamedValueSet& params)
{
    const int N = static_cast<int>(duration * sampleRate);
    AudioBuffer<float> buffer(2, jmax(0, N));
    if (N <= 0)
        return buffer;

    const double initialOffset = params.getWithDefault("initial_offset", 0.0);
    const double postOffset    = params.getWithDefault("post_offset", 0.0);
    const String curve         = params.getWithDefault("transition_curve", "linear");
    auto alpha = calculateTransitionAlpha(duration, sampleRate, initialOffset, postOffset, curve);

    NamedValueSet beatParams;
    for (const auto& k : {"start_amp_lower_L","end_amp_lower_L","start_amp_upper_L","end_amp_upper_L","start_amp_lower_R","end_amp_lower_R","start_amp_upper_R","end_amp_upper_R","startBaseFreq","endBaseFreq","startBeatFreq","endBeatFreq","startStartPhaseL","endStartPhaseL","startStartPhaseU","endStartPhaseU","startPhaseOscFreq","endPhaseOscFreq","startPhaseOscRange","endPhaseOscRange","startAmpOscDepth","endAmpOscDepth","startAmpOscFreq","endAmpOscFreq","startAmpOscPhaseOffset","endAmpOscPhaseOffset"})
        if (params.contains(k)) beatParams.set(k, params[k]);
    auto beat = monauralBeatStereoAmpsTransition(duration, sampleRate, beatParams);
    std::vector<float> mono(N);
    for (int i = 0; i < N; ++i)
        mono[i] = 0.5f * (beat.getSample(0, i) + beat.getSample(1, i));

    const double sAOD = params.getWithDefault("start_sam_ampOscDepth", params.getWithDefault("sam_ampOscDepth", 0.0));
    const double eAOD = params.getWithDefault("end_sam_ampOscDepth", sAOD);
    const double sAOF = params.getWithDefault("start_sam_ampOscFreq", params.getWithDefault("sam_ampOscFreq", 0.0));
    const double eAOF = params.getWithDefault("end_sam_ampOscFreq", sAOF);
    const double sAOP = params.getWithDefault("start_sam_ampOscPhaseOffset", params.getWithDefault("sam_ampOscPhaseOffset", 0.0));
    const double eAOP = params.getWithDefault("end_sam_ampOscPhaseOffset", sAOP);
    const double sFreq = params.getWithDefault("startSpatialBeatFreq", params.getWithDefault("spatialBeatFreq", 4.0));
    const double eFreq = params.getWithDefault("endSpatialBeatFreq", sFreq);
    const double sRad  = params.getWithDefault("startPathRadius", params.getWithDefault("pathRadius", 1.0));
    const double eRad  = params.getWithDefault("endPathRadius", sRad);

    double phase = 0.0;
    const double dt = 1.0 / sampleRate;
    for (int i = 0; i < N; ++i)
    {
        double a = alpha.empty() ? static_cast<double>(i) / (N > 1 ? N - 1 : 1) : alpha[i];
        double depth = sAOD + (eAOD - sAOD) * a;
        double freq  = sAOF + (eAOF - sAOF) * a;
        double phOff = sAOP + (eAOP - sAOP) * a;
        double spatialF = sFreq + (eFreq - sFreq) * a;
        double r = sRad + (eRad - sRad) * a;

        double env = 1.0;
        if (depth != 0.0 && freq != 0.0)
        {
            double clamped = std::clamp(depth, 0.0, 2.0);
            env = (1.0 - clamped * 0.5) + (clamped * 0.5) * std::sin(MathConstants<double>::twoPi * freq * i * dt + phOff);
        }

        double pan = std::sin(phase) * r;
        auto gains = getPanGains(pan);
        float v = mono[i] * static_cast<float>(env);
        buffer.setSample(0, i, v * gains.first);
        buffer.setSample(1, i, v * gains.second);
        phase += MathConstants<double>::twoPi * spatialF * dt;
    }

    return buffer;
}

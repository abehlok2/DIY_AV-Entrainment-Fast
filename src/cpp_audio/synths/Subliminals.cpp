#include "Subliminals.h"
#include "AudioUtils.h"

using namespace juce;

AudioBuffer<float> subliminalEncode(double duration, double sampleRate, const NamedValueSet& params)
{
    const int N = static_cast<int>(duration * sampleRate);
    AudioBuffer<float> buffer(2, jmax(0, N));
    if (N <= 0)
        return buffer;

    const double carrier = std::clamp(params.getWithDefault("carrierFreq", 17500.0), 15000.0, 20000.0);
    const double amp = params.getWithDefault("amp", 0.5);
    String mode = params.getWithDefault("mode", "sequence");

    String audioPathsStr = params.getWithDefault("audio_paths", "");
    String audioPath = params.getWithDefault("audio_path", "");
    StringArray files;
    if (audioPathsStr.isNotEmpty())
        files.addTokens(audioPathsStr, ";", "");
    else if (audioPath.isNotEmpty())
        files.add(audioPath);

    if (files.isEmpty())
        return buffer;

    AudioFormatManager fm;
    fm.registerBasicFormats();

    std::vector<AudioBuffer<float>> segments;
    for (const auto& f : files)
    {
        std::unique_ptr<AudioFormatReader> reader(fm.createReaderFor(File(f)));
        if (! reader)
            continue;

        const int len = static_cast<int>(reader->lengthInSamples);
        AudioBuffer<float> monoBuf(1, len);
        reader->read(&monoBuf, 0, len, 0, true, true);
        if (reader->sampleRate != sampleRate && len > 0)
        {
            LagrangeInterpolator resamp;
            const int newLen = static_cast<int>(len * sampleRate / reader->sampleRate);
            AudioBuffer<float> tmp(1, newLen);
            resamp.reset();
            resamp.process(reader->sampleRate / sampleRate, monoBuf.getReadPointer(0), tmp.getWritePointer(0), newLen);
            monoBuf = std::move(tmp);
        }

        const int segN = monoBuf.getNumSamples();
        if (segN <= 0)
            continue;

        AudioBuffer<float> modBuf(1, segN);
        double phase = 0.0;
        const double inc = MathConstants<double>::twoPi * carrier / sampleRate;
        for (int i = 0; i < segN; ++i)
        {
            float s = monoBuf.getSample(0, i);
            modBuf.setSample(0, i, s * std::sin(phase));
            phase += inc;
        }

        float maxAbs = 0.0f;
        for (int i = 0; i < segN; ++i)
            maxAbs = std::max(maxAbs, std::abs(modBuf.getSample(0, i)));
        if (maxAbs > 1e-6f)
            modBuf.applyGain(1.0f / maxAbs);
        segments.push_back(std::move(modBuf));
    }

    if (segments.empty())
        return buffer;

    buffer.clear();
    if (mode == "stack")
    {
        for (const auto& seg : segments)
        {
            const int segN = seg.getNumSamples();
            for (int i = 0; i < N; ++i)
            {
                float v = seg.getSample(0, i % segN);
                buffer.addSample(0, i, v);
                buffer.addSample(1, i, v);
            }
        }
        buffer.applyGain(1.0f / segments.size());
    }
    else
    {
        int pos = 0;
        int idx = 0;
        const int pauseSamples = static_cast<int>(sampleRate);
        while (pos < N)
        {
            const auto& seg = segments[idx % segments.size()];
            const int segN = seg.getNumSamples();
            const int copyLen = std::min(segN, N - pos);
            for (int i = 0; i < copyLen; ++i)
            {
                float v = seg.getSample(0, i);
                buffer.setSample(0, pos + i, v);
                buffer.setSample(1, pos + i, v);
            }
            pos += copyLen;
            if (pos >= N)
                break;
            const int pauseLen = std::min(pauseSamples, N - pos);
            pos += pauseLen;
            ++idx;
        }
    }

    buffer.applyGain(static_cast<float>(amp));
    return buffer;
}

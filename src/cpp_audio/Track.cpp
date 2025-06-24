#include "Track.h"
#include <juce_data_structures/juce_data_structures.h>
#include <juce_audio_formats/juce_audio_formats.h>

Track loadTrackFromJson(const juce::File& file)
{
    Track track;
    auto stream = file.createInputStream();
    if (!stream)
        return track;

    juce::var parsed = juce::JSON::parse(stream->readEntireStreamAsString());
    if (auto* obj = parsed.getDynamicObject())
    {
        if (auto* gs = obj->getProperty("global_settings").getDynamicObject())
        {
            track.settings.sampleRate = gs->getProperty("sample_rate", 44100.0);
            track.settings.crossfadeDuration = gs->getProperty("crossfade_duration", 1.0);
            track.settings.crossfadeCurve = gs->getProperty("crossfade_curve").toString();
        }

        if (auto* stepsVar = obj->getProperty("steps").getArray())
        {
            for (const auto& s : *stepsVar)
            {
                Step step;
                if (auto* sobj = s.getDynamicObject())
                {
                    step.durationSeconds = sobj->getProperty("duration", 0.0);
                    if (auto* voicesVar = sobj->getProperty("voices").getArray())
                    {
                        for (const auto& v : *voicesVar)
                        {
                            Voice voice;
                            if (auto* vobj = v.getDynamicObject())
                            {
                                voice.synthFunction = vobj->getProperty("synth_function_name").toString().toStdString();
                                voice.isTransition = vobj->getProperty("is_transition", false);
                                if (auto* paramsObj = vobj->getProperty("params").getDynamicObject())
                                    voice.params = *paramsObj;
                            }
                            step.voices.push_back(std::move(voice));
                        }
                    }
                }
                track.steps.push_back(std::move(step));
            }
        }
    }
    return track;
}

bool writeWavFile(const juce::File& file, const juce::AudioBuffer<float>& buffer, double sampleRate)
{
    juce::WavAudioFormat format;
    std::unique_ptr<juce::FileOutputStream> stream(file.createOutputStream());
    if (!stream)
        return false;
    std::unique_ptr<juce::AudioFormatWriter> writer(format.createWriterFor(stream.get(), sampleRate, buffer.getNumChannels(), 16, {}, 0));
    if (!writer)
        return false;
    stream.release();
    return writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
}


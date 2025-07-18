#include "TrackXml.h"
#include "VarUtils.h"
#include <juce_data_structures/juce_data_structures.h>

Track loadTrackFromXml(const juce::File& file)
{
    Track track;
    std::unique_ptr<juce::XmlElement> root = juce::XmlDocument::parse(file);
    if (!root)
        return track;

    if (auto* gs = root->getChildByName("global_settings"))
    {
        track.settings.sampleRate = gs->getDoubleAttribute("sample_rate", 44100.0);
        track.settings.crossfadeDuration = gs->getDoubleAttribute("crossfade_duration", 1.0);
        track.settings.crossfadeCurve = gs->getStringAttribute("crossfade_curve", "linear");
        track.settings.outputFilename = gs->getStringAttribute("output_filename", "my_track.wav");
    }

    if (auto* bg = root->getChildByName("background_noise"))
    {
        track.backgroundNoise.filePath = bg->getStringAttribute("file_path");
        track.backgroundNoise.amp = bg->getDoubleAttribute("amp", 0.0);
        track.backgroundNoise.pan = bg->getDoubleAttribute("pan", 0.0);
        track.backgroundNoise.startTime = bg->getDoubleAttribute("start_time", 0.0);
        track.backgroundNoise.fadeIn = bg->getDoubleAttribute("fade_in", 0.0);
        track.backgroundNoise.fadeOut = bg->getDoubleAttribute("fade_out", 0.0);
    }

    if (auto* clips = root->getChildByName("clips"))
    {
        forEachXmlChildElement(*clips, c)
        {
            if (c->hasTagName("clip"))
            {
                Clip clip;
                clip.filePath = c->getStringAttribute("file_path");
                clip.description = c->getStringAttribute("description");
                clip.start = c->getDoubleAttribute("start", 0.0);
                clip.duration = c->getDoubleAttribute("duration", 0.0);
                clip.amp = c->getDoubleAttribute("amp", 1.0);
                clip.pan = c->getDoubleAttribute("pan", 0.0);
                clip.fadeIn = c->getDoubleAttribute("fade_in", 0.0);
                clip.fadeOut = c->getDoubleAttribute("fade_out", 0.0);
                track.clips.push_back(std::move(clip));
            }
        }
    }

    if (auto* steps = root->getChildByName("steps"))
    {
        forEachXmlChildElement(*steps, s)
        {
            if (s->hasTagName("step"))
            {
                Step step;
                step.durationSeconds = s->getDoubleAttribute("duration", 0.0);
                step.description = s->getStringAttribute("description");
                if (auto* voices = s->getChildByName("voices"))
                {
                    forEachXmlChildElement(*voices, v)
                    {
                        if (v->hasTagName("voice"))
                        {
                            Voice voice;
                            voice.synthFunction = v->getStringAttribute("synth_function_name").toStdString();
                            voice.isTransition = v->getBoolAttribute("is_transition", false);
                            if (auto* params = v->getChildByName("params"))
                            {
                                forEachXmlChildElement(*params, p)
                                {
                                    voice.params.set(p->getTagName(), p->getAllSubText());
                                }
                            }
                            voice.description = v->getStringAttribute("description");
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

bool saveTrackToXml(const Track& track, const juce::File& file)
{
    auto root = std::make_unique<juce::XmlElement>("track");
    auto* gs = root->createNewChildElement("global_settings");
    gs->setAttribute("sample_rate", track.settings.sampleRate);
    gs->setAttribute("crossfade_duration", track.settings.crossfadeDuration);
    gs->setAttribute("crossfade_curve", track.settings.crossfadeCurve);
    gs->setAttribute("output_filename", track.settings.outputFilename);

    auto* bg = root->createNewChildElement("background_noise");
    bg->setAttribute("file_path", track.backgroundNoise.filePath);
    bg->setAttribute("amp", track.backgroundNoise.amp);
    bg->setAttribute("pan", track.backgroundNoise.pan);
    bg->setAttribute("start_time", track.backgroundNoise.startTime);
    bg->setAttribute("fade_in", track.backgroundNoise.fadeIn);
    bg->setAttribute("fade_out", track.backgroundNoise.fadeOut);

    if (!track.clips.empty())
    {
        auto* clips = root->createNewChildElement("clips");
        for (const auto& clip : track.clips)
        {
            auto* c = clips->createNewChildElement("clip");
            c->setAttribute("file_path", clip.filePath);
            c->setAttribute("description", clip.description);
            c->setAttribute("start", clip.start);
            c->setAttribute("duration", clip.duration);
            c->setAttribute("amp", clip.amp);
            c->setAttribute("pan", clip.pan);
            c->setAttribute("fade_in", clip.fadeIn);
            c->setAttribute("fade_out", clip.fadeOut);
        }
    }

    auto* steps = root->createNewChildElement("steps");
    for (const auto& step : track.steps)
    {
        auto* s = steps->createNewChildElement("step");
        s->setAttribute("duration", step.durationSeconds);
        s->setAttribute("description", step.description);
        auto* voices = s->createNewChildElement("voices");
        for (const auto& voice : step.voices)
        {
            auto* v = voices->createNewChildElement("voice");
            v->setAttribute("synth_function_name", voice.synthFunction);
            v->setAttribute("is_transition", voice.isTransition);
            v->setAttribute("description", voice.description);
            if (!voice.params.isEmpty())
            {
                auto* params = v->createNewChildElement("params");
                for (const auto& p : voice.params)
                {
                    auto* param = params->createNewChildElement(p.name.toString());
                    param->addTextElement(p.value.toString());
                }
            }
        }
    }

    return root->writeToFile(file, {});
}

#include "Track.h"
#include "AudioUtils.h"
#include "SynthFunctions.h"
#include <juce_data_structures/juce_data_structures.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_dsp/juce_dsp.h>
#include <map>

using SynthFunc = juce::AudioBuffer<float>(*)(double, double, const juce::NamedValueSet&);

static std::map<juce::String, SynthFunc> synthMap{
    { "binaural_beat", binauralBeat },
    { "binaural_beat_transition", binauralBeatTransition },
    { "isochronic_tone", isochronicTone },
    { "isochronic_tone_transition", isochronicToneTransition },
    { "rhythmic_waveshaping", rhythmicWaveshaping },
    { "rhythmic_waveshaping_transition", rhythmicWaveshapingTransition },
    { "stereo_am_independent", stereoAMIndependent },
    { "stereo_am_independent_transition", stereoAMIndependentTransition },
    { "wave_shape_stereo_am", waveShapeStereoAm },
    { "wave_shape_stereo_am_transition", waveShapeStereoAmTransition },
    { "monaural_beat_stereo_amps", monauralBeatStereoAmps },
    { "monaural_beat_stereo_amps_transition", monauralBeatStereoAmpsTransition },
    { "qam_beat", qamBeat },
    { "qam_beat_transition", qamBeatTransition },
    { "hybrid_qam_monaural_beat", hybridQamMonauralBeat },
    { "hybrid_qam_monaural_beat_transition", hybridQamMonauralBeatTransition },
    { "spatial_angle_modulation", spatialAngleModulation },
    { "spatial_angle_modulation_transition", spatialAngleModulationTransition },
    { "spatial_angle_modulation_monaural_beat", spatialAngleModulationMonauralBeat },
    { "spatial_angle_modulation_monaural_beat_transition", spatialAngleModulationMonauralBeatTransition },
    { "generate_swept_notch_pink_sound", generateSweptNotchPinkSound },
    { "generate_swept_notch_pink_sound_transition", generateSweptNotchPinkSoundTransition },
    { "subliminal_encode", subliminalEncode }
};

static juce::AudioBuffer<float> resampleBuffer(const juce::AudioBuffer<float>& in,
                                               double srcRate,
                                               double dstRate)
{
    if (std::abs(srcRate - dstRate) < 1e-6)
        return in;

    int destSamples = static_cast<int>(in.getNumSamples() * dstRate / srcRate);
    juce::AudioBuffer<float> out(in.getNumChannels(), destSamples);
    for (int ch = 0; ch < in.getNumChannels(); ++ch)
    {
        juce::LagrangeInterpolator interp;
        interp.reset();
        interp.process(srcRate / dstRate,
                       in.getReadPointer(ch),
                       out.getWritePointer(ch),
                       destSamples);
    }
    return out;
}

static juce::AudioBuffer<float> loadAudioFile(const juce::File& file,
                                              double sampleRate)
{
    juce::AudioFormatManager fm;
    fm.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(fm.createReaderFor(file));
    if (! reader)
        return {};

    juce::AudioBuffer<float> buf(reader->numChannels,
                                 static_cast<int>(reader->lengthInSamples));
    reader->read(&buf, 0, buf.getNumSamples(), 0, true, true);

    if (reader->sampleRate != sampleRate && reader->sampleRate > 0)
        buf = resampleBuffer(buf, reader->sampleRate, sampleRate);

    if (buf.getNumChannels() == 1)
    {
        buf.setSize(2, buf.getNumSamples(), true, true, true);
        buf.copyFrom(1, 0, buf, 0, 0, buf.getNumSamples());
    }
    return buf;
}

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
            track.settings.outputFilename = gs->getProperty("output_filename", "my_track.wav").toString();
        }

        if (auto* bg = obj->getProperty("background_noise").getDynamicObject())
        {
            track.backgroundNoise.filePath = bg->getProperty("file_path").toString();
            track.backgroundNoise.amp = bg->getProperty("amp", 0.0);
            track.backgroundNoise.pan = bg->getProperty("pan", 0.0);
            track.backgroundNoise.startTime = bg->getProperty("start_time", 0.0);
            track.backgroundNoise.fadeIn = bg->getProperty("fade_in", 0.0);
            track.backgroundNoise.fadeOut = bg->getProperty("fade_out", 0.0);
            if (auto* envArr = bg->getProperty("amp_envelope").getArray())
            {
                for (const auto& p : *envArr)
                {
                    if (auto* pair = p.getArray())
                    {
                        if (pair->size() >= 2)
                            track.backgroundNoise.ampEnvelope.emplace_back(
                                (*pair)[0], (*pair)[1]);
                    }
                }
            }
        }

        if (auto* clipsArr = obj->getProperty("clips").getArray())
        {
            for (const auto& c : *clipsArr)
            {
                Clip clip;
                if (auto* cobj = c.getDynamicObject())
                {
                    clip.filePath = cobj->getProperty("file_path").toString();
                    clip.description = cobj->getProperty("description").toString();
                    clip.start = cobj->getProperty("start", cobj->getProperty("start_time", 0.0));
                    clip.duration = cobj->getProperty("duration", 0.0);
                    clip.amp = cobj->getProperty("amp", cobj->getProperty("gain", 1.0));
                    clip.pan = cobj->getProperty("pan", 0.0);
                    clip.fadeIn = cobj->getProperty("fade_in", 0.0);
                    clip.fadeOut = cobj->getProperty("fade_out", 0.0);
                }
                track.clips.push_back(std::move(clip));
            }
        }

        if (auto* stepsVar = obj->getProperty("steps").getArray())
        {
            for (const auto& s : *stepsVar)
            {
                Step step;
                if (auto* sobj = s.getDynamicObject())
                {
                    step.durationSeconds = sobj->getProperty("duration", 0.0);
                    step.description = sobj->getProperty("description").toString();
                    if (auto* voicesVar = sobj->getProperty("voices").getArray())
                    {
                        for (const auto& v : *voicesVar)
                        {
                            Voice voice;
                            if (auto* vobj = v.getDynamicObject())
                            {
                                voice.synthFunction =
                                    vobj->getProperty("synth_function_name").toString().toStdString();
                                voice.isTransition = vobj->getProperty("is_transition", false);
                                if (auto* paramsObj =
                                        vobj->getProperty("params").getDynamicObject())
                                    voice.params = paramsObj->getProperties();
                                voice.description =
                                    vobj->getProperty("description").toString();
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

static juce::var namedValueSetToVar(const juce::NamedValueSet& set)
{
    auto* obj = new juce::DynamicObject();
    for (const auto& p : set)
        obj->setProperty(p.name, p.value);
    return juce::var(obj);
}

bool saveTrackToJson(const Track& track, const juce::File& file)
{
    juce::File target = file;
    if (target.getFileExtension() != ".json")
        target = target.withFileExtension(".json");

    auto* obj = new juce::DynamicObject();

    // Global settings
    {
        auto* gs = new juce::DynamicObject();
        gs->setProperty("sample_rate", track.settings.sampleRate);
        gs->setProperty("crossfade_duration", track.settings.crossfadeDuration);
        gs->setProperty("crossfade_curve", track.settings.crossfadeCurve);
        gs->setProperty("output_filename", track.settings.outputFilename);
        obj->setProperty("global_settings", juce::var(gs));
    }

    // Background noise
    {
        auto* bg = new juce::DynamicObject();
        bg->setProperty("file_path", track.backgroundNoise.filePath);
        bg->setProperty("amp", track.backgroundNoise.amp);
        bg->setProperty("pan", track.backgroundNoise.pan);
        bg->setProperty("start_time", track.backgroundNoise.startTime);
        bg->setProperty("fade_in", track.backgroundNoise.fadeIn);
        bg->setProperty("fade_out", track.backgroundNoise.fadeOut);
        juce::Array<juce::var> env;
        for (const auto& p : track.backgroundNoise.ampEnvelope)
        {
            juce::Array<juce::var> pair;
            pair.add(p.first);
            pair.add(p.second);
            env.add(pair);
        }
        bg->setProperty("amp_envelope", env);
        obj->setProperty("background_noise", juce::var(bg));
    }

    // Clips
    {
        juce::Array<juce::var> clipsVar;
        for (const auto& clip : track.clips)
        {
            auto* cobj = new juce::DynamicObject();
            cobj->setProperty("file_path", clip.filePath);
            cobj->setProperty("description", clip.description);
            cobj->setProperty("start", clip.start);
            cobj->setProperty("duration", clip.duration);
            cobj->setProperty("amp", clip.amp);
            cobj->setProperty("pan", clip.pan);
            cobj->setProperty("fade_in", clip.fadeIn);
            cobj->setProperty("fade_out", clip.fadeOut);
            clipsVar.add(juce::var(cobj));
        }
        obj->setProperty("clips", clipsVar);
    }

    // Steps
    {
        juce::Array<juce::var> stepsVar;
        for (const auto& step : track.steps)
        {
            auto* sobj = new juce::DynamicObject();
            sobj->setProperty("duration", step.durationSeconds);
            sobj->setProperty("description", step.description);
            juce::Array<juce::var> voicesVar;
            for (const auto& voice : step.voices)
            {
                auto* vobj = new juce::DynamicObject();
                vobj->setProperty("synth_function_name",
                                   juce::String(voice.synthFunction));
                vobj->setProperty("is_transition", voice.isTransition);
                vobj->setProperty("params", namedValueSetToVar(voice.params));
                vobj->setProperty("description", voice.description);
                voicesVar.add(juce::var(vobj));
            }
            sobj->setProperty("voices", voicesVar);
            stepsVar.add(juce::var(sobj));
        }
        obj->setProperty("steps", stepsVar);
    }

    juce::String jsonString = juce::JSON::toString(juce::var(obj), true);
    return target.replaceWithText(jsonString);
}

int loadExternalStepsFromJson(const juce::File& file, std::vector<Step>& steps)
{
    auto stream = file.createInputStream();
    if (!stream)
        return 0;

    juce::var parsed = juce::JSON::parse(stream->readEntireStreamAsString());
    if (auto* obj = parsed.getDynamicObject())
    {
        if (auto* stepsVar = obj->getProperty("steps").getArray())
        {
            int loaded = 0;
            for (const auto& s : *stepsVar)
            {
                if (auto* sobj = s.getDynamicObject())
                {
                    if (!sobj->hasProperty("duration") || !sobj->hasProperty("voices"))
                        continue;

                    Step step;
                    step.durationSeconds = sobj->getProperty("duration", 0.0);
                    step.description = sobj->getProperty("description").toString();

                    if (auto* voicesVar = sobj->getProperty("voices").getArray())
                    {
                        for (const auto& v : *voicesVar)
                        {
                            Voice voice;
                            if (auto* vobj = v.getDynamicObject())
                            {
                                voice.synthFunction =
                                    vobj->getProperty("synth_function_name").toString().toStdString();
                                voice.isTransition = vobj->getProperty("is_transition", false);
                                if (auto* paramsObj =
                                        vobj->getProperty("params").getDynamicObject())
                                    voice.params = paramsObj->getProperties();
                                voice.description =
                                    vobj->getProperty("description").toString();
                            }
                            step.voices.push_back(std::move(voice));
                        }
                    }

                    steps.push_back(std::move(step));
                    ++loaded;
                }
            }
            return loaded;
        }
    }
    return 0;
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

juce::AudioBuffer<float> assembleTrack(const Track& track)
{
    double sampleRate = track.settings.sampleRate;
    double crossfadeDuration = track.settings.crossfadeDuration;
    int crossfadeSamples = static_cast<int>(crossfadeDuration * sampleRate);

    double totalDuration = 0.0;
    for (const auto& step : track.steps)
        totalDuration += step.durationSeconds;

    int estimatedSamples = static_cast<int>(totalDuration * sampleRate) + static_cast<int>(sampleRate);
    juce::AudioBuffer<float> buffer(2, estimatedSamples);
    buffer.clear();

    double currentTime = 0.0;
    int lastStepEnd = 0;

    for (size_t i = 0; i < track.steps.size(); ++i)
    {
        const auto& step = track.steps[i];
        int stepSamples = static_cast<int>(step.durationSeconds * sampleRate);
        if (stepSamples <= 0)
            continue;

        juce::AudioBuffer<float> stepBuf(2, stepSamples);
        stepBuf.clear();

        for (const auto& voice : step.voices)
        {
            auto it = synthMap.find(voice.synthFunction);
            if (it != synthMap.end())
            {
                juce::AudioBuffer<float> voiceBuf = it->second(step.durationSeconds, sampleRate, voice.params);
                for (int ch = 0; ch < 2; ++ch)
                    stepBuf.addFrom(ch, 0, voiceBuf, ch, 0, voiceBuf.getNumSamples());
            }
        }

        float peak = 0.0f;
        for (int ch = 0; ch < stepBuf.getNumChannels(); ++ch)
            peak = std::max(peak, stepBuf.getMagnitude(ch, 0, stepBuf.getNumSamples()));
        if (peak > 1.0f)
            stepBuf.applyGain(1.0f / peak);

        int stepStart = static_cast<int>(currentTime * sampleRate);
        int stepEnd   = stepStart + stepSamples;
        int safeStart = std::max(0, stepStart);
        int safeEnd   = std::min(stepEnd, estimatedSamples);
        int lenToPlace = safeEnd - safeStart;
        if (lenToPlace <= 0)
            continue;

        int overlapStart = safeStart;
        int overlapEnd   = std::min(safeEnd, lastStepEnd);
        int overlap      = overlapEnd - overlapStart;
        bool doCrossfade = (i > 0 && crossfadeSamples > 0 && overlap > 0);

        if (doCrossfade)
        {
            int actual = std::min(overlap, crossfadeSamples);
            juce::AudioBuffer<float> prevSeg(2, actual);
            prevSeg.copyFrom(0, 0, buffer, 0, overlapStart, actual);
            prevSeg.copyFrom(1, 0, buffer, 1, overlapStart, actual);

            juce::AudioBuffer<float> newSeg(2, actual);
            newSeg.copyFrom(0, 0, stepBuf, 0, 0, actual);
            newSeg.copyFrom(1, 0, stepBuf, 1, 0, actual);

            juce::AudioBuffer<float> blended = crossfade(prevSeg, newSeg,
                                                         static_cast<double>(actual) / sampleRate,
                                                         sampleRate,
                                                         track.settings.crossfadeCurve);

            buffer.copyFrom(0, overlapStart, blended, 0, 0, actual);
            buffer.copyFrom(1, overlapStart, blended, 1, 0, actual);

            int remainingStartStep = actual;
            int remainingStartTrack = overlapStart + actual;
            int remainingLen = lenToPlace - actual;
            if (remainingLen > 0)
            {
                buffer.addFrom(0, remainingStartTrack, stepBuf, 0, remainingStartStep, remainingLen);
                buffer.addFrom(1, remainingStartTrack, stepBuf, 1, remainingStartStep, remainingLen);
            }
        }
        else
        {
            buffer.addFrom(0, safeStart, stepBuf, 0, 0, lenToPlace);
            buffer.addFrom(1, safeStart, stepBuf, 1, 0, lenToPlace);
        }

        lastStepEnd = std::max(lastStepEnd, safeEnd);
        currentTime += step.durationSeconds - (crossfadeSamples > 0 ? crossfadeDuration : 0.0);
    }

    juce::AudioBuffer<float> finalBuf(2, lastStepEnd);
    finalBuf.clear();
    if (lastStepEnd > 0 && lastStepEnd <= buffer.getNumSamples())
    {
        finalBuf.copyFrom(0, 0, buffer, 0, 0, lastStepEnd);
        finalBuf.copyFrom(1, 0, buffer, 1, 0, lastStepEnd);
    }

    // Background noise
    if (track.backgroundNoise.filePath.isNotEmpty())
    {
        juce::AudioBuffer<float> noise =
            loadAudioFile(juce::File(track.backgroundNoise.filePath), sampleRate);
        int len = noise.getNumSamples();
        if (len > 0)
        {
            float leftGain, rightGain;
            std::tie(leftGain, rightGain) = getPanGains(track.backgroundNoise.pan);
            float amp = static_cast<float>(track.backgroundNoise.amp);
            noise.applyGain(0, 0, len, amp * leftGain);
            noise.applyGain(1, 0, len, amp * rightGain);

            if (track.backgroundNoise.fadeIn > 0.0)
            {
                int n = std::min(len,
                                 (int)(track.backgroundNoise.fadeIn * sampleRate));
                noise.applyGainRamp(0, 0, n, 0.0f, amp * leftGain);
                noise.applyGainRamp(1, 0, n, 0.0f, amp * rightGain);
            }
            if (track.backgroundNoise.fadeOut > 0.0)
            {
                int n = std::min(len,
                                 (int)(track.backgroundNoise.fadeOut * sampleRate));
                noise.applyGainRamp(0, len - n, n, amp * leftGain, 0.0f);
                noise.applyGainRamp(1, len - n, n, amp * rightGain, 0.0f);
            }

            int start = static_cast<int>(track.backgroundNoise.startTime * sampleRate);
            int end = start + len;
            if (end > finalBuf.getNumSamples())
                finalBuf.setSize(2, end, true, true, true);

            finalBuf.addFrom(0, start, noise, 0, 0, len);
            finalBuf.addFrom(1, start, noise, 1, 0, len);
            lastStepEnd = std::max(lastStepEnd, end);
        }
    }

    // Overlay clips
    for (const auto& clip : track.clips)
    {
        if (clip.filePath.isEmpty())
            continue;

        juce::AudioBuffer<float> clipBuf = loadAudioFile(juce::File(clip.filePath), sampleRate);
        int len = clipBuf.getNumSamples();
        if (len <= 0)
            continue;

        float leftGain, rightGain;
        std::tie(leftGain, rightGain) = getPanGains(clip.pan);
        float amp = static_cast<float>(clip.amp);
        clipBuf.applyGain(0, 0, len, amp * leftGain);
        clipBuf.applyGain(1, 0, len, amp * rightGain);

        if (clip.fadeIn > 0.0)
        {
            int n = std::min(len, (int)(clip.fadeIn * sampleRate));
            clipBuf.applyGainRamp(0, 0, n, 0.0f, amp * leftGain);
            clipBuf.applyGainRamp(1, 0, n, 0.0f, amp * rightGain);
        }
        if (clip.fadeOut > 0.0)
        {
            int n = std::min(len, (int)(clip.fadeOut * sampleRate));
            clipBuf.applyGainRamp(0, len - n, n, amp * leftGain, 0.0f);
            clipBuf.applyGainRamp(1, len - n, n, amp * rightGain, 0.0f);
        }

        int start = static_cast<int>(clip.start * sampleRate);
        int end = start + len;
        if (end > finalBuf.getNumSamples())
            finalBuf.setSize(2, end, true, true, true);

        finalBuf.addFrom(0, start, clipBuf, 0, 0, len);
        finalBuf.addFrom(1, start, clipBuf, 1, 0, len);
        lastStepEnd = std::max(lastStepEnd, end);
    }

    if (lastStepEnd < finalBuf.getNumSamples())
        finalBuf.setSize(2, lastStepEnd, true, true, true);

    return finalBuf;
}


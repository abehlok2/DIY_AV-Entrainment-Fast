#include "Track.h"
#include "AudioUtils.h"
#include "SynthFunctions.h"
#include <juce_data_structures/juce_data_structures.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <map>

using SynthFunc = juce::AudioBuffer<float>(*)(double, double, const juce::NamedValueSet&);

static std::map<juce::String, SynthFunc> synthMap{
    { "binaural_beat", binauralBeat },
    { "binaural_beat_transition", binauralBeatTransition },
    { "isochronic_tone", isochronicTone },
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
                    step.description = sobj->getProperty("description").toString();
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
                                voice.description = vobj->getProperty("description").toString();
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
    return finalBuf;
}


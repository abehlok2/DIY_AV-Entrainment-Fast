#include "NoiseParams.h"

using namespace juce;

bool saveNoiseParams(const NoiseParams& params, const File& file)
{
    File target = file;
    if (target.getFileExtension() != NOISE_FILE_EXTENSION)
        target = target.withFileExtension(NOISE_FILE_EXTENSION);

    auto* obj = new DynamicObject();
    obj->setProperty("duration_seconds", params.durationSeconds);
    obj->setProperty("sample_rate", params.sampleRate);
    obj->setProperty("noise_type", params.noiseType);
    obj->setProperty("lfo_waveform", params.lfoWaveform);
    obj->setProperty("transition", params.transition);
    obj->setProperty("lfo_freq", params.lfoFreq);
    obj->setProperty("start_lfo_freq", params.startLfoFreq);
    obj->setProperty("end_lfo_freq", params.endLfoFreq);
    obj->setProperty("sweeps", params.sweeps);
    obj->setProperty("start_lfo_phase_offset_deg", params.startLfoPhaseOffsetDeg);
    obj->setProperty("end_lfo_phase_offset_deg", params.endLfoPhaseOffsetDeg);
    obj->setProperty("start_intra_phase_offset_deg", params.startIntraPhaseOffsetDeg);
    obj->setProperty("end_intra_phase_offset_deg", params.endIntraPhaseOffsetDeg);
    obj->setProperty("initial_offset", params.initialOffset);
    obj->setProperty("post_offset", params.postOffset);
    obj->setProperty("input_audio_path", params.inputAudioPath);
    obj->setProperty("start_time", params.startTime);
    obj->setProperty("fade_in", params.fadeIn);
    obj->setProperty("fade_out", params.fadeOut);
    obj->setProperty("amp_envelope", params.ampEnvelope);

    var root(obj);
    String json = JSON::toString(root, true);
    return target.replaceWithText(json);
}

NoiseParams loadNoiseParams(const File& file)
{
    NoiseParams params;
    if (!file.existsAsFile())
        return params;

    var root = JSON::parse(file.loadFileAsString());
    if (auto* obj = root.getDynamicObject())
    {
        params.durationSeconds = obj->getProperty("duration_seconds", params.durationSeconds);
        params.sampleRate = obj->getProperty("sample_rate", params.sampleRate);
        params.noiseType = obj->getProperty("noise_type", params.noiseType).toString();
        params.lfoWaveform = obj->getProperty("lfo_waveform", params.lfoWaveform).toString();
        params.transition = obj->getProperty("transition", params.transition);
        params.lfoFreq = obj->getProperty("lfo_freq", params.lfoFreq);
        params.startLfoFreq = obj->getProperty("start_lfo_freq", params.startLfoFreq);
        params.endLfoFreq = obj->getProperty("end_lfo_freq", params.endLfoFreq);
        params.sweeps = obj->getProperty("sweeps");
        params.startLfoPhaseOffsetDeg = obj->getProperty("start_lfo_phase_offset_deg", params.startLfoPhaseOffsetDeg);
        params.endLfoPhaseOffsetDeg = obj->getProperty("end_lfo_phase_offset_deg", params.endLfoPhaseOffsetDeg);
        params.startIntraPhaseOffsetDeg = obj->getProperty("start_intra_phase_offset_deg", params.startIntraPhaseOffsetDeg);
        params.endIntraPhaseOffsetDeg = obj->getProperty("end_intra_phase_offset_deg", params.endIntraPhaseOffsetDeg);
        params.initialOffset = obj->getProperty("initial_offset", params.initialOffset);
        params.postOffset = obj->getProperty("post_offset", params.postOffset);
        params.inputAudioPath = obj->getProperty("input_audio_path", params.inputAudioPath).toString();
        params.startTime = obj->getProperty("start_time", params.startTime);
        params.fadeIn = obj->getProperty("fade_in", params.fadeIn);
        params.fadeOut = obj->getProperty("fade_out", params.fadeOut);
        params.ampEnvelope = obj->getProperty("amp_envelope");
    }
    return params;
}

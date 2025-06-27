#include "NoiseParams.h"
#include "../utils/VarUtils.h"

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
        params.durationSeconds = getPropertyWithDefault(obj, "duration_seconds", params.durationSeconds);
        params.sampleRate = getPropertyWithDefault(obj, "sample_rate", params.sampleRate);
        params.noiseType = getPropertyWithDefault(obj, "noise_type", params.noiseType).toString();
        params.lfoWaveform = getPropertyWithDefault(obj, "lfo_waveform", params.lfoWaveform).toString();
        params.transition = getPropertyWithDefault(obj, "transition", params.transition);
        params.lfoFreq = getPropertyWithDefault(obj, "lfo_freq", params.lfoFreq);
        params.startLfoFreq = getPropertyWithDefault(obj, "start_lfo_freq", params.startLfoFreq);
        params.endLfoFreq = getPropertyWithDefault(obj, "end_lfo_freq", params.endLfoFreq);
        params.sweeps = obj->getProperty("sweeps");
        params.startLfoPhaseOffsetDeg = getPropertyWithDefault(obj, "start_lfo_phase_offset_deg", params.startLfoPhaseOffsetDeg);
        params.endLfoPhaseOffsetDeg = getPropertyWithDefault(obj, "end_lfo_phase_offset_deg", params.endLfoPhaseOffsetDeg);
        params.startIntraPhaseOffsetDeg = getPropertyWithDefault(obj, "start_intra_phase_offset_deg", params.startIntraPhaseOffsetDeg);
        params.endIntraPhaseOffsetDeg = getPropertyWithDefault(obj, "end_intra_phase_offset_deg", params.endIntraPhaseOffsetDeg);
        params.initialOffset = getPropertyWithDefault(obj, "initial_offset", params.initialOffset);
        params.postOffset = getPropertyWithDefault(obj, "post_offset", params.postOffset);
        params.inputAudioPath = getPropertyWithDefault(obj, "input_audio_path", params.inputAudioPath).toString();
        params.startTime = getPropertyWithDefault(obj, "start_time", params.startTime);
        params.fadeIn = getPropertyWithDefault(obj, "fade_in", params.fadeIn);
        params.fadeOut = getPropertyWithDefault(obj, "fade_out", params.fadeOut);
        params.ampEnvelope = obj->getProperty("amp_envelope");
    }
    return params;
}

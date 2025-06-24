#include "Track.h"
#include "SynthFunctions.h"
#include "AudioUtils.h"
#include <juce_core/juce_core.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <map>

using SynthFunc = juce::AudioBuffer<float>(*)(double, double, const juce::NamedValueSet&);

static std::map<juce::String, SynthFunc> synthMap {
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

int main (int argc, char* argv[])
{
    juce::ConsoleApplication app (argc, argv);

    if (argc < 3)
    {
        juce::Logger::writeToLog("Usage: diy_av_audio_cpp <input.json> <output.wav>");
        return 1;
    }

    juce::File inFile(argv[1]);
    juce::File outFile(argv[2]);
    Track track = loadTrackFromJson(inFile);

    double sampleRate = track.settings.sampleRate;
    int totalSamples = 0;
    for (const auto& step : track.steps)
        totalSamples += static_cast<int>(step.durationSeconds * sampleRate);

    juce::AudioBuffer<float> trackBuffer(2, totalSamples);
    int pos = 0;

    for (size_t i = 0; i < track.steps.size(); ++i)
    {
        const auto& step = track.steps[i];
        juce::AudioBuffer<float> stepBuffer(2, static_cast<int>(step.durationSeconds * sampleRate));
        stepBuffer.clear();
        for (const auto& voice : step.voices)
        {
            auto it = synthMap.find(voice.synthFunction);
            if (it != synthMap.end())
            {
                auto voiceBuf = it->second(step.durationSeconds, sampleRate, voice.params);
                for (int ch = 0; ch < 2; ++ch)
                    stepBuffer.addFrom(ch, 0, voiceBuf, ch, 0, voiceBuf.getNumSamples());
            }
        }

        trackBuffer.copyFrom(0, pos, stepBuffer, 0, 0, stepBuffer.getNumSamples());
        trackBuffer.copyFrom(1, pos, stepBuffer, 1, 0, stepBuffer.getNumSamples());
        pos += stepBuffer.getNumSamples();
    }

    writeWavFile(outFile, trackBuffer, sampleRate);
    return 0;
}


#include "Track.h"
#include "AudioUtils.h"
#include <juce_core/juce_core.h>
#include <juce_audio_formats/juce_audio_formats.h>

int main (int argc, char* argv[])
{
    juce::ConsoleApplication app (argc, argv);

    if (argc < 3 || argc > 4)
    {
        juce::Logger::writeToLog("Usage: diy_av_audio_cpp <input.json> <output.wav> [extra_steps.json]");
        return 1;
    }

    juce::File inFile(argv[1]);
    juce::File outFile(argv[2]);
    Track track = loadTrackFromJson(inFile);

    if (argc == 4)
    {
        juce::File stepsFile(argv[3]);
        int added = loadExternalStepsFromJson(stepsFile, track.steps);
        juce::Logger::writeToLog(juce::String("Loaded ") + juce::String(added) + " external step(s)");
    }

    double sampleRate = track.settings.sampleRate;

    juce::AudioBuffer<float> trackBuffer = assembleTrack(track);

    // Normalize final track to -12 dBFS (0.25)
    float peak = 0.0f;
    for (int ch = 0; ch < trackBuffer.getNumChannels(); ++ch)
        peak = std::max(peak, trackBuffer.getMagnitude(ch, 0, trackBuffer.getNumSamples()));
    if (peak > 0.0f)
        trackBuffer.applyGain(0.25f / peak);

    writeWavFile(outFile, trackBuffer, sampleRate);
    return 0;
}


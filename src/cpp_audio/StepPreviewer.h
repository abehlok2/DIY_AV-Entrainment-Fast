#pragma once
#include "Track.h"
#include "BufferAudioSource.h"
#include <juce_audio_utils/juce_audio_utils.h>

class StepPreviewer
{
public:
    explicit StepPreviewer(juce::AudioDeviceManager& dm);

    bool loadStep(const Step& step, const GlobalSettings& settings, double previewDuration);

    void play();
    void pause();
    void stop();

    void setPosition(double seconds);
    double getPosition() const;
    double getLength() const;
    bool isPlaying() const;

private:
    juce::AudioBuffer<float> generateAudio(const Step& step, const GlobalSettings& settings, double previewDuration);

    juce::AudioDeviceManager& deviceManager;
    std::unique_ptr<juce::AudioSourcePlayer> player;
    juce::AudioTransportSource transport;
    std::unique_ptr<BufferAudioSource> bufferSource;
    double sampleRate = 44100.0;
    double lengthSeconds = 0.0;
    bool playing = false;
};


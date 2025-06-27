#pragma once
#include "Track.h"
#include "BufferAudioSource.h"
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <atomic>

class StepPreviewJob;

class StepPreviewer
{
public:
    explicit StepPreviewer(juce::AudioDeviceManager& dm);
    ~StepPreviewer();

    bool loadStep(const Step& step, const GlobalSettings& settings, double previewDuration);

    void play();
    void pause();
    void stop();

    void setPosition(double seconds);
    double getPosition() const;
    double getLength() const;
    bool isPlaying() const;
    bool isReady() const;

private:
    void cancelJob();
    friend class StepPreviewJob;
    juce::AudioBuffer<float> generateAudio(const Step& step, const GlobalSettings& settings, double previewDuration);

    juce::AudioDeviceManager& deviceManager;
    std::unique_ptr<juce::AudioSourcePlayer> player;
    juce::AudioTransportSource transport;
    std::unique_ptr<BufferAudioSource> bufferSource;
    double sampleRate = 44100.0;
    double lengthSeconds = 0.0;
    bool playing = false;
    std::atomic<bool> ready { false };
    juce::ThreadPool pool { 1 };
    std::unique_ptr<juce::ThreadPoolJob> job;
    juce::CriticalSection lock;
};


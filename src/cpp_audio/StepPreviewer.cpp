#include "StepPreviewer.h"

StepPreviewer::StepPreviewer(juce::AudioDeviceManager& dm)
    : deviceManager(dm)
{
    player = std::make_unique<juce::AudioSourcePlayer>();
    player->setSource(&transport);
}

StepPreviewer::~StepPreviewer()
{
    if (playing)
        deviceManager.removeAudioCallback(player.get());
    player->setSource(nullptr);
    transport.setSource(nullptr);
}

bool StepPreviewer::loadStep(const Step& step, const GlobalSettings& settings, double previewDuration)
{
    sampleRate = settings.sampleRate;
    auto audio = generateAudio(step, settings, previewDuration);
    bufferSource = std::make_unique<BufferAudioSource>();
    bufferSource->setBuffer(std::move(audio));
    transport.setSource(bufferSource.get(), 0, nullptr, sampleRate);
    lengthSeconds = transport.getLengthInSeconds();
    transport.setPosition(0.0);
    return bufferSource->getTotalLength() > 0;
}

void StepPreviewer::play()
{
    if (playing)
        return;
    deviceManager.addAudioCallback(player.get());
    transport.start();
    playing = true;
}

void StepPreviewer::pause()
{
    if (!playing)
        return;
    transport.stop();
    deviceManager.removeAudioCallback(player.get());
    playing = false;
}

void StepPreviewer::stop()
{
    transport.stop();
    transport.setPosition(0.0);
    if (playing)
    {
        deviceManager.removeAudioCallback(player.get());
        playing = false;
    }
}

void StepPreviewer::setPosition(double seconds)
{
    transport.setPosition(seconds);
}

double StepPreviewer::getPosition() const
{
    return transport.getCurrentPosition();
}

double StepPreviewer::getLength() const
{
    return lengthSeconds;
}

bool StepPreviewer::isPlaying() const
{
    return playing;
}

juce::AudioBuffer<float> StepPreviewer::generateAudio(const Step& step, const GlobalSettings& settings, double previewDuration)
{
    Track t;
    t.settings = settings;
    t.settings.crossfadeDuration = 0.0; // no crossfade
    t.steps.push_back(step);

    auto stepBuffer = assembleTrack(t);

    int previewSamples = static_cast<int>(previewDuration * settings.sampleRate);
    juce::AudioBuffer<float> result(2, std::max(0, previewSamples));
    result.clear();

    if (stepBuffer.getNumSamples() == 0 || previewSamples <= 0)
        return result;

    int pos = 0;
    while (pos < previewSamples)
    {
        int remain = previewSamples - pos;
        int copy = std::min(remain, stepBuffer.getNumSamples());
        for (int ch = 0; ch < 2; ++ch)
            result.copyFrom(ch, pos, stepBuffer, ch, 0, copy);
        pos += copy;
    }
    return result;
}


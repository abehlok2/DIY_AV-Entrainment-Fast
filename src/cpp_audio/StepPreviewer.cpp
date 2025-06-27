#include "StepPreviewer.h"

namespace {
class StepPreviewJob : public juce::ThreadPoolJob
{
public:
    StepPreviewJob(StepPreviewer& o, Step s, GlobalSettings g, double dur)
        : juce::ThreadPoolJob("StepPreviewJob"), owner(o), step(std::move(s)), settings(std::move(g)), previewDur(dur) {}

    JobStatus runJob() override;

private:
    StepPreviewer& owner;
    Step step;
    GlobalSettings settings;
    double previewDur = 0.0;
};
}

StepPreviewer::StepPreviewer(juce::AudioDeviceManager& dm)
    : deviceManager(dm)
{
    player = std::make_unique<juce::AudioSourcePlayer>();
    player->setSource(&transport);
}

StepPreviewer::~StepPreviewer()
{

    cancelJob();

    if (playing)
        deviceManager.removeAudioCallback(player.get());
    player->setSource(nullptr);
    transport.setSource(nullptr);
}

bool StepPreviewer::loadStep(const Step& step, const GlobalSettings& settings, double previewDuration)
{
    cancelJob();
    sampleRate = settings.sampleRate;
    ready.store(false);
    job.reset(new StepPreviewJob(*this, step, settings, previewDuration));
    pool.addJob(job.get(), true);
    return true;
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

void StepPreviewer::cancelJob()
{
    if (job)
    {
        pool.removeJob(job.get(), true, 4000);
        job.reset();
    }
    ready.store(false);
}

bool StepPreviewer::isReady() const
{
    return ready.load();
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

StepPreviewJob::JobStatus StepPreviewJob::runJob()
{
    auto audio = owner.generateAudio(step, settings, previewDur);
    {
        const juce::ScopedLock sl(owner.lock);
        owner.bufferSource = std::make_unique<BufferAudioSource>();
        owner.bufferSource->setBuffer(std::move(audio));
        owner.transport.setSource(owner.bufferSource.get(), 0, nullptr, owner.sampleRate);
        owner.lengthSeconds = owner.transport.getLengthInSeconds();
        owner.transport.setPosition(0.0);
    }
    owner.ready.store(true);
    return jobHasFinished;
}


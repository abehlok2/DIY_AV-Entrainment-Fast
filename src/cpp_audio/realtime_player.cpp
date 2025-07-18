#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_core/juce_core.h>
#include "core/Track.h"
#include "core/TrackXml.h"
#include <thread>
#include <atomic>

// Simple real-time player that loads a Track and streams it to the audio device
class RealtimePlayer : public juce::AudioIODeviceCallback
{
public:
    explicit RealtimePlayer(Track t) : track(std::move(t)) {}

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override
    {
        sampleRate = device->getCurrentSampleRate();
        bufferSize = device->getCurrentBufferSizeSamples();
        buffers[0].setSize(2, bufferSize);
        buffers[1].setSize(2, bufferSize);
        currentBuffer = 0;
        stepIndex = 0;
        stepPos = 0;
        fillBuffer(0);
        fillBuffer(1);
        running = true;
    }

    void audioDeviceStopped() override
    {
        running = false;
        if (worker.joinable())
            worker.join();
    }

    void audioDeviceIOCallback(const float** /*inputChannelData*/, int /*numInputChannels*/,
                               float** outputChannelData, int numOutputChannels,
                               int numSamples) override
    {
        juce::ignoreUnused(numSamples); // assume matches bufferSize
        auto& buf = buffers[currentBuffer];
        for (int ch = 0; ch < numOutputChannels; ++ch)
        {
            const float* src = buf.getReadPointer(ch % buf.getNumChannels());
            float* dest = outputChannelData[ch];
            std::memcpy(dest, src, sizeof(float) * bufferSize);
        }

        if (worker.joinable())
            worker.join();
        int next = (currentBuffer + 1) % 2;
        worker = std::thread([this, next]() { fillBuffer(next); });
        currentBuffer = next;
    }

private:
    void fillBuffer(int index)
    {
        auto& buf = buffers[index];
        buf.clear();
        int filled = 0;
        while (filled < bufferSize)
        {
            if (stepIndex >= static_cast<int>(track.steps.size()))
            {
                break; // no more audio
            }

            if (stepBuffer.getNumSamples() == 0)
            {
                Track t;
                t.settings = track.settings;
                t.settings.crossfadeDuration = 0.0;
                t.steps.push_back(track.steps[stepIndex]);
                stepBuffer = assembleTrack(t);
                stepPos = 0;
            }

            int remain = stepBuffer.getNumSamples() - stepPos;
            int toCopy = std::min(remain, bufferSize - filled);
            for (int ch = 0; ch < buf.getNumChannels(); ++ch)
                buf.copyFrom(ch, filled, stepBuffer, ch, stepPos, toCopy);

            stepPos += toCopy;
            filled += toCopy;

            if (stepPos >= stepBuffer.getNumSamples())
            {
                stepIndex++;
                stepBuffer.setSize(0, 0);
            }
        }
    }

    Track track;
    double sampleRate = 44100.0;
    int bufferSize = 512;
    juce::AudioBuffer<float> buffers[2];
    int currentBuffer = 0;
    std::thread worker;
    std::atomic<bool> running { false };

    // step generation state
    int stepIndex = 0;
    juce::AudioBuffer<float> stepBuffer;
    int stepPos = 0;
};

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: realtime_player <track.json>" << std::endl;
        return 1;
    }

    juce::File trackFile (argv[1]);
    if (! trackFile.existsAsFile())
    {
        std::cerr << "Track file not found: " << trackFile.getFullPathName() << std::endl;
        return 1;
    }

    Track track;
    if (trackFile.hasFileExtension(".xml"))
        track = loadTrackFromXml(trackFile);
    else
        track = loadTrackFromJson(trackFile);

    juce::AudioDeviceManager deviceManager;
    deviceManager.initialise(0, 2, nullptr, true);

    RealtimePlayer player(std::move(track));
    deviceManager.addAudioCallback(&player);

    std::cout << "Playing... press ENTER to quit" << std::endl;
    std::cin.get();

    deviceManager.removeAudioCallback(&player);
    return 0;
}


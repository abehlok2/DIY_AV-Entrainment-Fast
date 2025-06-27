#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

class BufferAudioSource : public juce::PositionableAudioSource
{
public:
    BufferAudioSource() = default;

    void setBuffer(juce::AudioBuffer<float> newBuffer)
    {
        buffer = std::move(newBuffer);
        position = 0;
    }

    void prepareToPlay(int /*samplesPerBlockExpected*/, double /*sr*/) override
    {
        position = 0;
    }

    void releaseResources() override {}

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& info) override
    {
        if (buffer.getNumSamples() == 0)
        {
            info.clearActiveBufferRegion();
            return;
        }

        int remaining = buffer.getNumSamples() - position;
        int toCopy = std::min(remaining, info.numSamples);

        for (int ch = 0; ch < info.buffer->getNumChannels(); ++ch)
            info.buffer->copyFrom(ch, info.startSample, buffer, ch % buffer.getNumChannels(), position, toCopy);

        if (toCopy < info.numSamples)
        {
            int left = info.numSamples - toCopy;
            for (int ch = 0; ch < info.buffer->getNumChannels(); ++ch)
                info.buffer->copyFrom(ch, info.startSample + toCopy, buffer, ch % buffer.getNumChannels(), 0, left);
            position = left;
        }
        else
        {
            position += toCopy;
            if (looping && position >= buffer.getNumSamples())
                position = 0;
        }
    }

    void setNextReadPosition(juce::int64 newPosition) override { position = (int)newPosition; }
    juce::int64 getNextReadPosition() const override { return position; }
    juce::int64 getTotalLength() const override { return buffer.getNumSamples(); }
    bool isLooping() const override { return looping; }
    void setLooping(bool shouldLoop) override { looping = shouldLoop; }
    bool isReady() const { return buffer.getNumSamples() > 0; }

private:
    juce::AudioBuffer<float> buffer;
    int position = 0;
    bool looping = false;
};


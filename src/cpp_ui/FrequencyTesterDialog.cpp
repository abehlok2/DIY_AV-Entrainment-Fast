// C++ implementation of the FrequencyTesterDialog.
// Translated from src/audio/ui/frequency_tester_dialog.py and adapted for JUCE.

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_dsp/juce_dsp.h>
#include "../cpp_audio/SynthFunctions.h"

using namespace juce;

namespace {
constexpr int MAX_VOICES = 10;
constexpr double SEGMENT_DURATION_S = 60.0;
constexpr double MIN_DB = -60.0;

inline double amplitudeToDb (double amp)
{
    return amp <= 0.0 ? MIN_DB : 20.0 * std::log10 (amp);
}

inline double dbToAmplitude (double db)
{
    return db <= MIN_DB ? 0.0 : std::pow (10.0, db / 20.0);
}

struct Preferences
{
    int sampleRate = 44100;
    String amplitudeDisplayMode { "absolute" }; // or "dB"
};

class BufferAudioSource : public AudioSource
{
public:
    BufferAudioSource() = default;

    void setBuffer (AudioBuffer<float> newBuffer)
    {
        buffer = std::move (newBuffer);
        position = 0;
    }

    void prepareToPlay (int /*samplesPerBlockExpected*/, double /*sr*/) override
    {
        position = 0;
    }

    void releaseResources() override {}

    void getNextAudioBlock (const AudioSourceChannelInfo& info) override
    {
        if (buffer.getNumSamples() == 0)
        {
            info.clearActiveBufferRegion();
            return;
        }

        int remaining = buffer.getNumSamples() - position;
        int toCopy = jmin (remaining, info.numSamples);

        for (int ch = 0; ch < info.buffer->getNumChannels(); ++ch)
            info.buffer->copyFrom (ch, info.startSample, buffer, ch % buffer.getNumChannels(), position, toCopy);

        if (toCopy < info.numSamples)
        {
            int left = info.numSamples - toCopy;
            for (int ch = 0; ch < info.buffer->getNumChannels(); ++ch)
                info.buffer->copyFrom (ch, info.startSample + toCopy, buffer, ch % buffer.getNumChannels(), 0, left);
            position = left;
        }
        else
        {
            position += toCopy;
            if (position >= buffer.getNumSamples())
                position = 0; // loop
        }
    }

private:
    AudioBuffer<float> buffer;
    int position = 0;
};
}

class FrequencyTesterDialog  : public Component,
                               private Button::Listener
{
public:
    FrequencyTesterDialog (AudioDeviceManager& dm, Preferences* prefsIn = nullptr)
        : deviceManager (dm)
    {
        if (prefsIn)
            prefs = *prefsIn;

        setSize (600, 400);

        for (int i = 0; i < MAX_VOICES; ++i)
        {
            auto* vc = voiceControls.add (new VoiceControl());
            vc->enable.setButtonText ("Voice " + String (i + 1));
            addAndMakeVisible (vc->enable);

            addAndMakeVisible (vc->baseLabel);
            vc->baseLabel.setText ("Base Freq", dontSendNotification);
            addAndMakeVisible (vc->base);
            vc->base.setRange (20.0, 20000.0, 0.01);
            vc->base.setValue (200.0);

            addAndMakeVisible (vc->beatLabel);
            vc->beatLabel.setText ("Beat Freq", dontSendNotification);
            addAndMakeVisible (vc->beat);
            vc->beat.setRange (0.0, 40.0, 0.01);
            vc->beat.setValue (4.0);

            addAndMakeVisible (vc->ampLabel);
            vc->ampLabel.setText ("Amp", dontSendNotification);
            addAndMakeVisible (vc->amp);

            if (prefs.amplitudeDisplayMode == "dB")
            {
                vc->amp.setRange (MIN_DB, 0.0, 0.1);
                vc->amp.setValue (amplitudeToDb (0.5));
            }
            else
            {
                vc->amp.setRange (0.0, 1.0, 0.01);
                vc->amp.setValue (0.5);
            }
        }

        startButton.setButtonText ("Start");
        stopButton.setButtonText ("Stop");
        stopButton.setEnabled (false);
        addAndMakeVisible (startButton);
        addAndMakeVisible (stopButton);
        startButton.addListener (this);
        stopButton.addListener (this);
    }

    ~FrequencyTesterDialog() override
    {
        stopPlayback();
    }

    void resized() override
    {
        int y = 10;
        for (auto* vc : voiceControls)
        {
            vc->enable.setBounds (10, y, 80, 24);
            vc->baseLabel.setBounds (100, y, 70, 24);
            vc->base.setBounds (170, y, 100, 24);
            vc->beatLabel.setBounds (280, y, 70, 24);
            vc->beat.setBounds (350, y, 80, 24);
            vc->ampLabel.setBounds (440, y, 40, 24);
            vc->amp.setBounds (480, y, 80, 24);
            y += 30;
        }

        startButton.setBounds (getWidth() - 190, y + 10, 80, 26);
        stopButton.setBounds (getWidth() - 100, y + 10, 80, 26);
    }

private:
    struct VoiceControl
    {
        ToggleButton enable;
        Label baseLabel;
        Slider base;
        Label beatLabel;
        Slider beat;
        Label ampLabel;
        Slider amp;
    };

    OwnedArray<VoiceControl> voiceControls;
    TextButton startButton, stopButton;
    AudioDeviceManager& deviceManager;
    std::unique_ptr<AudioSourcePlayer> player;
    std::unique_ptr<BufferAudioSource> bufferSource;
    Preferences prefs;

    void buttonClicked (Button* b) override
    {
        if (b == &startButton)
            startPlayback();
        else if (b == &stopButton)
            stopPlayback();
    }

    void startPlayback()
    {
        auto audio = generateAudio();
        if (audio.getNumSamples() == 0)
            return;

        bufferSource.reset (new BufferAudioSource());
        bufferSource->setBuffer (std::move (audio));

        player.reset (new AudioSourcePlayer());
        player->setSource (bufferSource.get());

        deviceManager.addAudioCallback (player.get());

        startButton.setEnabled (false);
        stopButton.setEnabled (true);
    }

    void stopPlayback()
    {
        if (player)
            deviceManager.removeAudioCallback (player.get());
        player.reset();
        bufferSource.reset();
        startButton.setEnabled (true);
        stopButton.setEnabled (false);
    }

    AudioBuffer<float> generateAudio()
    {
        int sampleRate = prefs.sampleRate;
        int totalSamples = static_cast<int> (SEGMENT_DURATION_S * sampleRate);
        AudioBuffer<float> mix (2, totalSamples);
        mix.clear();

        bool anyEnabled = false;

        for (auto* vc : voiceControls)
        {
            if (vc->enable.getToggleState())
            {
                anyEnabled = true;
                double base = vc->base.getValue();
                double beat = vc->beat.getValue();
                double amp = vc->amp.getValue();

                if (prefs.amplitudeDisplayMode == "dB")
                    amp = dbToAmplitude (amp);

                NamedValueSet params;
                params.set ("ampL", amp);
                params.set ("ampR", amp);
                params.set ("baseFreq", base);
                params.set ("beatFreq", beat);

                auto voiceBuf = binauralBeat (SEGMENT_DURATION_S, (double) sampleRate, params);
                if (voiceBuf.getNumSamples() < totalSamples)
                {
                    AudioBuffer<float> padded (2, totalSamples);
                    padded.clear();
                    padded.copyFrom (0, 0, voiceBuf, 0, 0, voiceBuf.getNumSamples());
                    padded.copyFrom (1, 0, voiceBuf, 1, 0, voiceBuf.getNumSamples());
                    voiceBuf = std::move (padded);
                }

                for (int ch = 0; ch < 2; ++ch)
                    mix.addFrom (ch, 0, voiceBuf, ch, 0, totalSamples);
            }
        }

        if (! anyEnabled)
            return {};

        float peak = 0.0f;
        for (int ch = 0; ch < 2; ++ch)
            peak = jmax (peak, mix.getMagnitude (ch, 0, mix.getNumSamples()));

        if (peak > 1.0f)
            mix.applyGain (1.0f / peak);

        return mix;
    }
};

std::unique_ptr<juce::Component> createFrequencyTesterDialog(juce::AudioDeviceManager& dm)
{
    return std::make_unique<FrequencyTesterDialog>(dm, nullptr);
}



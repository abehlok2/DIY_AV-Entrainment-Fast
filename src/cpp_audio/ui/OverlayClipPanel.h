#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <functional>
#include <vector>
#include "OverlayClipDialog.h"

class OverlayClipPanel : public juce::Component,
                         private juce::ListBoxModel,
                         private juce::Button::Listener
{
public:
    OverlayClipPanel();
    ~OverlayClipPanel() override;

    using ClipData = OverlayClipDialog::ClipData;

    int getNumRows() override;
    void paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void resized() override;

    void setClips(const std::vector<ClipData>& c);
    std::vector<ClipData> getClips() const;

    std::function<void()> onClipsChanged;

private:
    void buttonClicked(juce::Button*) override;
    void addClip();
    void editClip();
    void removeClip();
    void startPlayback();
    void stopPlayback();

    juce::ListBox clipList;
    juce::TextButton addButton {"Add Clip"};
    juce::TextButton editButton {"Edit Clip"};
    juce::TextButton removeButton {"Remove Clip"};
    juce::TextButton playButton {"Start Clip"};

    juce::AudioDeviceManager deviceManager;
    juce::AudioFormatManager formatManager;
    juce::AudioTransportSource transport;
    juce::AudioSourcePlayer sourcePlayer;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;

    std::vector<ClipData> clips;
};

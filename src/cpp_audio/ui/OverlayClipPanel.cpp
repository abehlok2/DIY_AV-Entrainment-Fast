#include "OverlayClipPanel.h"
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>

OverlayClipPanel::OverlayClipPanel()
{
    addAndMakeVisible(clipList);
    clipList.setModel(this);

    for (auto* b : { &addButton, &editButton, &removeButton, &playButton })
    {
        addAndMakeVisible(b);
        b->addListener(this);
    }

    formatManager.registerBasicFormats();
}

OverlayClipPanel::~OverlayClipPanel()
{
    for (auto* b : { &addButton, &editButton, &removeButton, &playButton })
        b->removeListener(this);
    stopPlayback();
}

int OverlayClipPanel::getNumRows()
{
    return static_cast<int>(clips.size());
}

void OverlayClipPanel::paintListBoxItem(int row, juce::Graphics& g, int width, int height, bool rowSel)
{
    if (rowSel)
        g.fillAll(juce::Colours::lightblue);

    if (row >= 0 && row < (int)clips.size())
    {
        const auto& c = clips[(size_t)row];
        auto name = juce::File(c.filePath).getFileName();
        auto text = name + " - " + juce::String(c.start, 2) + "s";
        g.setColour(juce::Colours::black);
        g.drawText(text, 4, 0, width - 4, height, juce::Justification::centredLeft);
    }
}

void OverlayClipPanel::resized()
{
    auto area = getLocalBounds().reduced(4);
    clipList.setBounds(area.removeFromTop(getHeight() - 40));
    auto buttons = area;
    auto each = buttons.getWidth() / 4;
    addButton.setBounds(buttons.removeFromLeft(each));
    editButton.setBounds(buttons.removeFromLeft(each));
    removeButton.setBounds(buttons.removeFromLeft(each));
    playButton.setBounds(buttons);
}

void OverlayClipPanel::buttonClicked(juce::Button* b)
{
    if (b == &addButton)
        addClip();
    else if (b == &editButton)
        editClip();
    else if (b == &removeButton)
        removeClip();
    else if (b == &playButton)
    {
        if (transport.isPlaying())
            stopPlayback();
        else
            startPlayback();
    }
}

void OverlayClipPanel::addClip()
{
    bool ok = false;
    auto data = showOverlayClipEditor(false, nullptr, &ok);
    if (ok)
    {
        clips.push_back(data);
        clipList.updateContent();
        clipList.selectRow((int)clips.size() - 1);
    }
}

void OverlayClipPanel::editClip()
{
    int row = clipList.getSelectedRow();
    if (row < 0 || row >= (int)clips.size())
        return;
    bool ok = false;
    auto data = showOverlayClipEditor(false, &clips[(size_t)row], &ok);
    if (ok)
    {
        clips[(size_t)row] = data;
        clipList.repaintRow(row);
    }
}

void OverlayClipPanel::removeClip()
{
    int row = clipList.getSelectedRow();
    if (row < 0 || row >= (int)clips.size())
        return;
    clips.erase(clips.begin() + row);
    clipList.updateContent();
}

void OverlayClipPanel::startPlayback()
{
    int row = clipList.getSelectedRow();
    if (row < 0 || row >= (int)clips.size())
        return;
    juce::File f(clips[(size_t)row].filePath);
    if (!f.existsAsFile())
        return;

    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(f));
    if (!reader)
        return;

    readerSource.reset(new juce::AudioFormatReaderSource(reader.release(), true));
    transport.setSource(readerSource.get(), 0, nullptr, readerSource->sampleRate);
    deviceManager.addAudioCallback(&transport);
    transport.start();
    playButton.setButtonText("Stop Clip");
}

void OverlayClipPanel::stopPlayback()
{
    transport.stop();
    transport.setSource(nullptr);
    if (readerSource)
        readerSource.reset();
    deviceManager.removeAudioCallback(&transport);
    playButton.setButtonText("Start Clip");
}

// OverlayClipDialog C++ implementation.
// Translated from src/audio/ui/overlay_clip_dialog.py

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "OverlayClipDialog.h"
#include <cmath>
#include <optional>

namespace
{
constexpr double MIN_DB = -60.0;

double amplitudeToDb(double amplitude)
{
    if (amplitude <= 0.0)
        return MIN_DB;
    return 20.0 * std::log10(amplitude);
}

double dbToAmplitude(double db)
{
    if (db <= MIN_DB)
        return 0.0;
    return std::pow(10.0, db / 20.0);
}

double getClipDuration(const juce::File& file)
{
    juce::AudioFormatManager fm;
    fm.registerBasicFormats();
    if (auto* reader = fm.createReaderFor(file))
    {
        std::unique_ptr<juce::AudioFormatReader> r(reader);
        if (r->sampleRate > 0.0)
            return (double) r->lengthInSamples / r->sampleRate;
    }
    return 0.0;
}
}

// Implementation component used by showOverlayClipEditor.
// Uses the ClipData struct defined in OverlayClipDialog.
struct OverlayClipDialogWindow  : public juce::Component,
                            private juce::Button::Listener,
                            private juce::Timer
{
    using ClipData = OverlayClipDialog::ClipData;

    OverlayClipDialogWindow(bool ampInDb = false, const ClipData* existing = nullptr)
        : amplitudeInDb(ampInDb)
    {
        setOpaque(true);
        setSize(400, 260);
        addAndMakeVisible(&fileLabel);
        addAndMakeVisible(&fileEdit);
        addAndMakeVisible(&browseButton);
        browseButton.addListener(this);

        addAndMakeVisible(&descLabel);
        addAndMakeVisible(&descEdit);

        addAndMakeVisible(&startLabel);
        addAndMakeVisible(&startEdit);

        addAndMakeVisible(&ampLabel);
        addAndMakeVisible(&ampEdit);

        addAndMakeVisible(&panLabel);
        addAndMakeVisible(&panEdit);

        addAndMakeVisible(&fadeInLabel);
        addAndMakeVisible(&fadeInEdit);

        addAndMakeVisible(&fadeOutLabel);
        addAndMakeVisible(&fadeOutEdit);

        addAndMakeVisible(&okButton);
        addAndMakeVisible(&cancelButton);
        addAndMakeVisible(&playButton);
        okButton.addListener(this);
        cancelButton.addListener(this);
        playButton.addListener(this);

        deviceManager.initialise(0, 2, nullptr, true);
        formatManager.registerBasicFormats();
        startTimer(100);

        fileLabel.setText("Audio File:", juce::dontSendNotification);
        descLabel.setText("Description:", juce::dontSendNotification);
        startLabel.setText("Start Time (s):", juce::dontSendNotification);
        ampLabel.setText("Amplitude:", juce::dontSendNotification);
        panLabel.setText("Pan:", juce::dontSendNotification);
        fadeInLabel.setText("Fade In (s):", juce::dontSendNotification);
        fadeOutLabel.setText("Fade Out (s):", juce::dontSendNotification);

        startEdit.setInputRestrictions(0, "0123456789.-");
        ampEdit.setInputRestrictions(0, "0123456789.-");
        panEdit.setInputRestrictions(0, "0123456789.-");
        fadeInEdit.setInputRestrictions(0, "0123456789.-");
        fadeOutEdit.setInputRestrictions(0, "0123456789.-");

        if (amplitudeInDb)
        {
            ampEdit.setText(juce::String(amplitudeToDb(1.0), 1));
        }
        else
        {
            ampEdit.setText("1.0");
        }

        fileEdit.setColour(juce::TextEditor::backgroundColourId, juce::Colours::white);

        if (existing)
            populateFromData(*existing);
    }

    ~OverlayClipDialogWindow() override
    {
        stopPlayback();
        transport.releaseResources();
    }

    bool wasAccepted() const { return accepted; }
    ClipData getClipData() const { return data; }

    void resized() override
    {
        auto area = getLocalBounds().reduced(10);
        const int labelW = 100;
        const int rowH = 24;
        auto row = area.removeFromTop(rowH);
        fileLabel.setBounds(row.removeFromLeft(labelW));
        browseButton.setBounds(row.removeFromRight(80));
        fileEdit.setBounds(row);

        row = area.removeFromTop(rowH);
        descLabel.setBounds(row.removeFromLeft(labelW));
        descEdit.setBounds(row);

        row = area.removeFromTop(rowH);
        startLabel.setBounds(row.removeFromLeft(labelW));
        startEdit.setBounds(row);

        row = area.removeFromTop(rowH);
        ampLabel.setBounds(row.removeFromLeft(labelW));
        ampEdit.setBounds(row);

        row = area.removeFromTop(rowH);
        panLabel.setBounds(row.removeFromLeft(labelW));
        panEdit.setBounds(row);

        row = area.removeFromTop(rowH);
        fadeInLabel.setBounds(row.removeFromLeft(labelW));
        fadeInEdit.setBounds(row);

        row = area.removeFromTop(rowH);
        fadeOutLabel.setBounds(row.removeFromLeft(labelW));
        fadeOutEdit.setBounds(row);

        area.removeFromTop(10);
        row = area.removeFromTop(30);
        okButton.setBounds(row.removeFromRight(80));
        cancelButton.setBounds(row.removeFromRight(80));
        playButton.setBounds(row.removeFromLeft(100));
    }

private:
    bool amplitudeInDb = false;
    bool accepted = false;
    ClipData data;

    juce::Label fileLabel, descLabel, startLabel, ampLabel, panLabel, fadeInLabel, fadeOutLabel;
    juce::TextEditor fileEdit, descEdit, startEdit, ampEdit, panEdit, fadeInEdit, fadeOutEdit;
    juce::TextButton browseButton { "Browse" }, okButton { "OK" }, cancelButton { "Cancel" }, playButton { "Start Clip" };

    juce::AudioDeviceManager deviceManager;
    juce::AudioFormatManager formatManager;
    juce::AudioTransportSource transport;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;

    void buttonClicked(juce::Button* b) override
    {
        if (b == &browseButton)
            browseFile();
        else if (b == &okButton)
            onAccept();
        else if (b == &cancelButton)
            if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
                dw->exitModalState(0);
        else if (b == &playButton)
            if (transport.isPlaying())
                stopPlayback();
            else
                startPlayback();
    }

    void populateFromData(const ClipData& d)
    {
        fileEdit.setText(d.filePath);
        startEdit.setText(juce::String(d.start, 3));
        double ampVal = d.amp;
        if (amplitudeInDb)
            ampVal = amplitudeToDb(ampVal);
        ampEdit.setText(juce::String(ampVal, amplitudeInDb ? 1 : 2));
        panEdit.setText(juce::String(d.pan, 2));
        fadeInEdit.setText(juce::String(d.fadeIn, 3));
        fadeOutEdit.setText(juce::String(d.fadeOut, 3));
        descEdit.setText(d.description);
    }

    void browseFile()
    {
        juce::FileChooser chooser("Select Audio File", {}, "*.wav;*.flac;*.mp3");
        if (chooser.browseForFileToOpen())
            fileEdit.setText(chooser.getResult().getFullPathName());
    }

    bool collectData()
    {
        juce::File f(fileEdit.getText().trim());
        if (! f.existsAsFile())
        {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                                   "Input Required",
                                                   "Please select an audio file.");
            return false;
        }

        data.filePath = f.getFullPathName();
        data.start = startEdit.getText().trim().getDoubleValue();
        data.duration = getClipDuration(f);

        double ampVal = ampEdit.getText().trim().getDoubleValue();
        if (amplitudeInDb)
            ampVal = dbToAmplitude(ampVal);
        data.amp = ampVal;

        data.pan = panEdit.getText().trim().getDoubleValue();
        data.fadeIn = fadeInEdit.getText().trim().getDoubleValue();
        data.fadeOut = fadeOutEdit.getText().trim().getDoubleValue();
        data.description = descEdit.getText().trim();
        return true;
    }

    void startPlayback()
    {
        juce::File f(fileEdit.getText().trim());
        if (!f.existsAsFile())
        {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                                   "Play Clip",
                                                   "Please select a valid audio file.");
            return;
        }
        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(f));
        if (!reader)
        {
            juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                                   "Play Clip",
                                                   "Could not read audio file.");
            return;
        }
        double sr = reader->sampleRate;
        readerSource.reset(new juce::AudioFormatReaderSource(reader.release(), true));
        transport.setSource(readerSource.get(), 0, nullptr, sr);
        transport.start();
        playButton.setButtonText("Stop Clip");
    }

    void stopPlayback()
    {
        transport.stop();
        transport.setSource(nullptr);
        readerSource.reset();
        playButton.setButtonText("Start Clip");
    }

    void timerCallback() override
    {
        if (transport.isPlaying() && transport.hasStreamFinished())
            stopPlayback();
    }

    void onAccept()
    {
        if (collectData())
        {
            accepted = true;
            if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
                dw->exitModalState(1);
        }
    }
};

OverlayClipDialog::ClipData showOverlayClipEditor(bool amplitudeInDb,
                                                  const OverlayClipDialog::ClipData* existing,
                                                  bool* success)
{
    OverlayClipDialogWindow dialog(amplitudeInDb, existing);
    juce::DialogWindow::LaunchOptions opts;
    opts.content.setNonOwned(&dialog);
    opts.dialogTitle = "Overlay Clip";
    opts.dialogBackgroundColour = juce::Colours::lightgrey;
    opts.escapeKeyTriggersCloseButton = true;
    opts.useNativeTitleBar = true;
    opts.resizable = false;
    int result = opts.runModal();
    if (success)
        *success = (result != 0 && dialog.wasAccepted());
    return dialog.getClipData();
}


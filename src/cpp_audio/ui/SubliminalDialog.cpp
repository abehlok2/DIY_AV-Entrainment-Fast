// SubliminalDialog C++ implementation.
// Translated from src/audio/ui/subliminal_dialog.py and adapted for JUCE.

#include "../Track.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <cmath>

using namespace juce;

namespace {
constexpr double MIN_DB = -60.0;

inline double amplitudeToDb (double amp)
{
    return amp <= 0.0 ? MIN_DB : 20.0 * std::log10 (amp);
}

inline double dbToAmplitude (double db)
{
    return db <= MIN_DB ? 0.0 : std::pow (10.0, db / 20.0);
}
}

struct SubliminalDialog  : public Component,
                           private Button::Listener
{
    SubliminalDialog (bool ampInDb = false)
        : amplitudeInDb (ampInDb)
    {
        setSize (400, 160);

        addAndMakeVisible (fileLabel);
        addAndMakeVisible (fileEdit);
        addAndMakeVisible (browseButton);
        browseButton.addListener (this);

        addAndMakeVisible (freqLabel);
        addAndMakeVisible (freqSlider);

        addAndMakeVisible (ampLabel);
        addAndMakeVisible (ampSlider);

        addAndMakeVisible (modeLabel);
        addAndMakeVisible (modeBox);

        addAndMakeVisible (addButton);
        addAndMakeVisible (cancelButton);
        addButton.addListener (this);
        cancelButton.addListener (this);

        fileLabel.setText ("Audio File(s):", dontSendNotification);
        freqLabel.setText ("Carrier Freq (Hz):", dontSendNotification);
        ampLabel.setText ("Amplitude:", dontSendNotification);
        modeLabel.setText ("Mode:", dontSendNotification);

        freqSlider.setRange (15000.0, 20000.0, 0.1);
        freqSlider.setValue (17500.0);
        freqSlider.setTextValueSuffix (" Hz");

        if (amplitudeInDb)
        {
            ampSlider.setRange (MIN_DB, 0.0, 0.1);
            ampSlider.setTextValueSuffix (" dB");
            ampSlider.setValue (amplitudeToDb (0.5));
        }
        else
        {
            ampSlider.setRange (0.0, 1.0, 0.01);
            ampSlider.setValue (0.5);
        }

        modeBox.addItem ("sequence", 1);
        modeBox.addItem ("stack", 2);
        modeBox.setSelectedId (1);

        fileEdit.setColour (TextEditor::backgroundColourId, Colours::white);
    }

    bool wasAccepted () const                       { return accepted; }
    Voice getVoice () const                         { return voice; }

    void resized () override
    {
        auto area = getLocalBounds().reduced (10);
        const int labelW = 120;
        const int rowH = 24;

        auto row = area.removeFromTop (rowH);
        fileLabel.setBounds (row.removeFromLeft (labelW));
        browseButton.setBounds (row.removeFromRight (80));
        fileEdit.setBounds (row);

        row = area.removeFromTop (rowH);
        freqLabel.setBounds (row.removeFromLeft (labelW));
        freqSlider.setBounds (row);

        row = area.removeFromTop (rowH);
        ampLabel.setBounds (row.removeFromLeft (labelW));
        ampSlider.setBounds (row);

        row = area.removeFromTop (rowH);
        modeLabel.setBounds (row.removeFromLeft (labelW));
        modeBox.setBounds (row);

        area.removeFromTop (10);
        row = area.removeFromTop (30);
        addButton.setBounds (row.removeFromRight (80));
        cancelButton.setBounds (row.removeFromRight (80));
    }

private:
    bool amplitudeInDb = false;
    bool accepted = false;
    Voice voice;

    Label fileLabel, freqLabel, ampLabel, modeLabel;
    TextEditor fileEdit;
    TextButton browseButton { "Browse" };
    Slider freqSlider { Slider::LinearHorizontal, Slider::TextBoxRight };
    Slider ampSlider  { Slider::LinearHorizontal, Slider::TextBoxRight };
    ComboBox modeBox;
    TextButton addButton { "Add" }, cancelButton { "Cancel" };

    void buttonClicked (Button* b) override
    {
        if (b == &browseButton)
        {
            FileChooser fc ("Select Audio", {}, "*.wav;*.flac;*.mp3");
            if (fc.browseForMultipleFilesToOpen())
            {
                StringArray arr;
                for (auto& f : fc.getResults())
                    arr.add (f.getFullPathName());
                fileEdit.setText (arr.joinIntoString (";"));
            }
        }
        else if (b == &addButton)
        {
            onAccept();
        }
        else if (b == &cancelButton)
        {
            if (auto* dw = findParentComponentOfClass<DialogWindow>())
                dw->exitModalState (0);
        }
    }

    void onAccept ()
    {
        auto raw = fileEdit.getText().trim();
        if (raw.isEmpty())
        {
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "Input Required",
                                              "Please select an audio file.");
            return;
        }

        StringArray paths;
        paths.addTokens (raw, ";", "");
        paths.trim();
        paths.removeEmptyStrings();
        if (paths.isEmpty())
        {
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "Input Required",
                                              "Please select an audio file.");
            return;
        }

        double ampVal = ampSlider.getValue();
        if (amplitudeInDb)
            ampVal = dbToAmplitude (ampVal);

        voice = {};
        voice.synthFunction = "subliminal_encode";
        voice.isTransition = false;
        voice.description = "Subliminal";
        voice.params.set ("carrierFreq", freqSlider.getValue());
        voice.params.set ("amp", ampVal);
        voice.params.set ("mode", modeBox.getText());

        if (paths.size() == 1)
        {
            voice.params.set ("audio_path", paths[0]);
        }
        else
        {
            Array<var> arr;
            for (auto& p : paths)
                arr.add (p);
            voice.params.set ("audio_paths", var (arr));
        }

        accepted = true;
        if (auto* dw = findParentComponentOfClass<DialogWindow>())
            dw->exitModalState (1);
    }
};



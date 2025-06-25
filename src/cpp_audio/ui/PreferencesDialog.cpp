#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <cmath>
#include "Preferences.h"

using namespace juce;

namespace {
constexpr double MIN_DB = -60.0;

static double amplitudeToDb (double amp)
{
    return amp <= 0.0 ? MIN_DB : 20.0 * std::log10 (amp);
}

static double dbToAmplitude (double db)
{
    return db <= MIN_DB ? 0.0 : std::pow (10.0, db / 20.0);
}
}


class PreferencesDialog  : public Component,
                           private Button::Listener,
                           private ComboBox::Listener
{
public:
    explicit PreferencesDialog (const Preferences& prefsIn)
        : prefs (prefsIn), ampMode (prefsIn.amplitudeDisplayMode)
    {
        setSize (420, 360);

        addAndMakeVisible (&fontFamilyLabel);
        fontFamilyLabel.setText ("Font Family:", dontSendNotification);
        addAndMakeVisible (&fontCombo);
        fontCombo.addItemList (Font::getAvailableTypefaces(), 1);
        if (prefs.fontFamily.isNotEmpty())
            fontCombo.setText (prefs.fontFamily, dontSendNotification);

        addAndMakeVisible (&fontSizeLabel);
        fontSizeLabel.setText ("Font Size:", dontSendNotification);
        addAndMakeVisible (&fontSizeSlider);
        fontSizeSlider.setRange (6, 48, 1);
        fontSizeSlider.setSliderStyle (Slider::IncDecButtons);
        fontSizeSlider.setTextBoxStyle (Slider::TextBoxLeft, false, 60, 20);
        fontSizeSlider.setValue (prefs.fontSize);

        addAndMakeVisible (&themeLabel);
        themeLabel.setText ("Theme:", dontSendNotification);
        addAndMakeVisible (&themeCombo);
        themeCombo.addItem ("Dark", 1);
        themeCombo.addItem ("Green", 2);
        themeCombo.addItem ("light-blue", 3);
        themeCombo.addItem ("Material", 4);
        if (auto* item = themeCombo.getItemID (themeCombo.indexOfItemText (prefs.theme)))
            themeCombo.setText (prefs.theme, dontSendNotification);
        else
            themeCombo.setSelectedId (1);

        addAndMakeVisible (&exportLabel);
        exportLabel.setText ("Export Directory:", dontSendNotification);
        addAndMakeVisible (&exportEdit);
        exportEdit.setText (prefs.exportDir);
        addAndMakeVisible (&browseButton);
        browseButton.setButtonText ("Browse");
        browseButton.addListener (this);

        addAndMakeVisible (&sampleRateLabel);
        sampleRateLabel.setText ("Sample Rate (Hz):", dontSendNotification);
        addAndMakeVisible (&sampleRateSlider);
        sampleRateSlider.setRange (8000, 192000, 1);
        sampleRateSlider.setSliderStyle (Slider::IncDecButtons);
        sampleRateSlider.setTextBoxStyle (Slider::TextBoxLeft, false, 80, 20);
        sampleRateSlider.setValue (prefs.sampleRate);

        addAndMakeVisible (&testDurationLabel);
        testDurationLabel.setText ("Test Step Duration (s):", dontSendNotification);
        addAndMakeVisible (&testDurationSlider);
        testDurationSlider.setRange (0.1, 600.0, 0.1);
        testDurationSlider.setSliderStyle (Slider::IncDecButtons);
        testDurationSlider.setTextBoxStyle (Slider::TextBoxLeft, false, 80, 20);
        testDurationSlider.setValue (prefs.testStepDuration);

        addAndMakeVisible (&targetAmpLabel);
        targetAmpLabel.setText ("Target Output Amplitude:", dontSendNotification);
        addAndMakeVisible (&targetAmpSlider);
        targetAmpSlider.setSliderStyle (Slider::IncDecButtons);
        targetAmpSlider.setTextBoxStyle (Slider::TextBoxLeft, false, 80, 20);
        if (ampMode == "dB")
        {
            targetAmpSlider.setRange (MIN_DB, 0.0, 1.0);
            targetAmpSlider.setValue (amplitudeToDb (prefs.targetOutputAmplitude));
            targetAmpSlider.setTextValueSuffix (" dB");
        }
        else
        {
            targetAmpSlider.setRange (0.0, 1.0, 0.01);
            targetAmpSlider.setValue (prefs.targetOutputAmplitude);
        }

        addAndMakeVisible (&ampModeLabel);
        ampModeLabel.setText ("Amplitude Display:", dontSendNotification);
        addAndMakeVisible (&ampModeCombo);
        ampModeCombo.addItem ("absolute", 1);
        ampModeCombo.addItem ("dB", 2);
        ampModeCombo.setSelectedId (ampMode == "dB" ? 2 : 1);
        ampModeCombo.addListener (this);

        addAndMakeVisible (&crossfadeCurveLabel);
        crossfadeCurveLabel.setText ("Crossfade Curve:", dontSendNotification);
        addAndMakeVisible (&crossfadeCurveCombo);
        crossfadeCurveCombo.addItem ("linear", 1);
        crossfadeCurveCombo.addItem ("equal_power", 2);
        crossfadeCurveCombo.setSelectedId (prefs.crossfadeCurve == "equal_power" ? 2 : 1);

        trackMetadataToggle.setButtonText ("Include track export metadata");
        trackMetadataToggle.setToggleState (prefs.trackMetadata, dontSendNotification);
        addAndMakeVisible (&trackMetadataToggle);

        applyTargetAmpToggle.setButtonText ("Apply Target Amplitude");
        applyTargetAmpToggle.setToggleState (prefs.applyTargetAmplitude, dontSendNotification);
        addAndMakeVisible (&applyTargetAmpToggle);

        addAndMakeVisible (&okButton);
        okButton.setButtonText ("OK");
        okButton.addListener (this);
        addAndMakeVisible (&cancelButton);
        cancelButton.setButtonText ("Cancel");
        cancelButton.addListener (this);
    }

    bool wasAccepted () const { return accepted; }

    Preferences getPreferences () const
    {
        Preferences p;
        p.fontFamily = fontCombo.getText();
        p.fontSize = (int) fontSizeSlider.getValue();
        p.theme = themeCombo.getText();
        p.exportDir = exportEdit.getText();
        p.sampleRate = (int) sampleRateSlider.getValue();
        p.testStepDuration = testDurationSlider.getValue();
        p.trackMetadata = trackMetadataToggle.getToggleState();
        p.crossfadeCurve = crossfadeCurveCombo.getText();
        p.amplitudeDisplayMode = ampMode;
        p.applyTargetAmplitude = applyTargetAmpToggle.getToggleState();
        p.defaultVoice = prefs.defaultVoice;
        double amp = targetAmpSlider.getValue();
        if (ampMode == "dB")
            amp = dbToAmplitude (amp);
        p.targetOutputAmplitude = amp;
        return p;
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (10);
        const int labelW = 140;
        const int rowH = 26;

        auto row = area.removeFromTop (rowH);
        fontFamilyLabel.setBounds (row.removeFromLeft (labelW));
        fontCombo.setBounds (row);

        row = area.removeFromTop (rowH);
        fontSizeLabel.setBounds (row.removeFromLeft (labelW));
        fontSizeSlider.setBounds (row);

        row = area.removeFromTop (rowH);
        themeLabel.setBounds (row.removeFromLeft (labelW));
        themeCombo.setBounds (row);

        row = area.removeFromTop (rowH);
        exportLabel.setBounds (row.removeFromLeft (labelW));
        browseButton.setBounds (row.removeFromRight (80));
        exportEdit.setBounds (row);

        row = area.removeFromTop (rowH);
        sampleRateLabel.setBounds (row.removeFromLeft (labelW));
        sampleRateSlider.setBounds (row);

        row = area.removeFromTop (rowH);
        testDurationLabel.setBounds (row.removeFromLeft (labelW));
        testDurationSlider.setBounds (row);

        row = area.removeFromTop (rowH);
        targetAmpLabel.setBounds (row.removeFromLeft (labelW));
        targetAmpSlider.setBounds (row);

        row = area.removeFromTop (rowH);
        ampModeLabel.setBounds (row.removeFromLeft (labelW));
        ampModeCombo.setBounds (row);

        row = area.removeFromTop (rowH);
        crossfadeCurveLabel.setBounds (row.removeFromLeft (labelW));
        crossfadeCurveCombo.setBounds (row);

        row = area.removeFromTop (rowH);
        trackMetadataToggle.setBounds (row);

        row = area.removeFromTop (rowH);
        applyTargetAmpToggle.setBounds (row);

        area.removeFromTop (10);
        row = area.removeFromTop (30);
        okButton.setBounds (row.removeFromRight (80));
        cancelButton.setBounds (row.removeFromRight (80));
    }

private:
    Preferences prefs;
    String ampMode;
    bool accepted { false };

    Label fontFamilyLabel, fontSizeLabel, themeLabel, exportLabel,
          sampleRateLabel, testDurationLabel, targetAmpLabel,
          ampModeLabel, crossfadeCurveLabel;

    ComboBox fontCombo, themeCombo, ampModeCombo, crossfadeCurveCombo;
    Slider fontSizeSlider, sampleRateSlider, testDurationSlider, targetAmpSlider;
    TextEditor exportEdit;
    TextButton browseButton { "Browse" }, okButton { "OK" }, cancelButton { "Cancel" };
    ToggleButton trackMetadataToggle, applyTargetAmpToggle;

    void buttonClicked (Button* b) override
    {
        if (b == &browseButton)
        {
            FileChooser chooser ("Select Export Directory", File (exportEdit.getText()));
            if (chooser.browseForDirectory())
                exportEdit.setText (chooser.getResult().getFullPathName());
        }
        else if (b == &okButton)
        {
            accepted = true;
            if (auto* dw = findParentComponentOfClass<DialogWindow>())
                dw->exitModalState (1);
        }
        else if (b == &cancelButton)
        {
            if (auto* dw = findParentComponentOfClass<DialogWindow>())
                dw->exitModalState (0);
        }
    }

    void comboBoxChanged (ComboBox* box) override
    {
        if (box == &ampModeCombo)
            updateAmpMode (ampModeCombo.getText());
    }

    void updateAmpMode (const String& mode)
    {
        double value = targetAmpSlider.getValue();
        if (ampMode == "dB" && mode == "absolute")
            value = dbToAmplitude (value);
        else if (ampMode == "absolute" && mode == "dB")
            value = amplitudeToDb (value);

        ampMode = mode;
        if (mode == "dB")
        {
            targetAmpSlider.setRange (MIN_DB, 0.0, 1.0);
            targetAmpSlider.setTextValueSuffix (" dB");
        }
        else
        {
            targetAmpSlider.setRange (0.0, 1.0, 0.01);
            targetAmpSlider.setTextValueSuffix ("");
        }
        targetAmpSlider.setValue (value, dontSendNotification);
    }
};

bool showPreferencesDialog (Preferences& prefs)
{
    PreferencesDialog dialog (prefs);
    DialogWindow::LaunchOptions opts;
    opts.content.setOwned (&dialog);
    opts.dialogTitle = "Preferences";
    opts.dialogBackgroundColour = Colours::lightgrey;
    opts.escapeKeyTriggersCloseButton = true;
    opts.useNativeTitleBar = true;
    opts.resizable = false;
    int result = opts.runModal();
    if (result != 0 && dialog.wasAccepted())
    {
        prefs = dialog.getPreferences();
        return true;
    }
    return false;
}


#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class GlobalSettingsComponent : public juce::Component,
                                private juce::Button::Listener
{
public:
    struct Settings
    {
        double sampleRate { 44100.0 };
        double crossfadeSeconds { 1.0 };
        juce::String outputFile { "my_track.wav" };
        juce::String noiseFile;
        double noiseAmp { 0.0 };
    };

    GlobalSettingsComponent();
    ~GlobalSettingsComponent() override;

    Settings getSettings() const;
    void setSettings(const Settings& s);

    void resized() override;

private:
    void buttonClicked(juce::Button*) override;

    juce::Label srLabel, cfLabel, outFileLabel, noiseFileLabel, noiseAmpLabel;
    juce::TextEditor srEdit, cfEdit, outFileEdit, noiseFileEdit, noiseAmpEdit;
    juce::TextButton browseOutButton {"Browse"}, browseNoiseButton {"Browse"};
};

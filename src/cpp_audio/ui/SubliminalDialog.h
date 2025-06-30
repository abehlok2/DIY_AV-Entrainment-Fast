#pragma once
#include "../models/TrackData.h"
#include <juce_gui_basics/juce_gui_basics.h>

struct SubliminalDialog : public juce::Component,
                          private juce::Button::Listener
{
    explicit SubliminalDialog(bool ampInDb = false);

    bool wasAccepted() const;
    Voice getVoice() const;

    void paint(juce::Graphics& g) override;

    void resized() override;

private:
    bool amplitudeInDb = false;
    bool accepted = false;
    Voice voice;

    juce::Label fileLabel, freqLabel, ampLabel, modeLabel;
    juce::TextEditor fileEdit;
    juce::TextButton browseButton { "Browse" };
    juce::Slider freqSlider { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
    juce::Slider ampSlider  { juce::Slider::LinearHorizontal, juce::Slider::TextBoxRight };
    juce::ComboBox modeBox;
    juce::TextButton addButton { "Add" }, cancelButton { "Cancel" };

    void buttonClicked(juce::Button*) override;
    void onAccept();
};

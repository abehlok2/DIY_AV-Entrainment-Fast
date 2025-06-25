#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include "Preferences.h"

class DefaultVoiceDialog  : public juce::DialogWindow,
                            private juce::Button::Listener
{
public:
    explicit DefaultVoiceDialog(Preferences& prefs);
    ~DefaultVoiceDialog() override;

    juce::var getDefaultVoice() const;

    void closeButtonPressed() override;
    void buttonClicked(juce::Button* button) override;
    void resized() override;

private:
    void saveVoice();

    Preferences& preferences;

    juce::Label synthLabel;
    juce::TextEditor synthEditor;
    juce::ToggleButton transitionToggle;

    juce::Label paramsLabel;
    juce::TextEditor paramsEditor;

    juce::Label envelopeLabel;
    juce::TextEditor envelopeEditor;

    juce::TextButton saveButton;
    juce::TextButton cancelButton;
};


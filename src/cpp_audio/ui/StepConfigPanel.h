#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class StepConfigPanel : public juce::Component,
                        private juce::ListBoxModel,
                        private juce::Button::Listener
{
public:
    StepConfigPanel();
    ~StepConfigPanel() override;

    int getNumRows() override;
    void paintListBoxItem(int row, juce::Graphics&, int width, int height, bool selected) override;
    void resized() override;

private:
    juce::ListBox voiceList;
    juce::TextButton addButton, dupButton, removeButton, upButton, downButton;
    juce::StringArray voices;

    void buttonClicked(juce::Button*) override;
    void addVoice();
    void duplicateVoice();
    void removeVoice();
    void moveVoice(int delta);
};

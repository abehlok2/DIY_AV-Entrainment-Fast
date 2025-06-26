#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "VoiceEditorDialog.h"

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

    void setVoices(const juce::Array<VoiceEditorDialog::VoiceData>& v);
    juce::Array<VoiceEditorDialog::VoiceData> getVoices() const;

    std::function<void()> onVoicesChanged;

private:
    juce::ListBox voiceList;
    juce::TextButton addButton, dupButton, removeButton, editButton, upButton, downButton;
    juce::Array<VoiceEditorDialog::VoiceData> voices;

    void buttonClicked(juce::Button*) override;
    void addVoice();
    void duplicateVoice();
    void removeVoice();
    void editVoice();
    void moveVoice(int delta);
};

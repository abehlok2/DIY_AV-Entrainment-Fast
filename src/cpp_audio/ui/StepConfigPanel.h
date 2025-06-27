#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>
#include "VoiceEditorComponent.h"

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

    void setVoices(const juce::Array<VoiceEditorComponent::VoiceData>& v);
    juce::Array<VoiceEditorComponent::VoiceData> getVoices() const;

    std::function<void()> onVoicesChanged;

private:
    juce::ListBox voiceList;
    juce::TextButton addButton, dupButton, removeButton, editButton, upButton, downButton;
    juce::Array<VoiceEditorComponent::VoiceData> voices;
    std::unique_ptr<VoiceEditorComponent> editor;

    void buttonClicked(juce::Button*) override;
    void addVoice();
    void duplicateVoice();
    void removeVoice();
    void editVoice();
    void moveVoice(int delta);
};

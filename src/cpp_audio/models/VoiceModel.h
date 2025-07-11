#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include <string>
#include "TrackData.h"

class VoiceModel : public juce::TableListBoxModel
{
public:
    VoiceModel(std::vector<Voice>* voices = nullptr, juce::TableListBox* owner = nullptr);

    void setOwner(juce::TableListBox* newOwner) { owner = newOwner; }
    juce::TableListBox* getOwner() const { return owner; }

    int getNumRows() override;
    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;

    void refresh(std::vector<Voice>* newVoices = nullptr);

private:
    juce::String formatNumber(const juce::var& value) const;
    juce::String getBeatFrequency(const juce::NamedValueSet& params, bool isTransition) const;
    std::vector<Voice>* voices;
    juce::TableListBox* owner { nullptr };
};

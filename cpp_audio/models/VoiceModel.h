#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include <string>
#include "../Track.h"

class VoiceModel : public juce::TableListBoxModel
{
public:
    VoiceModel(std::vector<Voice>* voices = nullptr);

    int getNumRows() override;
    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;

    void refresh(std::vector<Voice>* newVoices = nullptr);

private:
    juce::String formatNumber(const juce::var& value) const;
    juce::String getBeatFrequency(const juce::NamedValueSet& params, bool isTransition) const;
    std::vector<Voice>* voices;
};

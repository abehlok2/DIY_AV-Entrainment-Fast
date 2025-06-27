#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include <string>
#include "../core/Track.h"

class StepModel : public juce::TableListBoxModel
{
public:
    StepModel(std::vector<Step>* steps = nullptr, juce::TableListBox* owner = nullptr);

    void setOwner(juce::TableListBox* newOwner) { owner = newOwner; }
    juce::TableListBox* getOwner() const { return owner; }

    int getNumRows() override;
    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;

    void refresh(std::vector<Step>* newSteps = nullptr);

private:
    std::vector<Step>* steps;
    juce::TableListBox* owner { nullptr };
};

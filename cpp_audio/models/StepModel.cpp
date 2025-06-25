#include "StepModel.h"

StepModel::StepModel(std::vector<Step>* stepsIn)
    : steps(stepsIn) {}

int StepModel::getNumRows()
{
    return steps ? static_cast<int>(steps->size()) : 0;
}

void StepModel::paintRowBackground(juce::Graphics& g, int /*rowNumber*/, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colours::lightblue);
}

void StepModel::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/)
{
    if (!steps || rowNumber >= (int)steps->size())
        return;

    const Step& step = (*steps)[rowNumber];
    juce::String text;
    if (columnId == 1)
        text = juce::String(step.durationSeconds, 2);
    else if (columnId == 2)
        text = step.description;
    else if (columnId == 3)
        text = juce::String((int)step.voices.size());

    g.drawText(text, 0, 0, width, height, juce::Justification::centredLeft, true);
}

void StepModel::refresh(std::vector<Step>* newSteps)
{
    if (newSteps)
        steps = newSteps;
    if (auto* owner = getOwner())
        owner->updateContent();
}

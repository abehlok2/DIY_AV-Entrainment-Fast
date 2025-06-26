#include "StepConfigPanel.h"
#include "VoiceEditorDialog.h"

using namespace juce;

StepConfigPanel::StepConfigPanel()
{
    addAndMakeVisible(&voiceList);
    voiceList.setModel(this);

    addButton.setButtonText("Add Voice");
    dupButton.setButtonText("Duplicate Voice");
    removeButton.setButtonText("Remove Voice");
    editButton.setButtonText("Edit Voice");
    upButton.setButtonText("Move Up");
    downButton.setButtonText("Move Down");

    for (auto* b : { &addButton, &dupButton, &removeButton, &editButton, &upButton, &downButton })
    {
        addAndMakeVisible(b);
        b->addListener(this);
    }
}

StepConfigPanel::~StepConfigPanel()
{
    for (auto* b : { &addButton, &dupButton, &removeButton, &editButton, &upButton, &downButton })
        b->removeListener(this);
}

int StepConfigPanel::getNumRows()
{
    return voices.size();
}

void StepConfigPanel::paintListBoxItem(int row, Graphics& g, int width, int height, bool selected)
{
    if (selected)
        g.fillAll(Colours::lightblue);

    if (isPositiveAndBelow(row, voices.size()))
    {
        g.setColour(Colours::black);
        g.drawText(voices[row], 4, 0, width - 4, height, Justification::centredLeft);
    }
}

void StepConfigPanel::resized()
{
    auto area = getLocalBounds().reduced(4);
    voiceList.setBounds(area.removeFromTop(getHeight() - 48));

    auto buttons = area.removeFromTop(44);
    auto each = buttons.getWidth() / 6;
    addButton.setBounds(buttons.removeFromLeft(each));
    dupButton.setBounds(buttons.removeFromLeft(each));
    removeButton.setBounds(buttons.removeFromLeft(each));
    editButton.setBounds(buttons.removeFromLeft(each));
    upButton.setBounds(buttons.removeFromLeft(each));
    downButton.setBounds(buttons);
}

void StepConfigPanel::buttonClicked(Button* b)
{
    if (b == &addButton)
        addVoice();
    else if (b == &dupButton)
        duplicateVoice();
    else if (b == &removeButton)
        removeVoice();
    else if (b == &editButton)
        editVoice();
    else if (b == &upButton)
        moveVoice(-1);
    else if (b == &downButton)
        moveVoice(1);

    voiceList.updateContent();
    voiceList.repaint();
}

void StepConfigPanel::addVoice()
{
    voices.add("Voice " + String(voices.size() + 1));
    voiceList.selectRow(voices.size() - 1);
}

void StepConfigPanel::duplicateVoice()
{
    int row = voiceList.getSelectedRow();
    if (isPositiveAndBelow(row, voices.size()))
    {
        voices.insert(row + 1, voices[row] + " (Copy)");
        voiceList.selectRow(row + 1);
    }
}

void StepConfigPanel::removeVoice()
{
    int row = voiceList.getSelectedRow();
    if (isPositiveAndBelow(row, voices.size()))
    {
        voices.remove(row);
        if (row > 0)
            voiceList.selectRow(row - 1);
    }
}

void StepConfigPanel::moveVoice(int delta)
{
    int row = voiceList.getSelectedRow();
    int target = row + delta;
    if (isPositiveAndBelow(row, voices.size()) && isPositiveAndBelow(target, voices.size()))
    {
        voices.move(row, target);
        voiceList.selectRow(target);
    }
}

void StepConfigPanel::editVoice()
{
    int row = voiceList.getSelectedRow();
    if (!isPositiveAndBelow(row, voices.size()))
        return;

    juce::StringArray synthNames;
    synthNames.add("binaural_beat");
    synthNames.add("isochronic_tone");

    bool ok = false;
    auto data = showVoiceEditor(synthNames, nullptr, nullptr, &ok);
    if (ok)
    {
        juce::String label = data.description.isNotEmpty() ? data.description
                                                            : data.synthFunction;
        voices.set(row, label);
    }
}

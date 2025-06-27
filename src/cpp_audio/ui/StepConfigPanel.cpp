#include "StepConfigPanel.h"
#include "VoiceEditorComponent.h"
#include <memory>
#include "../Track.h"

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
        auto label = voices[row].description.isNotEmpty() ? voices[row].description
                                                          : voices[row].synthFunction;
        g.drawText(label, 4, 0, width - 4, height, Justification::centredLeft);
    }
}

void StepConfigPanel::resized()
{
    auto area = getLocalBounds().reduced(4);
    if (editor)
    {
        auto left = area.removeFromLeft(area.getWidth() / 2);
        voiceList.setBounds(left.removeFromTop(getHeight() - 48));
        auto buttons = left.removeFromTop(44);
        auto each = buttons.getWidth() / 6;
        addButton.setBounds(buttons.removeFromLeft(each));
        dupButton.setBounds(buttons.removeFromLeft(each));
        removeButton.setBounds(buttons.removeFromLeft(each));
        editButton.setBounds(buttons.removeFromLeft(each));
        upButton.setBounds(buttons.removeFromLeft(each));
        downButton.setBounds(buttons);
        editor->setBounds(area);
    }
    else
    {
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
    if (onVoicesChanged)
        onVoicesChanged();
}

void StepConfigPanel::addVoice()
{
    juce::StringArray synthNames;
    for (const auto& n : getAvailableSynthNames())
        synthNames.add(n);

    editor.reset(new VoiceEditorComponent(synthNames));
    addAndMakeVisible(editor.get());
    editor->onSave = [this](const VoiceEditorComponent::VoiceData& d)
    {
        voices.add(d);
        voiceList.selectRow(voices.size() - 1);
        editor.reset();
        resized();
        if (onVoicesChanged)
            onVoicesChanged();
    };
    editor->onCancel = [this]() { editor.reset(); resized(); };
    resized();
}

void StepConfigPanel::duplicateVoice()
{
    int row = voiceList.getSelectedRow();
    if (isPositiveAndBelow(row, voices.size()))
    {
        auto copy = voices[row];
        copy.description = copy.description + " (Copy)";
        voices.insert(row + 1, copy);
        voiceList.selectRow(row + 1);
        if (onVoicesChanged)
            onVoicesChanged();
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
        if (onVoicesChanged)
            onVoicesChanged();
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
        if (onVoicesChanged)
            onVoicesChanged();
    }
}

void StepConfigPanel::editVoice()
{
    int row = voiceList.getSelectedRow();
    if (!isPositiveAndBelow(row, voices.size()))
        return;

    juce::StringArray synthNames;
    for (const auto& n : getAvailableSynthNames())
        synthNames.add(n);

    editor.reset(new VoiceEditorComponent(synthNames, &voices.getReference(row), nullptr));
    addAndMakeVisible(editor.get());
    editor->onSave = [this, row](const VoiceEditorComponent::VoiceData& d)
    {
        voices.getReference(row) = d;
        editor.reset();
        resized();
        if (onVoicesChanged)
            onVoicesChanged();
    };
    editor->onCancel = [this]() { editor.reset(); resized(); };
    resized();
}

void StepConfigPanel::setVoices(const juce::Array<VoiceEditorComponent::VoiceData>& v)
{
    voices = v;
    voiceList.updateContent();
    voiceList.repaint();
}

juce::Array<VoiceEditorComponent::VoiceData> StepConfigPanel::getVoices() const
{
    return voices;
}

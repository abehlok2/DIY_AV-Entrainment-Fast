// Simplified StepConfigPanel implemented with JUCE widgets.  The panel hosts a
// list of voices for the currently selected step and provides controls for
// adding, duplicating, deleting and reordering voices.  It mirrors a subset of
// the behaviour found in the PyQt step configuration panel.

#include <juce_gui_basics/juce_gui_basics.h>

using namespace juce;

class StepConfigPanel  : public Component,
                         private ListBoxModel,
                         private Button::Listener
{
public:
    StepConfigPanel()
    {
        addAndMakeVisible(&voiceList);
        voiceList.setModel(this);

        addButton.setButtonText("Add Voice");
        dupButton.setButtonText("Duplicate Voice");
        removeButton.setButtonText("Remove Voice");
        upButton.setButtonText("Move Up");
        downButton.setButtonText("Move Down");

        for (auto* b : { &addButton, &dupButton, &removeButton, &upButton, &downButton })
        {
            addAndMakeVisible(b);
            b->addListener(this);
        }
    }

    ~StepConfigPanel() override
    {
        for (auto* b : { &addButton, &dupButton, &removeButton, &upButton, &downButton })
            b->removeListener(this);
    }

    //=========================================================================
    // ListBoxModel implementation
    int getNumRows() override { return voices.size(); }

    void paintListBoxItem(int row, Graphics& g, int width, int height, bool selected) override
    {
        if (selected)
            g.fillAll(Colours::lightblue);

        if (isPositiveAndBelow(row, voices.size()))
        {
            g.setColour(Colours::black);
            g.drawText(voices[row], 4, 0, width - 4, height, Justification::centredLeft);
        }
    }

    //=========================================================================
    void resized() override
    {
        auto area = getLocalBounds().reduced(4);
        voiceList.setBounds(area.removeFromTop(getHeight() - 48));

        auto buttons = area.removeFromTop(44);
        auto each = buttons.getWidth() / 5;
        addButton.setBounds(buttons.removeFromLeft(each));
        dupButton.setBounds(buttons.removeFromLeft(each));
        removeButton.setBounds(buttons.removeFromLeft(each));
        upButton.setBounds(buttons.removeFromLeft(each));
        downButton.setBounds(buttons);
    }

private:
    ListBox voiceList;
    TextButton addButton, dupButton, removeButton, upButton, downButton;
    StringArray voices;

    //=========================================================================
    void buttonClicked(Button* b) override
    {
        if (b == &addButton)
            addVoice();
        else if (b == &dupButton)
            duplicateVoice();
        else if (b == &removeButton)
            removeVoice();
        else if (b == &upButton)
            moveVoice(-1);
        else if (b == &downButton)
            moveVoice(1);

        voiceList.updateContent();
        voiceList.repaint();
    }

    void addVoice()
    {
        voices.add("Voice " + String(voices.size() + 1));
        voiceList.selectRow(voices.size() - 1);
    }

    void duplicateVoice()
    {
        int row = voiceList.getSelectedRow();
        if (isPositiveAndBelow(row, voices.size()))
        {
            voices.insert(row + 1, voices[row] + " (Copy)");
            voiceList.selectRow(row + 1);
        }
    }

    void removeVoice()
    {
        int row = voiceList.getSelectedRow();
        if (isPositiveAndBelow(row, voices.size()))
        {
            voices.remove(row);
            if (row > 0)
                voiceList.selectRow(row - 1);
        }
    }

    void moveVoice(int delta)
    {
        int row = voiceList.getSelectedRow();
        int target = row + delta;
        if (isPositiveAndBelow(row, voices.size()) && isPositiveAndBelow(target, voices.size()))
        {
            voices.move(row, target);
            voiceList.selectRow(target);
        }
    }
};



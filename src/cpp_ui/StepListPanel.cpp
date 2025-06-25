// Basic StepListPanel implementation using JUCE widgets.  This panel mimics the
// functionality provided by the original PyQt version found in
// ``src/ui/step_list_panel.py``.  It maintains a list of steps and exposes
// controls for adding, duplicating, removing and reordering them.  The panel is
// intentionally simple and is suitable for unit testing of the underlying C++
// logic without requiring the full application framework.

#include <juce_gui_basics/juce_gui_basics.h>

using namespace juce;

class StepListPanel  : public Component,
                       private ListBoxModel,
                       private Button::Listener
{
public:
    StepListPanel()
    {
        addAndMakeVisible(stepList);
        stepList.setModel(this);

        addButton.setButtonText("Add Step");
        dupButton.setButtonText("Duplicate Step");
        removeButton.setButtonText("Remove Step");
        upButton.setButtonText("Move Up");
        downButton.setButtonText("Move Down");
        undoButton.setButtonText("Undo");
        redoButton.setButtonText("Redo");

        for (auto* b : { &addButton, &dupButton, &removeButton, &upButton, &downButton, &undoButton, &redoButton })
        {
            addAndMakeVisible(b);
            b->addListener(this);
        }

        setWantsKeyboardFocus(true);

        pushHistory();

        addAndMakeVisible(totalDuration);
        totalDuration.setJustificationType(Justification::centredLeft);

        // start with an empty step list
    }

    ~StepListPanel() override
    {
        for (auto* b : { &addButton, &dupButton, &removeButton, &upButton, &downButton, &undoButton, &redoButton })
            b->removeListener(this);
    }

    //=========================================================================
    // ListBoxModel overrides
    int getNumRows() override
    {
        return steps.size();
    }

    void paintListBoxItem(int row, Graphics& g, int width, int height, bool rowIsSelected) override
    {
        if (rowIsSelected)
            g.fillAll(Colours::lightblue);

        if (isPositiveAndBelow(row, steps.size()))
        {
            g.setColour(Colours::black);
            g.drawText(steps[row], 4, 0, width - 4, height, Justification::centredLeft);
        }
    }

    //=========================================================================
    // Component layout
    void resized() override
    {
        auto area = getLocalBounds().reduced(4);
        auto top = area.removeFromTop(24);
        totalDuration.setBounds(top);

        area.removeFromTop(4);
        stepList.setBounds(area.removeFromTop(getHeight() - 80));

        auto buttons = area.removeFromTop(48);
        auto each = buttons.getWidth() / 7;
        addButton.setBounds(buttons.removeFromLeft(each));
        dupButton.setBounds(buttons.removeFromLeft(each));
        removeButton.setBounds(buttons.removeFromLeft(each));
        upButton.setBounds(buttons.removeFromLeft(each));
        downButton.setBounds(buttons.removeFromLeft(each));
        undoButton.setBounds(buttons.removeFromLeft(each));
        redoButton.setBounds(buttons);
    }

private:
    ListBox stepList;
    TextButton addButton, dupButton, removeButton, upButton, downButton,
              undoButton, redoButton;
    Label totalDuration;
    StringArray steps;
    Array<StringArray> history;
    int historyIndex { -1 };

    //=========================================================================
    void buttonClicked(Button* b) override
    {
        if (b == &addButton)
            addStep();
        else if (b == &dupButton)
            duplicateStep();
        else if (b == &removeButton)
            removeStep();
        else if (b == &upButton)
            moveStep(-1);
        else if (b == &downButton)
            moveStep(1);
        else if (b == &undoButton)
            undo();
        else if (b == &redoButton)
            redo();

        stepList.updateContent();
        stepList.repaint();
        updateDuration();
    }

    void addStep()
    {
        steps.add("New Step " + String(steps.size() + 1));
        stepList.selectRow(steps.size() - 1);
        pushHistory();
    }

    void duplicateStep()
    {
        int row = stepList.getSelectedRow();
        if (isPositiveAndBelow(row, steps.size()))
        {
            steps.insert(row + 1, steps[row] + " (Copy)");
            stepList.selectRow(row + 1);
            pushHistory();
        }
    }

    void removeStep()
    {
        int row = stepList.getSelectedRow();
        if (isPositiveAndBelow(row, steps.size()))
        {
            steps.remove(row);
            if (row > 0)
                stepList.selectRow(row - 1);
            pushHistory();
        }
    }

    void moveStep(int delta)
    {
        int row = stepList.getSelectedRow();
        int target = row + delta;
        if (isPositiveAndBelow(row, steps.size()) && isPositiveAndBelow(target, steps.size()))
        {
            steps.move(row, target);
            stepList.selectRow(target);
            pushHistory();
        }
    }

    void updateDuration()
    {
        int totalSecs = steps.size() * 30; // placeholder constant duration
        int mins = totalSecs / 60;
        int secs = totalSecs % 60;
        totalDuration.setText("Total Duration: " + String(mins).paddedLeft('0',2)
                              + ":" + String(secs).paddedLeft('0',2), dontSendNotification);
    }

    //==========================================================================
    void pushHistory()
    {
        if (historyIndex < history.size() - 1)
            history.removeRange(historyIndex + 1, history.size() - historyIndex - 1);
        history.add(steps);
        historyIndex = history.size() - 1;
        updateUndoRedoButtons();
    }

    void undo()
    {
        if (historyIndex > 0)
        {
            --historyIndex;
            steps = history[historyIndex];
            stepList.selectRow(juce::jlimit(0, steps.size() - 1, stepList.getSelectedRow()));
            stepList.updateContent();
            stepList.repaint();
            updateDuration();
        }
        updateUndoRedoButtons();
    }

    void redo()
    {
        if (historyIndex < history.size() - 1)
        {
            ++historyIndex;
            steps = history[historyIndex];
            stepList.selectRow(juce::jlimit(0, steps.size() - 1, stepList.getSelectedRow()));
            stepList.updateContent();
            stepList.repaint();
            updateDuration();
        }
        updateUndoRedoButtons();
    }

    void updateUndoRedoButtons()
    {
        undoButton.setEnabled(historyIndex > 0);
        redoButton.setEnabled(historyIndex < history.size() - 1);
    }

    bool keyPressed(const KeyPress& key) override
    {
        if (key == KeyPress('z', ModifierKeys::commandModifier, 0))
        {
            undo();
            return true;
        }
        if (key == KeyPress('z', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0))
        {
            redo();
            return true;
        }
        return false;
    }
};



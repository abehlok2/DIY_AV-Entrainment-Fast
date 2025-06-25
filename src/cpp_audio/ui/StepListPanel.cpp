
#include "StepListPanel.h"
#include "../VarUtils.h"

using namespace juce;

StepListPanel::StepListPanel()
{
public:
    StepListPanel()
    {
        addAndMakeVisible(stepList);
        stepList.setModel(this);

        addButton.setButtonText("Add Step");
        loadButton.setButtonText("Load Steps");
        dupButton.setButtonText("Duplicate Step");
        removeButton.setButtonText("Remove Step");
        editDurationButton.setButtonText("Edit Duration");
        editDescriptionButton.setButtonText("Edit Description");
        upButton.setButtonText("Move Up");
        downButton.setButtonText("Move Down");
        undoButton.setButtonText("Undo");
        redoButton.setButtonText("Redo");

        for (auto* b : { &addButton, &loadButton, &dupButton, &removeButton, &editDurationButton,
                          &editDescriptionButton, &upButton, &downButton,
                          &undoButton, &redoButton })
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
        for (auto* b : { &addButton, &loadButton, &dupButton, &removeButton, &editDurationButton,
                        &editDescriptionButton, &upButton, &downButton,
                        &undoButton, &redoButton })
            b->removeListener(this);

    }

    setWantsKeyboardFocus(true);

    pushHistory();

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
        auto each = buttons.getWidth() / 10;
        addButton.setBounds(buttons.removeFromLeft(each));
        loadButton.setBounds(buttons.removeFromLeft(each));
        dupButton.setBounds(buttons.removeFromLeft(each));
        removeButton.setBounds(buttons.removeFromLeft(each));
        editDurationButton.setBounds(buttons.removeFromLeft(each));
        editDescriptionButton.setBounds(buttons.removeFromLeft(each));
        upButton.setBounds(buttons.removeFromLeft(each));
        downButton.setBounds(buttons.removeFromLeft(each));
        undoButton.setBounds(buttons.removeFromLeft(each));
        redoButton.setBounds(buttons);
    }

private:
    ListBox stepList;
    TextButton addButton, loadButton, dupButton, removeButton, editDurationButton,
              editDescriptionButton, upButton, downButton,
              undoButton, redoButton;
    Label totalDuration;


int StepListPanel::getNumRows()
{
    return steps.size();
}

void StepListPanel::paintListBoxItem(int row, Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(Colours::lightblue);

    if (isPositiveAndBelow(row, steps.size()))
    {

        if (b == &addButton)
            addStep();
        else if (b == &loadButton)
            loadExternalSteps();
        else if (b == &dupButton)
            duplicateStep();
        else if (b == &removeButton)
            removeStep();
        else if (b == &editDurationButton)
            editStepDuration();
        else if (b == &editDescriptionButton)
            editStepDescription();
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
}

void StepListPanel::resized()
{
    auto area = getLocalBounds().reduced(4);
    auto top = area.removeFromTop(24);
    totalDuration.setBounds(top);

    area.removeFromTop(4);
    stepList.setBounds(area.removeFromTop(getHeight() - 80));

    auto buttons = area.removeFromTop(48);
    auto each = buttons.getWidth() / 9;
    addButton.setBounds(buttons.removeFromLeft(each));
    dupButton.setBounds(buttons.removeFromLeft(each));
    removeButton.setBounds(buttons.removeFromLeft(each));
    editDurationButton.setBounds(buttons.removeFromLeft(each));
    editDescriptionButton.setBounds(buttons.removeFromLeft(each));
    upButton.setBounds(buttons.removeFromLeft(each));
    downButton.setBounds(buttons.removeFromLeft(each));
    undoButton.setBounds(buttons.removeFromLeft(each));
    redoButton.setBounds(buttons);
}

void StepListPanel::buttonClicked(Button* b)
{
    if (b == &addButton)
        addStep();
    else if (b == &dupButton)
        duplicateStep();
    else if (b == &removeButton)
        removeStep();
    else if (b == &editDurationButton)
        editStepDuration();
    else if (b == &editDescriptionButton)
        editStepDescription();
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

void StepListPanel::addStep()
{
    StepData s;
    s.description = "New Step " + String(steps.size() + 1);
    s.duration = 10.0;
    steps.add(s);
    stepList.selectRow(steps.size() - 1);
    pushHistory();
}

void StepListPanel::duplicateStep()
{
    int row = stepList.getSelectedRow();
    if (isPositiveAndBelow(row, steps.size()))
    {
        StepData copy = steps[row];
        copy.description += " (Copy)";
        steps.insert(row + 1, copy);
        stepList.selectRow(row + 1);
        pushHistory();
    }
}


    void loadExternalSteps()
    {
        FileChooser chooser("Load External Steps from JSON", {}, "*.json");
        if (! chooser.browseForFileToOpen())
            return;

        auto file = chooser.getResult();
        std::unique_ptr<FileInputStream> stream(file.createInputStream());
        if (! stream || ! stream->openedOk())
            return;

        var parsed = JSON::parse(stream->readEntireStreamAsString());
        if (auto* obj = parsed.getDynamicObject())
        {
            if (auto* stepsVar = obj->getProperty("steps").getArray())
            {
                for (const auto& s : *stepsVar)
                {
                    if (auto* sobj = s.getDynamicObject())
                    {
                        double dur = getPropertyWithDefault(sobj, "duration", 0.0);
                        if (dur <= 0.0)
                            continue;
                        String desc = sobj->getProperty("description").toString();
                        StepData sd;
                        sd.duration = dur;
                        sd.description = desc.isNotEmpty() ? desc : String("Step ") + String(steps.size()+1);
                        steps.add(sd);
                    }
                }
                pushHistory();
            }
        }
    }

    void duplicateStep()

    {
        steps.remove(row);
        if (row > 0)
            stepList.selectRow(row - 1);
        pushHistory();
    }
}

void StepListPanel::moveStep(int delta)
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

void StepListPanel::editStepDuration()
{
    int row = stepList.getSelectedRow();
    if (!isPositiveAndBelow(row, steps.size()))
        return;

    AlertWindow w("Edit Duration", "Enter new duration (seconds):", AlertWindow::NoIcon);
    w.addTextEditor("dur", String(steps[row].duration, 3));
    w.addButton("OK", 1, KeyPress(KeyPress::returnKey));
    w.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));
    if (w.runModalLoop() == 1)
    {
        double val = w.getTextEditor("dur")->getText().trim().getDoubleValue();
        if (val > 0.0)
        {
            steps.getReference(row).duration = val;
            pushHistory();
        }
    }
    updateDuration();
}

void StepListPanel::editStepDescription()
{
    int row = stepList.getSelectedRow();
    if (!isPositiveAndBelow(row, steps.size()))
        return;

    AlertWindow w("Edit Description", "Enter new description:", AlertWindow::NoIcon);
    w.addTextEditor("desc", steps[row].description);
    w.addButton("OK", 1, KeyPress(KeyPress::returnKey));
    w.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));
    if (w.runModalLoop() == 1)
    {
        steps.getReference(row).description = w.getTextEditor("desc")->getText();
        pushHistory();
    }
    stepList.updateContent();
    stepList.repaint();
}

void StepListPanel::updateDuration()
{
    double totalSecs = 0.0;
    for (const auto& s : steps)
        totalSecs += s.duration;
    int mins = static_cast<int>(totalSecs) / 60;
    int secs = static_cast<int>(totalSecs) % 60;
    totalDuration.setText("Total Duration: " + String(mins).paddedLeft('0',2)
                          + ":" + String(secs).paddedLeft('0',2), dontSendNotification);
}

void StepListPanel::pushHistory()
{
    if (historyIndex < history.size() - 1)
        history.removeRange(historyIndex + 1, history.size() - historyIndex - 1);
    history.add(steps);
    historyIndex = history.size() - 1;
    updateUndoRedoButtons();
}

void StepListPanel::undo()
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

void StepListPanel::redo()
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

bool StepListPanel::canUndo() const
{
    return historyIndex > 0;
}

bool StepListPanel::canRedo() const
{
    return historyIndex < history.size() - 1;
}

void StepListPanel::updateUndoRedoButtons()
{
    undoButton.setEnabled(canUndo());
    redoButton.setEnabled(canRedo());
}

bool StepListPanel::keyPressed(const KeyPress& key)
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


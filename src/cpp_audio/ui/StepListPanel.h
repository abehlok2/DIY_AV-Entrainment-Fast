#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

class StepListPanel : public juce::Component,
                      private juce::ListBoxModel,
                      private juce::ListBox::Listener,
                      private juce::Button::Listener
{
public:
    StepListPanel();
    ~StepListPanel() override;

    int getNumRows() override;
    void paintListBoxItem(int row, juce::Graphics&, int width, int height, bool rowIsSelected) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress&) override;

    // Expose undo/redo functionality
    void undo();
    void redo();
    bool canUndo() const;
    bool canRedo() const;

    // Access steps and selection
    const juce::Array<StepData>& getSteps() const { return steps; }
    int getSelectedIndex() const { return stepList.getSelectedRow(); }

    std::function<void(int)> onStepSelected;

private:
    // UI components
    juce::ListBox stepList;
    juce::TextButton addButton, loadButton, dupButton, removeButton,
                     editDurationButton, editDescriptionButton,
                     upButton, downButton, undoButton, redoButton;
    juce::Label totalDuration;

    struct StepData
    {
        juce::String description { "New Step" };
        double duration { 10.0 };
    };

    juce::Array<StepData> steps;
    juce::Array< juce::Array<StepData> > history;
    int historyIndex { -1 };

    void buttonClicked(juce::Button*) override;
    void addStep();
    void loadExternalSteps();
    void duplicateStep();
    void removeStep();
    void moveStep(int delta);
    void editStepDuration();
    void editStepDescription();
    void updateDuration();
    void pushHistory();
    void updateUndoRedoButtons();
    void selectedRowsChanged(int lastRowSelected) override;
};

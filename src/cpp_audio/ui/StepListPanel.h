#pragma once
#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>
#include "StepConfigPanel.h"
#include "../core/Track.h"

class StepListPanel : public juce::Component,
                      private juce::ListBoxModel,
                      private juce::Button::Listener {
public:
  // The StepData struct needs to be public so that getSteps() can return it.
  struct StepData {
    juce::String description{"New Step"};
    double duration{10.0};
    juce::Array<VoiceEditorComponent::VoiceData> voices;
  };

  StepListPanel();
  ~StepListPanel() override;

  int getNumRows() override;
  void paintListBoxItem(int row, juce::Graphics &g, int width, int height,
                        bool rowIsSelected) override;
  void resized() override;
  bool keyPressed(const juce::KeyPress &key) override;

  // Expose undo/redo functionality
  void undo();
  void redo();
  bool canUndo() const;
  bool canRedo() const;

  // Access steps and selection
  const juce::Array<StepData> &getSteps() const { return steps; }
  int getSelectedIndex() const { return stepList.getSelectedRow(); }
  void setSteps(const juce::Array<StepData>& newSteps);

  void setSteps(const std::vector<Step> &newSteps);
  std::vector<Step> toTrackSteps() const;
  void clearSteps();
  void updateStepVoices(int index, const juce::Array<VoiceEditorComponent::VoiceData>& v);

  std::function<void(int)> onStepSelected;
  std::function<void(int)> onEditVoices;

private:
  // UI components
  juce::ListBox stepList;
  juce::TextButton addButton, loadButton, dupButton, removeButton,
      editDurationButton, editDescriptionButton, editVoicesButton, upButton,
      downButton, undoButton, redoButton;
  juce::Label totalDuration;

  juce::Array<StepData> steps;
  juce::Array<juce::Array<StepData>> history;
  int historyIndex{-1};

  void buttonClicked(juce::Button *b) override;
  void addStep();
  void loadExternalSteps();
  void duplicateStep();
  void removeStep();
  void moveStep(int delta);
  void openStepConfig();
  void editStepDuration();
  void editStepDescription();
  void updateDuration();
  void pushHistory();
  void updateUndoRedoButtons();
  void selectedRowsChanged(int lastRowSelected) override;
};

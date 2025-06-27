
#include "StepListPanel.h"
#include "../VarUtils.h" // Assuming this is a valid path in your project
#include "StepConfigPanel.h"
#include "../Track.h"

using namespace juce;

StepListPanel::StepListPanel() {
  addAndMakeVisible(stepList);
  stepList.setModel(this);
  // REMOVED: stepList.addListener(this); - This is deprecated when using a
  // ListBoxModel

  addButton.setButtonText("Add Step");
  loadButton.setButtonText("Load Steps");
  dupButton.setButtonText("Duplicate Step");
  removeButton.setButtonText("Remove Step");
  editDurationButton.setButtonText("Edit Duration");
  editDescriptionButton.setButtonText("Edit Description");
  editVoicesButton.setButtonText("Edit Voices");
  upButton.setButtonText("Move Up");
  downButton.setButtonText("Move Down");
  undoButton.setButtonText("Undo");
  redoButton.setButtonText("Redo");

  for (auto *b : {&addButton, &loadButton, &dupButton, &removeButton,
                  &editDurationButton, &editDescriptionButton, &editVoicesButton,
                  &upButton, &downButton, &undoButton, &redoButton}) {
    addAndMakeVisible(b);
    b->addListener(this);
  }

  addAndMakeVisible(&totalDuration);
  totalDuration.setJustificationType(Justification::centredLeft);

  setWantsKeyboardFocus(true);
  pushHistory();
}

StepListPanel::~StepListPanel() {
  for (auto *b : {&addButton, &loadButton, &dupButton, &removeButton,
                  &editDurationButton, &editDescriptionButton, &editVoicesButton,
                  &upButton, &downButton, &undoButton, &redoButton}) {
    b->removeListener(this);
  }
  // REMOVED: stepList.removeListener(this);
  stepList.setModel(nullptr); // Good practice to null the model pointer
}

int StepListPanel::getNumRows() { return steps.size(); }

void StepListPanel::paintListBoxItem(int row, Graphics &g, int width,
                                     int height, bool rowIsSelected) {
  if (rowIsSelected)
    g.fillAll(Colours::lightblue);

  if (isPositiveAndBelow(row, steps.size())) {
    const auto &s = steps[row];
    String text = s.description + " (" + String(s.duration, 2) + "s)";
    g.setColour(Colours::black);
    g.drawText(text, 4, 0, width - 4, height, Justification::centredLeft);
  }
}

void StepListPanel::resized() {
  auto area = getLocalBounds().reduced(4);
  auto top = area.removeFromTop(24);
  totalDuration.setBounds(top);

  area.removeFromTop(4);
  auto buttonsArea = area.removeFromBottom(48);
  stepList.setBounds(area);

  auto each = buttonsArea.getWidth() / 11;
  addButton.setBounds(buttonsArea.removeFromLeft(each));
  loadButton.setBounds(buttonsArea.removeFromLeft(each));
  dupButton.setBounds(buttonsArea.removeFromLeft(each));
  removeButton.setBounds(buttonsArea.removeFromLeft(each));
  editDurationButton.setBounds(buttonsArea.removeFromLeft(each));
  editDescriptionButton.setBounds(buttonsArea.removeFromLeft(each));
  editVoicesButton.setBounds(buttonsArea.removeFromLeft(each));
  upButton.setBounds(buttonsArea.removeFromLeft(each));
  downButton.setBounds(buttonsArea.removeFromLeft(each));
  undoButton.setBounds(buttonsArea.removeFromLeft(each));
  redoButton.setBounds(buttonsArea);
}

void StepListPanel::buttonClicked(Button *b) {
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
  else if (b == &editVoicesButton)
    openStepConfig();
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

void StepListPanel::addStep() {
  StepData s;
  s.description = "New Step " + String(steps.size() + 1);
  s.duration = 10.0;
  steps.add(s);
  stepList.selectRow(steps.size() - 1);
  pushHistory();
}

void StepListPanel::loadExternalSteps() {
  FileChooser chooser("Load External Steps from JSON", {}, "*.json");
  if (chooser.browseForFileToOpen()) {
    auto file = chooser.getResult();
    auto parsed = JSON::parse(file);

    if (auto *obj = parsed.getDynamicObject()) {
      if (auto *stepsVar = obj->getProperty("steps").getArray()) {
        for (const auto &s : *stepsVar) {
          if (auto *sobj = s.getDynamicObject()) {
            double dur = withDefault(sobj->getProperty("duration"), 0.0);
            if (dur <= 0.0)
              continue;
            String desc = sobj->getProperty("description").toString();
            StepData sd;
            sd.duration = dur;
            sd.description = desc.isNotEmpty()
                                 ? desc
                                 : String("Step ") + String(steps.size() + 1);
            if (auto *voicesVar = sobj->getProperty("voices").getArray()) {
              for (const auto &v : *voicesVar) {
                if (auto *vobj = v.getDynamicObject()) {
                  VoiceEditorDialog::VoiceData vd;
                  vd.synthFunction =
                      vobj->getProperty("synth_function_name").toString();
                  vd.isTransition = withDefault(vobj->getProperty("is_transition"), false);
                  vd.params = vobj->getProperty("params");
                  vd.volumeEnvelope = vobj->getProperty("volume_envelope");
                  vd.description = vobj->getProperty("description").toString();
                  sd.voices.add(std::move(vd));
                }
              }
            }
            steps.add(sd);
          }
        }
        pushHistory();
      }
    }
  }
}

void StepListPanel::duplicateStep() {
  int row = stepList.getSelectedRow();
  if (isPositiveAndBelow(row, steps.size())) {
    StepData copy = steps[row];
    copy.description += " (Copy)";
    steps.insert(row + 1, copy);
    stepList.selectRow(row + 1);
    pushHistory();
  }
}

void StepListPanel::removeStep() {
  int row = stepList.getSelectedRow();
  if (isPositiveAndBelow(row, steps.size())) {
    steps.remove(row);
    if (row > 0)
      stepList.selectRow(row - 1);
    
    else if (!steps.isEmpty()) // FIXED: Was isNotEmpty(), which doesn't exist.

      stepList.selectRow(0);
    pushHistory();
  }
}

void StepListPanel::moveStep(int delta) {
  int row = stepList.getSelectedRow();
  int target = row + delta;
  if (isPositiveAndBelow(row, steps.size()) &&
      isPositiveAndBelow(target, steps.size())) {
    steps.move(row, target);
    stepList.selectRow(target);
    pushHistory();
  }
}

void StepListPanel::editStepDuration() {
  int row = stepList.getSelectedRow();
  if (!isPositiveAndBelow(row, steps.size()))
    return;

  AlertWindow w("Edit Duration",
                "Enter new duration (seconds):", AlertWindow::NoIcon);
  w.addTextEditor("dur", String(steps[row].duration, 3));
  w.addButton("OK", 1, KeyPress(KeyPress::returnKey));
  w.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));
  if (w.runModalLoop() == 1) {
    double val = w.getTextEditor("dur")->getText().trim().getDoubleValue();
    if (val > 0.0) {
      steps.getReference(row).duration = val;
      pushHistory();
    }
  }
  updateDuration();
}

void StepListPanel::editStepDescription() {
  int row = stepList.getSelectedRow();
  if (!isPositiveAndBelow(row, steps.size()))
    return;

  AlertWindow w("Edit Description",
                "Enter new description:", AlertWindow::NoIcon);
  w.addTextEditor("desc", steps[row].description);
  w.addButton("OK", 1, KeyPress(KeyPress::returnKey));
  w.addButton("Cancel", 0, KeyPress(KeyPress::escapeKey));
  if (w.runModalLoop() == 1) {
    steps.getReference(row).description = w.getTextEditor("desc")->getText();
    pushHistory();
  }
  stepList.updateContent();
  stepList.repaint();
}

void StepListPanel::updateDuration() {
  double totalSecs = 0.0;
  for (const auto &s : steps)
    totalSecs += s.duration;
  int mins = static_cast<int>(totalSecs) / 60;
  int secs = static_cast<int>(totalSecs) % 60;
  totalDuration.setText("Total Duration: " + String(mins).paddedLeft('0', 2) +
                            ":" + String(secs).paddedLeft('0', 2),
                        dontSendNotification);
}

void StepListPanel::pushHistory() {
  if (historyIndex < history.size() - 1)
    history.removeRange(historyIndex + 1, history.size() - historyIndex - 1);
  history.add(steps);
  historyIndex = history.size() - 1;
  updateUndoRedoButtons();
}

void StepListPanel::undo() {
  if (canUndo()) {
    --historyIndex;
    steps = history[historyIndex];
    stepList.selectRow(jlimit(0, steps.size() - 1, stepList.getSelectedRow()));
    stepList.updateContent();
    stepList.repaint();
    updateDuration();
  }
  updateUndoRedoButtons();
}

void StepListPanel::redo() {
  if (canRedo()) {
    ++historyIndex;
    steps = history[historyIndex];
    stepList.selectRow(jlimit(0, steps.size() - 1, stepList.getSelectedRow()));
    stepList.updateContent();
    stepList.repaint();
    updateDuration();
  }
  updateUndoRedoButtons();
}

bool StepListPanel::canUndo() const { return historyIndex > 0; }

bool StepListPanel::canRedo() const {
  return historyIndex < history.size() - 1;
}

void StepListPanel::updateUndoRedoButtons() {
  undoButton.setEnabled(canUndo());
  redoButton.setEnabled(canRedo());
}

bool StepListPanel::keyPressed(const KeyPress &key) {
  if (key.getModifiers() == ModifierKeys::commandModifier &&
      key.getKeyCode() == 'z') {
    undo();
    return true;
  }
  if (key.getModifiers() ==
          (ModifierKeys::commandModifier | ModifierKeys::shiftModifier) &&
      key.getKeyCode() == 'z') {
    redo();
    return true;
  }
  return false;
}

void StepListPanel::selectedRowsChanged(int lastRowSelected) {
  if (onStepSelected)
    onStepSelected(lastRowSelected);
}

void StepListPanel::openStepConfig() {
  int row = stepList.getSelectedRow();
  if (!isPositiveAndBelow(row, steps.size()))
    return;

  auto *panel = new StepConfigPanel();
  panel->setVoices(steps[row].voices);
  panel->onVoicesChanged = [this, panel, row]() {
    steps.getReference(row).voices = panel->getVoices();
  };

  DialogWindow::LaunchOptions opts;
  opts.content.setOwned(panel);
  opts.dialogTitle = "Edit Voices";
  opts.dialogBackgroundColour = Colours::lightgrey;
  opts.escapeKeyTriggersCloseButton = true;
  opts.useNativeTitleBar = true;
  opts.resizable = true;
  opts.runModal();
}


void StepListPanel::setSteps(const juce::Array<StepData>& newSteps) {
  steps = newSteps;
  stepList.updateContent();
  stepList.repaint();
  if (!steps.isEmpty())
    stepList.selectRow(0);
  pushHistory();
  updateDuration();
}

void StepListPanel::setSteps(const std::vector<Step> &newSteps) {
  steps.clear();
  for (const auto &s : newSteps) {
    StepData sd;
    sd.description = s.description;
    sd.duration = s.durationSeconds;
    for (const auto &v : s.voices) {
      VoiceEditorDialog::VoiceData vd;
      vd.synthFunction = juce::String(v.synthFunction);
      vd.isTransition = v.isTransition;
      vd.params = namedValueSetToVar(v.params);
      vd.description = v.description;
      sd.voices.add(std::move(vd));
    }
    steps.add(std::move(sd));
  }
  stepList.updateContent();
  stepList.repaint();
  if (!steps.isEmpty())
    stepList.selectRow(0);
  pushHistory();
  updateDuration();
}

std::vector<Step> StepListPanel::toTrackSteps() const {
  std::vector<Step> out;
  for (const auto &sd : steps) {
    Step s;
    s.durationSeconds = sd.duration;
    s.description = sd.description;
    for (const auto &vd : sd.voices) {
      Voice v;
      v.synthFunction = vd.synthFunction.toStdString();
      v.isTransition = vd.isTransition;
      v.description = vd.description;
      v.params = varToNamedValueSet(vd.params);
      s.voices.push_back(std::move(v));
    }
    out.push_back(std::move(s));
  }
  return out;
}

void StepListPanel::clearSteps() {
  steps.clear();
  stepList.updateContent();
  stepList.repaint();
  pushHistory();
  updateDuration();

}

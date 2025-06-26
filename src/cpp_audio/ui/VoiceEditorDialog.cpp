
#include "VoiceEditorDialog.h"

using namespace juce;

//==============================================================================
// ParameterRow implementation
//==============================================================================
ParameterRow::ParameterRow(const String &name, const String &value) {
  addAndMakeVisible(&nameLabel);
  nameLabel.setText(name, dontSendNotification);
  addAndMakeVisible(&valueEditor);
  valueEditor.setText(value);
}

void ParameterRow::resized() {
  auto area = getLocalBounds();
  nameLabel.setBounds(area.removeFromLeft(120));
  valueEditor.setBounds(area);
}

String ParameterRow::getName() const { return nameLabel.getText(); }
String ParameterRow::getValue() const { return valueEditor.getText(); }
void ParameterRow::setValue(const String &v) { valueEditor.setText(v); }

//==============================================================================
// VoiceEditorDialog implementation
//==============================================================================
VoiceEditorDialog::VoiceEditorDialog(
    const StringArray &synthNames, const VoiceData *existing,
    const std::vector<std::vector<VoiceData>> *refSteps)
    : DialogWindow("Edit Voice", Colours::lightgrey, true),
      referenceSteps(refSteps ? *refSteps
                              : std::vector<std::vector<VoiceData>>{}) {
  setUsingNativeTitleBar(true);
  setResizable(true, false);

  addAndMakeVisible(&funcLabel);
  funcLabel.setText("Synth Function", dontSendNotification);

  addAndMakeVisible(&funcCombo);
  for (int i = 0; i < synthNames.size(); ++i)
    funcCombo.addItem(synthNames[i], i + 1);

  addAndMakeVisible(&transitionToggle);
  transitionToggle.setButtonText("Is Transition");

  addAndMakeVisible(&paramsLabel);
  paramsLabel.setText("Parameters", dontSendNotification);
  addAndMakeVisible(&addParamButton);
  addParamButton.setButtonText("Add Param");
  addParamButton.addListener(this);
  addAndMakeVisible(&paramsViewport);
  paramsViewport.setViewedComponent(&paramsContainer, false);

  addAndMakeVisible(&envLabel);
  envLabel.setText("Volume Envelope", dontSendNotification);
  addAndMakeVisible(&envTypeCombo);
  envTypeCombo.addItem("None", 1);
  envTypeCombo.addItem("linear_fade", 2);
  envTypeCombo.onChange = [this] { rebuildEnvelopeUI(); };
  addAndMakeVisible(&envViewport);
  envViewport.setViewedComponent(&envContainer, false);

  addAndMakeVisible(&refStepCombo);
  addAndMakeVisible(&refVoiceCombo);
  addAndMakeVisible(&refDetails);
  refDetails.setMultiLine(true);
  refDetails.setReadOnly(true);
  refDetails.setScrollbarsShown(true);

  addAndMakeVisible(&descLabel);
  descLabel.setText("Description", dontSendNotification);
  addAndMakeVisible(&descEditor);
  descEditor.setMultiLine(true);
  descEditor.setReturnKeyStartsNewLine(true);

  addAndMakeVisible(&okButton);
  okButton.setButtonText("OK");
  okButton.addListener(this);

  addAndMakeVisible(&cancelButton);
  cancelButton.setButtonText("Cancel");
  cancelButton.addListener(this);

  if (existing)
    populateFromData(*existing);

  setSize(500, 600);
}

VoiceEditorDialog::~VoiceEditorDialog() {
  okButton.removeListener(this);
  cancelButton.removeListener(this);
  addParamButton.removeListener(this);
}

bool VoiceEditorDialog::wasAccepted() const { return accepted; }

VoiceEditorDialog::VoiceData VoiceEditorDialog::getVoiceData() const {
  return data;
}

void VoiceEditorDialog::closeButtonPressed() { exitModalState(0); }

void VoiceEditorDialog::buttonClicked(Button *b) {
  if (b == &okButton) {
    if (collectData())
      exitModalState(1);
  } else if (b == &cancelButton) {
    closeButtonPressed();
  } else if (b == &addParamButton) {
    auto *row = new ParameterRow("param", "0");
    paramRows.add(row);
    paramsContainer.addAndMakeVisible(row);
    layoutParamRows();
  }
}

void VoiceEditorDialog::resized() {
  auto area = getLocalBounds().reduced(10);
  const int labelH = 20;
  const int editorH = 60; // Reduced height for better fit
  const int buttonH = 24;
  const int gap = 6;

  funcLabel.setBounds(area.removeFromTop(labelH));
  funcCombo.setBounds(area.removeFromTop(buttonH));
  area.removeFromTop(gap);
  transitionToggle.setBounds(area.removeFromTop(buttonH));
  area.removeFromTop(gap);

  paramsLabel.setBounds(area.removeFromTop(labelH));
  addParamButton.setBounds(area.removeFromTop(buttonH));
  area.removeFromTop(4);
  paramsViewport.setBounds(area.removeFromTop(editorH));
  area.removeFromTop(gap);

  envLabel.setBounds(area.removeFromTop(labelH));
  envTypeCombo.setBounds(area.removeFromTop(buttonH));
  area.removeFromTop(4);
  envViewport.setBounds(area.removeFromTop(editorH));
  area.removeFromTop(gap);

  refStepCombo.setBounds(area.removeFromTop(buttonH));
  refVoiceCombo.setBounds(area.removeFromTop(buttonH));
  refDetails.setBounds(area.removeFromTop(editorH));
  area.removeFromTop(gap);

  descLabel.setBounds(area.removeFromTop(labelH));
  descEditor.setBounds(area.removeFromTop(editorH));
  area.removeFromTop(gap);

  auto buttons = area.removeFromBottom(30);
  okButton.setBounds(buttons.removeFromRight(80));
  buttons.removeFromRight(gap);
  cancelButton.setBounds(buttons.removeFromRight(80));
}

void VoiceEditorDialog::populateFromData(const VoiceData &d) {
  funcCombo.setText(d.synthFunction, dontSendNotification);
  transitionToggle.setToggleState(d.isTransition, dontSendNotification);

  rebuildParamUI(d.params);
  rebuildEnvelopeUI(d.volumeEnvelope);

  descEditor.setText(d.description);
  populateReferenceCombos();
}

bool VoiceEditorDialog::collectData() {
  data.synthFunction = funcCombo.getText();
  data.isTransition = transitionToggle.getToggleState();
  data.description = descEditor.getText();
  data.params = collectParamsVar();
  data.volumeEnvelope = collectEnvelopeVar();
  accepted = true;
  return true;
}

void VoiceEditorDialog::rebuildParamUI(const var &paramsVar) {
  paramRows.clear();
  paramsContainer.removeAllChildren();
  paramsContainer.setSize(300, 0);

  if (auto *obj = paramsVar.getDynamicObject()) {
    for (const auto &p : obj->getProperties()) {
      auto *row = new ParameterRow(p.name.toString(), p.value.toString());
      paramRows.add(row);
      paramsContainer.addAndMakeVisible(row);
    }
  }
  layoutParamRows();
}

void VoiceEditorDialog::layoutParamRows() {
  int y = 0;
  for (auto *r : paramRows) {
    r->setBounds(0, y, 280, 24);
    y += 26;
  }
  paramsContainer.setSize(300, y);
  paramsViewport.setViewPosition(0, 0);
}

var VoiceEditorDialog::collectParamsVar() {
  auto obj = std::make_unique<DynamicObject>();
  for (auto *r : paramRows)
    obj->setProperty(r->getName(), r->getValue());
  return var(obj.release());
}

void VoiceEditorDialog::rebuildEnvelopeUI(const var &envVar) {
  envRows.clear();
  envContainer.removeAllChildren();
  envContainer.setSize(300, 0);

  String type = "None";
  if (auto *obj = envVar.getDynamicObject()) {
    type = obj->getProperty("type").toString();
    if (auto *p = obj->getProperty("params").getDynamicObject()) {
      for (const auto &prop : p->getProperties()) {
        auto *row =
            new ParameterRow(prop.name.toString(), prop.value.toString());
        envRows.add(row);
        envContainer.addAndMakeVisible(row);
      }
    }
  }
  envTypeCombo.setSelectedItemIndex(type == "linear_fade" ? 1 : 0);
  layoutEnvRows();
}

void VoiceEditorDialog::layoutEnvRows() {
  int y = 0;
  for (auto *r : envRows) {
    r->setBounds(0, y, 280, 24);
    y += 26;
  }
  envContainer.setSize(300, y);
  envViewport.setViewPosition(0, 0);
}

var VoiceEditorDialog::collectEnvelopeVar() {
  if (envTypeCombo.getSelectedId() == 1) // "None"
    return var();

  String type = envTypeCombo.getSelectedId() == 2 ? "linear_fade" : "None";
  auto envObj = std::make_unique<DynamicObject>();
  envObj->setProperty("type", type);
  auto params = std::make_unique<DynamicObject>();
  for (auto *r : envRows)
    params->setProperty(r->getName(), r->getValue());
  envObj->setProperty("params", var(params.release()));
  return var(envObj.release());
}

void VoiceEditorDialog::populateReferenceCombos() {
  refStepCombo.clear();
  refVoiceCombo.clear();
  for (size_t i = 0; i < referenceSteps.size(); ++i)
    refStepCombo.addItem(String("Step ") + String(i + 1), (int)i + 1);

  if (referenceSteps.empty())
    return;

  refStepCombo.onChange = [this] { updateVoiceCombo(); };
  refVoiceCombo.onChange = [this] { updateReferenceDetails(); };
  refStepCombo.setSelectedItemIndex(0);
}

void VoiceEditorDialog::updateVoiceCombo() {
  refVoiceCombo.clear();
  int stepIdx = refStepCombo.getSelectedItemIndex();
  if (stepIdx >= 0 && (size_t)stepIdx < referenceSteps.size()) {
    const auto &voices = referenceSteps.at(stepIdx);
    for (size_t i = 0; i < voices.size(); ++i)
      refVoiceCombo.addItem(String("Voice ") + String(i + 1), (int)i + 1);
    if (!voices.empty())
      refVoiceCombo.setSelectedItemIndex(0);
  }
  updateReferenceDetails();
}

void VoiceEditorDialog::updateReferenceDetails() {
  int stepIdx = refStepCombo.getSelectedItemIndex();
  int vIdx = refVoiceCombo.getSelectedItemIndex();
  if (stepIdx >= 0 && vIdx >= 0 && (size_t)stepIdx < referenceSteps.size() &&
      (size_t)vIdx < referenceSteps.at(stepIdx).size()) {
    const auto &v = referenceSteps.at(stepIdx).at(vIdx);
    String text;
    text << "Function: " << v.synthFunction << "\n";
    text << "Transition: " << (v.isTransition ? "yes" : "no") << "\n";
    if (auto *obj = v.params.getDynamicObject()) {
      text << "Params:\n";
      for (const auto &p : obj->getProperties())
        text << "  " << p.name.toString() << ": " << p.value.toString() << "\n";
    }
    refDetails.setText(text);
  } else {
    refDetails.clear();
  }
}

//==============================================================================
// Helper function implementation
//==============================================================================
VoiceEditorDialog::VoiceData showVoiceEditor(
    const StringArray &synthNames, const VoiceEditorDialog::VoiceData *existing,
    const std::vector<std::vector<VoiceEditorDialog::VoiceData>> *refSteps,
    bool *success) {
  VoiceEditorDialog dialog(synthNames, existing, refSteps);
  if (dialog.runModalLoop() && success)
    *success = dialog.wasAccepted();
  else if (success)
    *success = false;

  return dialog.getVoiceData();
}

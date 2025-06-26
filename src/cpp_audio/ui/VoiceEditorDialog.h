
#pragma once

#include <juce_data_structures/juce_data_structures.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

// Forward declaration
class VoiceEditorDialog;

//==============================================================================
// Helper class for a simple UI row with a label and a text editor.
//==============================================================================
class ParameterRow : public juce::Component {
public:
  ParameterRow(const juce::String &name, const juce::String &value);
  void resized() override;

  juce::String getName() const;
  juce::String getValue() const;
  void setValue(const juce::String &v);

private:
  juce::Label nameLabel;
  juce::TextEditor valueEditor;
};

//==============================================================================
// Dialog for editing a Voice entry.
//==============================================================================
class VoiceEditorDialog : public juce::DialogWindow,
                          private juce::Button::Listener {
public:
  struct VoiceData {
    juce::String synthFunction;
    bool isTransition = false;
    juce::var params;
    juce::var volumeEnvelope;
    juce::String description;
  };

  VoiceEditorDialog(
      const juce::StringArray &synthNames, const VoiceData *existing = nullptr,
      const std::vector<std::vector<VoiceData>> *refSteps = nullptr);
  ~VoiceEditorDialog() override;

  bool wasAccepted() const;
  VoiceData getVoiceData() const;

  void closeButtonPressed() override;
  void resized() override;

private:
  void buttonClicked(juce::Button *b) override;

  void populateFromData(const VoiceData &d);
  bool collectData();
  void rebuildParamUI(const juce::var &paramsVar);
  void layoutParamRows();
  juce::var collectParamsVar();
  void rebuildEnvelopeUI(const juce::var &envVar = juce::var());
  void layoutEnvRows();
  juce::var collectEnvelopeVar();
  void populateReferenceCombos();
  void updateVoiceCombo();
  void updateReferenceDetails();

  juce::Label funcLabel;
  juce::ComboBox funcCombo;
  juce::ToggleButton transitionToggle;

  juce::Label paramsLabel;
  juce::TextButton addParamButton;
  juce::Viewport paramsViewport;
  juce::Component paramsContainer;
  juce::OwnedArray<ParameterRow> paramRows;

  juce::Label envLabel;
  juce::ComboBox envTypeCombo;
  juce::Viewport envViewport;
  juce::Component envContainer;
  juce::OwnedArray<ParameterRow> envRows;

  juce::ComboBox refStepCombo;
  juce::ComboBox refVoiceCombo;
  juce::TextEditor refDetails;

  std::vector<std::vector<VoiceData>> referenceSteps;

  juce::Label descLabel;
  juce::TextEditor descEditor;

  juce::TextButton okButton, cancelButton;

  VoiceData data;
  bool accepted = false;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoiceEditorDialog)
};

//==============================================================================
// Helper function declaration to show the dialog modally.
//==============================================================================
VoiceEditorDialog::VoiceData showVoiceEditor(
    const juce::StringArray &synthNames,
    const VoiceEditorDialog::VoiceData *existing,
    const std::vector<std::vector<VoiceEditorDialog::VoiceData>> *refSteps,
    bool *success);

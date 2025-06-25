#include "DefaultVoiceDialog.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_events/juce_events.h>
#include "../VarUtils.h"

using namespace juce;

//==============================================================================
DefaultVoiceDialog::DefaultVoiceDialog(Preferences& prefs)
    : DialogWindow("Configure Default Voice", Colours::lightgrey, true),
      preferences(prefs)
{
    setUsingNativeTitleBar(true);
    setResizable(true, false);

    addAndMakeVisible(&synthLabel);
    synthLabel.setText("Synth Function", dontSendNotification);

    addAndMakeVisible(&synthEditor);

    addAndMakeVisible(&transitionToggle);
    transitionToggle.setButtonText("Is Transition");

    addAndMakeVisible(&paramsLabel);
    paramsLabel.setText("Parameters (JSON)", dontSendNotification);
    addAndMakeVisible(&paramsEditor);
    paramsEditor.setMultiLine(true);
    paramsEditor.setReturnKeyStartsNewLine(true);

    addAndMakeVisible(&envelopeLabel);
    envelopeLabel.setText("Volume Envelope (JSON)", dontSendNotification);
    addAndMakeVisible(&envelopeEditor);
    envelopeEditor.setMultiLine(true);
    envelopeEditor.setReturnKeyStartsNewLine(true);

    addAndMakeVisible(&saveButton);
    saveButton.setButtonText("Save Defaults");
    saveButton.addListener(this);

    addAndMakeVisible(&cancelButton);
    cancelButton.setButtonText("Cancel");
    cancelButton.addListener(this);

    // Populate fields from preferences
    if (auto* obj = preferences.defaultVoice.getDynamicObject())
    {
        synthEditor.setText(obj->getProperty("synth_function_name").toString());
        transitionToggle.setToggleState(getPropertyWithDefault(obj, "is_transition", false), dontSendNotification);
        if (auto* p = obj->getProperty("params").getDynamicObject())
            paramsEditor.setText(JSON::toString(var(p)));
        if (auto* e = obj->getProperty("volume_envelope").getDynamicObject())
            envelopeEditor.setText(JSON::toString(var(e)));
    }

    setSize(400, 300);
}

DefaultVoiceDialog::~DefaultVoiceDialog()
{
    saveButton.removeListener(this);
    cancelButton.removeListener(this);
}

void DefaultVoiceDialog::closeButtonPressed()
{
    setVisible(false);
}

void DefaultVoiceDialog::buttonClicked(Button* b)
{
    if (b == &saveButton)
    {
        saveVoice();
    }
    else if (b == &cancelButton)
    {
        closeButtonPressed();
    }
}

void DefaultVoiceDialog::resized()
{
    auto area = getLocalBounds().reduced(10);
    synthLabel.setBounds(area.removeFromTop(20));
    synthEditor.setBounds(area.removeFromTop(24));
    area.removeFromTop(6);
    transitionToggle.setBounds(area.removeFromTop(24));
    area.removeFromTop(6);
    paramsLabel.setBounds(area.removeFromTop(20));
    paramsEditor.setBounds(area.removeFromTop(80));
    area.removeFromTop(6);
    envelopeLabel.setBounds(area.removeFromTop(20));
    envelopeEditor.setBounds(area.removeFromTop(80));
    area.removeFromTop(6);
    auto buttons = area.removeFromTop(30);
    saveButton.setBounds(buttons.removeFromLeft(120));
    cancelButton.setBounds(buttons.removeFromLeft(120));
}

void DefaultVoiceDialog::saveVoice()
{
    std::unique_ptr<DynamicObject> obj(new DynamicObject());
    obj->setProperty("synth_function_name", synthEditor.getText());
    obj->setProperty("is_transition", transitionToggle.getToggleState());
    if (auto paramsVar = JSON::parse(paramsEditor.getText()); !paramsVar.isVoid())
        obj->setProperty("params", paramsVar);
    if (auto envVar = JSON::parse(envelopeEditor.getText()); !envVar.isVoid())
        obj->setProperty("volume_envelope", envVar);

    preferences.defaultVoice = var(obj.release());
    closeButtonPressed();
}

var DefaultVoiceDialog::getDefaultVoice() const
{
    return preferences.defaultVoice;
}

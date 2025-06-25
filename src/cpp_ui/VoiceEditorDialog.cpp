#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>

using namespace juce;

//==============================================================================
// Basic dialog for editing a Voice entry.  This is a minimal C++ adaptation of
// the much more feature rich Python version.  Parameters and envelope data are
// edited as JSON strings.
//==============================================================================

class VoiceEditorDialog  : public DialogWindow,
                           private Button::Listener
{
public:
    struct VoiceData
    {
        String synthFunction;
        bool   isTransition = false;
        var     params;          // stored as DynamicObject / JSON
        var     volumeEnvelope;  // stored as DynamicObject / JSON
        String  description;
    };

    VoiceEditorDialog(const StringArray& synthNames,
                      const VoiceData* existing = nullptr)
        : DialogWindow("Edit Voice", Colours::lightgrey, true)
    {
        setUsingNativeTitleBar(true);
        setResizable(true, false);

        addAndMakeVisible(funcLabel);
        funcLabel.setText("Synth Function", dontSendNotification);

        addAndMakeVisible(funcCombo);
        for (int i = 0; i < synthNames.size(); ++i)
            funcCombo.addItem(synthNames[i], i + 1);

        addAndMakeVisible(transitionToggle);
        transitionToggle.setButtonText("Is Transition");

        addAndMakeVisible(paramsLabel);
        paramsLabel.setText("Parameters (JSON)", dontSendNotification);
        addAndMakeVisible(paramsEditor);
        paramsEditor.setMultiLine(true);
        paramsEditor.setReturnKeyStartsNewLine(true);

        addAndMakeVisible(envLabel);
        envLabel.setText("Volume Envelope (JSON)", dontSendNotification);
        addAndMakeVisible(envEditor);
        envEditor.setMultiLine(true);
        envEditor.setReturnKeyStartsNewLine(true);

        addAndMakeVisible(descLabel);
        descLabel.setText("Description", dontSendNotification);
        addAndMakeVisible(descEditor);
        descEditor.setMultiLine(true);
        descEditor.setReturnKeyStartsNewLine(true);

        addAndMakeVisible(okButton);
        okButton.setButtonText("OK");
        okButton.addListener(this);

        addAndMakeVisible(cancelButton);
        cancelButton.setButtonText("Cancel");
        cancelButton.addListener(this);

        if (existing)
            populateFromData(*existing);

        setSize(500, 500);
    }

    ~VoiceEditorDialog() override
    {
        okButton.removeListener(this);
        cancelButton.removeListener(this);
    }

    bool wasAccepted() const { return accepted; }

    VoiceData getVoiceData() const { return data; }

    //==============================================================================
    void closeButtonPressed() override
    {
        exitModalState(0);
    }

    void buttonClicked(Button* b) override
    {
        if (b == &okButton)
        {
            if (collectData())
                exitModalState(1);
        }
        else if (b == &cancelButton)
        {
            closeButtonPressed();
        }
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(10);
        const int labelH = 20;
        const int editorH = 80;

        funcLabel.setBounds(area.removeFromTop(labelH));
        funcCombo.setBounds(area.removeFromTop(24));
        area.removeFromTop(6);
        transitionToggle.setBounds(area.removeFromTop(24));
        area.removeFromTop(6);

        paramsLabel.setBounds(area.removeFromTop(labelH));
        paramsEditor.setBounds(area.removeFromTop(editorH));
        area.removeFromTop(6);

        envLabel.setBounds(area.removeFromTop(labelH));
        envEditor.setBounds(area.removeFromTop(editorH));
        area.removeFromTop(6);

        descLabel.setBounds(area.removeFromTop(labelH));
        descEditor.setBounds(area.removeFromTop(editorH));
        area.removeFromTop(6);

        auto buttons = area.removeFromBottom(30);
        okButton.setBounds(buttons.removeFromRight(80));
        cancelButton.setBounds(buttons.removeFromRight(80));
    }

private:
    Label funcLabel;
    ComboBox funcCombo;
    ToggleButton transitionToggle;

    Label paramsLabel;
    TextEditor paramsEditor;

    Label envLabel;
    TextEditor envEditor;

    Label descLabel;
    TextEditor descEditor;

    TextButton okButton, cancelButton;

    VoiceData data;
    bool accepted = false;

    void populateFromData(const VoiceData& d)
    {
        if (funcCombo.containsItem(d.synthFunction, 1))
            funcCombo.setText(d.synthFunction, dontSendNotification);
        else if (funcCombo.getNumItems() > 0)
            funcCombo.setSelectedItemIndex(0);

        transitionToggle.setToggleState(d.isTransition, dontSendNotification);
        if (! d.params.isVoid())
            paramsEditor.setText(JSON::toString(d.params));
        if (! d.volumeEnvelope.isVoid())
            envEditor.setText(JSON::toString(d.volumeEnvelope));
        descEditor.setText(d.description);
    }

    bool collectData()
    {
        data.synthFunction = funcCombo.getText();
        data.isTransition = transitionToggle.getToggleState();
        data.description = descEditor.getText();

        auto paramsVar = JSON::parse(paramsEditor.getText());
        if (! paramsVar.isVoid() && ! paramsVar.isObject())
        {
            AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
                                             "Invalid Input",
                                             "Parameters must be a JSON object.");
            return false;
        }
        data.params = paramsVar;

        auto envVar = JSON::parse(envEditor.getText());
        if (! envVar.isVoid() && ! envVar.isObject())
        {
            AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
                                             "Invalid Input",
                                             "Volume envelope must be a JSON object.");
            return false;
        }
        data.volumeEnvelope = envVar;

        accepted = true;
        return true;
    }
};

//==============================================================================
// Helper function to show the dialog modally and return edited voice data.
//==============================================================================

VoiceEditorDialog::VoiceData showVoiceEditor(const StringArray& synthNames,
                                             const VoiceEditorDialog::VoiceData* existing,
                                             bool* success)
{
    VoiceEditorDialog dialog(synthNames, existing);
    DialogWindow::LaunchOptions opts;
    opts.content.setOwned(&dialog);
    opts.dialogTitle = "Voice Editor";
    opts.dialogBackgroundColour = Colours::lightgrey;
    opts.escapeKeyTriggersCloseButton = true;
    opts.useNativeTitleBar = true;
    opts.resizable = true;
    int result = opts.runModal();
    if (success)
        *success = (result != 0 && dialog.wasAccepted());
    return dialog.getVoiceData();
}


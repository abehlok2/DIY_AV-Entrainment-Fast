#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include <vector>

using namespace juce;

// Simple UI row with a label and a text editor used for both synth parameters
// and envelope parameters.  Values are edited as strings and converted when
// collecting data.
class ParameterRow : public Component
{
public:
    ParameterRow(const String& name, const String& value)
    {
        addAndMakeVisible(&nameLabel);
        nameLabel.setText(name, dontSendNotification);
        addAndMakeVisible(&valueEditor);
        valueEditor.setText(value);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        nameLabel.setBounds(area.removeFromLeft(120));
        valueEditor.setBounds(area);
    }

    String getName() const { return nameLabel.getText(); }
    String getValue() const { return valueEditor.getText(); }
    void   setValue(const String& v) { valueEditor.setText(v); }

private:
    Label nameLabel;
    TextEditor valueEditor;
};

//==============================================================================
// Dialog for editing a Voice entry.  This started as a very small JSON based
// editor.  It now mirrors the much richer Python UI and exposes individual
// parameter fields, reference voice selection and envelope controls.
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
                      const VoiceData* existing = nullptr,
                      const std::vector<std::vector<VoiceData>>* refSteps = nullptr)
        : DialogWindow("Edit Voice", Colours::lightgrey, true),
          referenceSteps(refSteps ? *refSteps : std::vector<std::vector<VoiceData>>{})
    {
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

        setSize(500, 500);
    }

    ~VoiceEditorDialog() override
    {
        okButton.removeListener(this);
        cancelButton.removeListener(this);
        addParamButton.removeListener(this);
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
        else if (b == &addParamButton)
        {
            auto* row = new ParameterRow("param", "0");
            paramRows.add(row);
            paramsContainer.addAndMakeVisible(row);
            layoutParamRows();
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
        addParamButton.setBounds(area.removeFromTop(24));
        area.removeFromTop(4);
        paramsViewport.setBounds(area.removeFromTop(editorH));
        area.removeFromTop(6);

        envLabel.setBounds(area.removeFromTop(labelH));
        envTypeCombo.setBounds(area.removeFromTop(24));
        area.removeFromTop(4);
        envViewport.setBounds(area.removeFromTop(editorH));
        area.removeFromTop(6);

        refStepCombo.setBounds(area.removeFromTop(24));
        refVoiceCombo.setBounds(area.removeFromTop(24));
        refDetails.setBounds(area.removeFromTop(editorH));
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
    TextButton addParamButton;
    Viewport paramsViewport;
    Component paramsContainer;
    OwnedArray<ParameterRow> paramRows;

    Label envLabel;
    ComboBox envTypeCombo;
    Viewport envViewport;
    Component envContainer;
    OwnedArray<ParameterRow> envRows;

    ComboBox refStepCombo;
    ComboBox refVoiceCombo;
    TextEditor refDetails;

    std::vector<std::vector<VoiceData>> referenceSteps;

    Label descLabel;
    TextEditor descEditor;

    TextButton okButton, cancelButton;

    VoiceData data;
    bool accepted = false;

    void populateFromData(const VoiceData& d)
    {
        // JUCE's ComboBox no longer provides a containsItem method. Instead
        // we look for the item's ID using indexOfItemId and only update the
        // ComboBox text if the item exists.
        if (funcCombo.indexOfItemId(1) != -1)
            funcCombo.setText(d.synthFunction, dontSendNotification);
        else if (funcCombo.getNumItems() > 0)
            funcCombo.setSelectedItemIndex(0);

        transitionToggle.setToggleState(d.isTransition, dontSendNotification);
        if (! d.params.isVoid())
            rebuildParamUI(d.params);
        else
            rebuildParamUI(var());

        if (! d.volumeEnvelope.isVoid())
            rebuildEnvelopeUI(d.volumeEnvelope);
        else
            rebuildEnvelopeUI(var());
        descEditor.setText(d.description);
        populateReferenceCombos();
    }

    bool collectData()
    {
        data.synthFunction = funcCombo.getText();
        data.isTransition = transitionToggle.getToggleState();
        data.description = descEditor.getText();

        data.params = collectParamsVar();
        data.volumeEnvelope = collectEnvelopeVar();

        accepted = true;
        return true;
    }

    //==========================================================================
    void rebuildParamUI(const var& paramsVar)
    {
        for (auto* r : paramRows)
            delete r;
        paramRows.clear(true);

        paramsContainer.setSize(300, 0);

        if (auto* obj = paramsVar.getDynamicObject())
        {
            for (const auto& p : obj->getProperties())
            {
                auto* row = new ParameterRow(p.name.toString(), p.value.toString());
                paramRows.add(row);
                paramsContainer.addAndMakeVisible(row);
            }
        }

        layoutParamRows();
    }

    void layoutParamRows()
    {
        int y = 0;
        for (auto* r : paramRows)
        {
            r->setBounds(0, y, 280, 24);
            y += 26;
        }
        paramsContainer.setSize(300, y);
        paramsViewport.setViewPosition(0, 0);
    }

    var collectParamsVar()
    {
        std::unique_ptr<DynamicObject> obj(new DynamicObject());
        for (auto* r : paramRows)
            obj->setProperty(r->getName(), r->getValue());
        return var(obj.release());
    }

    //==========================================================================
    void rebuildEnvelopeUI(const var& envVar = var())
    {
        for (auto* r : envRows)
            delete r;
        envRows.clear(true);

        envContainer.setSize(300, 0);

        String type = "None";
        if (auto* obj = envVar.getDynamicObject())
        {
            type = obj->getProperty("type").toString();
            if (auto* p = obj->getProperty("params").getDynamicObject())
            {
                for (const auto& prop : p->getProperties())
                {
                    auto* row = new ParameterRow(prop.name.toString(), prop.value.toString());
                    envRows.add(row);
                    envContainer.addAndMakeVisible(row);
                }
            }
        }

        envTypeCombo.setSelectedItemIndex(type == "linear_fade" ? 1 : 0);

        layoutEnvRows();
    }

    void layoutEnvRows()
    {
        int y = 0;
        for (auto* r : envRows)
        {
            r->setBounds(0, y, 280, 24);
            y += 26;
        }
        envContainer.setSize(300, y);
        envViewport.setViewPosition(0, 0);
    }

    var collectEnvelopeVar()
    {
        if (envTypeCombo.getSelectedId() == 1)
            return var();

        String type = envTypeCombo.getSelectedId() == 2 ? "linear_fade" : "None";
        std::unique_ptr<DynamicObject> envObj(new DynamicObject());
        envObj->setProperty("type", type);
        std::unique_ptr<DynamicObject> params(new DynamicObject());
        for (auto* r : envRows)
            params->setProperty(r->getName(), r->getValue());
        envObj->setProperty("params", var(params.release()));
        return var(envObj.release());
    }

    void populateReferenceCombos()
    {
        refStepCombo.clear();
        refVoiceCombo.clear();
        for (size_t i = 0; i < referenceSteps.size(); ++i)
            refStepCombo.addItem(String("Step ") + String(i + 1), int(i + 1));

        if (referenceSteps.empty())
            return;

        refStepCombo.onChange = [this] { updateVoiceCombo(); };
        refVoiceCombo.onChange = [this] { updateReferenceDetails(); };
        refStepCombo.setSelectedItemIndex(0);
        updateVoiceCombo();
    }

    void updateVoiceCombo()
    {
        refVoiceCombo.clear();
        int stepIdx = refStepCombo.getSelectedItemIndex();
        if (stepIdx >= 0 && stepIdx < (int)referenceSteps.size())
        {
            const auto& voices = referenceSteps[(size_t)stepIdx];
            for (size_t i = 0; i < voices.size(); ++i)
                refVoiceCombo.addItem(String("Voice ") + String(i + 1), int(i + 1));
            if (!voices.empty())
                refVoiceCombo.setSelectedItemIndex(0);
        }
        updateReferenceDetails();
    }

    void updateReferenceDetails()
    {
        int stepIdx = refStepCombo.getSelectedItemIndex();
        int vIdx = refVoiceCombo.getSelectedItemIndex();
        if (stepIdx >= 0 && vIdx >= 0 &&
            stepIdx < (int)referenceSteps.size() &&
            vIdx < (int)referenceSteps[(size_t)stepIdx].size())
        {
            const auto& v = referenceSteps[(size_t)stepIdx][(size_t)vIdx];
            String text;
            text << "Function: " << v.synthFunction << "\n";
            text << "Transition: " << (v.isTransition ? "yes" : "no") << "\n";
            if (auto* obj = v.params.getDynamicObject())
            {
                text << "Params:\n";
                for (const auto& p : obj->getProperties())
                    text << "  " << p.name.toString() << ": " << p.value.toString() << "\n";
            }
            refDetails.setText(text);
        }
        else
        {
            refDetails.clear();
        }
    }
};

//==============================================================================
// Helper function to show the dialog modally and return edited voice data.
//==============================================================================

VoiceEditorDialog::VoiceData showVoiceEditor(const StringArray& synthNames,
                                             const VoiceEditorDialog::VoiceData* existing,
                                             const std::vector<std::vector<VoiceEditorDialog::VoiceData>>* refSteps,
                                             bool* success)
{
    VoiceEditorDialog dialog(synthNames, existing, refSteps);
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


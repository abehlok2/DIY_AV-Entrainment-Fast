#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include <vector>

class VoiceEditorComponent : public juce::Component,
                             private juce::Button::Listener
{
public:
    struct VoiceData
    {
        juce::String synthFunction;
        bool isTransition{false};
        juce::var params;
        juce::var volumeEnvelope;
        juce::String description;
    };

    VoiceEditorComponent(const juce::StringArray& synthNames,
                         const VoiceData* existing = nullptr,
                         const std::vector<std::vector<VoiceData>>* refSteps = nullptr);
    ~VoiceEditorComponent() override;

    void resized() override;
    VoiceData getVoiceData() const;

    std::function<void(const VoiceData&)> onSave;
    std::function<void()> onCancel;

private:
    class ParameterRow : public juce::Component
    {
    public:
        ParameterRow(const juce::String& name, const juce::String& value);
        void resized() override;
        juce::String getName() const;
        juce::String getValue() const;
        void setValue(const juce::String& v);
    private:
        juce::Label nameLabel;
        juce::TextEditor valueEditor;
    };

    void buttonClicked(juce::Button* b) override;
    void populateFromData(const VoiceData& d);
    bool collectData();
    void rebuildParamUI(const juce::var& paramsVar);
    void rebuildParamUIWithNames(const juce::var& paramsVar,
                                 const juce::StringArray& names);
    void layoutParamRows();
    juce::var collectParamsVar();
    void rebuildEnvelopeUI(const juce::var& envVar = juce::var());
    void layoutEnvRows();
    juce::var collectEnvelopeVar();
    void populateReferenceCombos();
    void updateVoiceCombo();
    void updateReferenceDetails();

    juce::Label funcLabel;
    juce::ComboBox funcCombo;
    juce::ToggleButton transitionToggle;

    juce::Label paramsLabel;
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
    bool hasReferences{false};

    juce::Label descLabel;
    juce::TextEditor descEditor;

    juce::TextButton okButton, cancelButton;

    VoiceData data;
    bool accepted{false};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VoiceEditorComponent)
};


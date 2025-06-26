#pragma once
#include <juce_core/juce_core.h>
#include <vector>

class VoiceEditorDialog
{
public:
    struct VoiceData;
};

struct VoiceEditorDialog::VoiceData
{
    juce::String synthFunction;
    bool isTransition = false;
    juce::var params;
    juce::var volumeEnvelope;
    juce::String description;
};

VoiceEditorDialog::VoiceData showVoiceEditor(const juce::StringArray& synthNames,
                                             const VoiceEditorDialog::VoiceData* existing = nullptr,
                                             const std::vector<std::vector<VoiceEditorDialog::VoiceData>>* refSteps = nullptr,
                                             bool* success = nullptr);

#pragma once

// VoiceEditorDialog.h
// -------------------
// Lightweight interface for launching the JUCE based voice editor.
// This header exposes only the VoiceData structure along with a
// helper function to open the modal dialog.  The dialog implementation
// lives in VoiceEditorDialog.cpp to keep dependencies minimal.

#include <juce_core/juce_core.h>
#include <vector>

struct VoiceEditorDialog
{
    struct VoiceData
    {
        juce::String synthFunction;
        bool isTransition = false;
        juce::var params;
        juce::var volumeEnvelope;
        juce::String description;
    };
};

VoiceEditorDialog::VoiceData showVoiceEditor(const juce::StringArray& synthNames,
                                             const VoiceEditorDialog::VoiceData* existing = nullptr,
                                             const std::vector<std::vector<VoiceEditorDialog::VoiceData>>* refSteps = nullptr,
                                             bool* success = nullptr);

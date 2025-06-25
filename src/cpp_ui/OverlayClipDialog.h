#pragma once
#include <juce_core/juce_core.h>

struct OverlayClipDialog
{
    struct ClipData
    {
        juce::String filePath;
        double start  { 0.0 };
        double duration { 0.0 };
        double amp    { 1.0 };
        double pan    { 0.0 };
        double fadeIn { 0.0 };
        double fadeOut{ 0.0 };
        juce::String description;
    };
};

OverlayClipDialog::ClipData showOverlayClipEditor(bool amplitudeInDb = false,
                                                  const OverlayClipDialog::ClipData* existing = nullptr,
                                                  bool* success = nullptr);

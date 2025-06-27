#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

class ToolsComponent : public juce::Component
{
public:
    ToolsComponent();

    std::function<void()> onOverlayClips;

    void resized() override;

private:
    juce::TextButton overlayClipsButton {"Overlay Clips..."};
};

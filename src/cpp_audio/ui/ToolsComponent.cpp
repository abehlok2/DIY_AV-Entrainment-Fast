#include "ToolsComponent.h"

ToolsComponent::ToolsComponent()
{
    addAndMakeVisible(overlayClipsButton);
    overlayClipsButton.onClick = [this]() { if (onOverlayClips) onOverlayClips(); };
}

void ToolsComponent::resized()
{
    overlayClipsButton.setBounds(getLocalBounds().reduced(4));
}

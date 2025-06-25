#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

class CollapsibleBox : public juce::Component,
                       private juce::Button::Listener
{
public:
    explicit CollapsibleBox(const juce::String& title = {});
    ~CollapsibleBox() override;

    void setContentComponent(std::unique_ptr<juce::Component> newContent);
    juce::Component* getContentComponent() { return content.get(); }

    void resized() override;

private:
    void buttonClicked(juce::Button* button) override;
    void updateToggleState();

    juce::String title;
    juce::ToggleButton toggleButton;
    std::unique_ptr<juce::Component> content;
};


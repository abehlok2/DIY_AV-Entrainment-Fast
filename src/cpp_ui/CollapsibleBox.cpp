#include "CollapsibleBox.h"

CollapsibleBox::CollapsibleBox(const juce::String& titleIn)
    : title(titleIn)
{
    toggleButton.setClickingTogglesState(true);
    toggleButton.setToggleState(true, juce::dontSendNotification);
    toggleButton.addListener(this);
    addAndMakeVisible(toggleButton);
    updateToggleState();
}

CollapsibleBox::~CollapsibleBox()
{
    toggleButton.removeListener(this);
}

void CollapsibleBox::setContentComponent(std::unique_ptr<juce::Component> newContent)
{
    if (content)
        removeChildComponent(content.get());

    content = std::move(newContent);
    if (content)
    {
        addAndMakeVisible(content.get());
        content->setVisible(toggleButton.getToggleState());
    }
    resized();
}

void CollapsibleBox::buttonClicked(juce::Button*)
{
    updateToggleState();
    if (content)
        content->setVisible(toggleButton.getToggleState());
    resized();
}

void CollapsibleBox::updateToggleState()
{
    const juce::String downArrow = juce::CharPointer_UTF8("\xE2\x96\xBC"); // ▼
    const juce::String rightArrow = juce::CharPointer_UTF8("\xE2\x96\xB6"); // ▶
    bool open = toggleButton.getToggleState();
    toggleButton.setButtonText((open ? downArrow : rightArrow) + " " + title);
}

void CollapsibleBox::resized()
{
    auto area = getLocalBounds();
    int btnHeight = 24;
    toggleButton.setBounds(area.removeFromTop(btnHeight));
    if (content && content->isVisible())
        content->setBounds(area);
}


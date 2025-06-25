// Minimal JUCE GUI application that demonstrates the C++ audio widgets.
#include <juce_gui_basics/juce_gui_basics.h>

#include "ui/GlobalSettingsComponent.h"
#include "ui/StepListPanel.h"

class MainComponent : public juce::Component
{
public:
    MainComponent()
    {
        addAndMakeVisible(settings);
        addAndMakeVisible(stepList);
        setSize (600, 400);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (8);
        settings.setBounds (area.removeFromTop (120));
        stepList.setBounds (area);
    }

private:
    GlobalSettingsComponent settings;
    StepListPanel stepList;
};

class MainWindow : public juce::DocumentWindow
{
public:
    explicit MainWindow(const juce::String& name)
        : DocumentWindow(name,
                          juce::Desktop::getInstance().getDefaultLookAndFeel()
                              .findColour(juce::ResizableWindow::backgroundColourId),
                          juce::DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setResizable(true, true);
        setContentOwned(new MainComponent(), true);
        centreWithSize(getWidth(), getHeight());
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
};

class AudioApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override       { return "DIY AV Audio"; }
    const juce::String getApplicationVersion() override    { return "1.0"; }

    void initialise (const juce::String&) override
    {
        mainWindow.reset(new MainWindow(getApplicationName()));
    }

    void shutdown() override { mainWindow = nullptr; }

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(AudioApplication)


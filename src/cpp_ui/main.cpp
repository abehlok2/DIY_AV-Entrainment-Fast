#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "NoiseGeneratorDialog.h"
#include "FrequencyTesterDialog.h"

using namespace juce;

// Forward declarations for UI components that will be implemented later.
class StepListPanel;
class StepConfigPanel;

class MainComponent : public Component,
                      private Button::Listener
{
public:
    MainComponent()
    {
        setSize (800, 600);

        deviceManager.initialise(0, 2, nullptr, true);

        addAndMakeVisible(noiseButton);
        noiseButton.setButtonText("Noise Generator");
        noiseButton.addListener(this);

        addAndMakeVisible(freqButton);
        freqButton.setButtonText("Frequency Tester");
        freqButton.addListener(this);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(10);
        noiseButton.setBounds(area.removeFromTop(30));
        area.removeFromTop(10);
        freqButton.setBounds(area.removeFromTop(30));
    }

private:
    TextButton noiseButton, freqButton;
    AudioDeviceManager deviceManager;

    void buttonClicked(Button* b)
    {
        if (b == &noiseButton)
        {
            DialogWindow::LaunchOptions opts;
            opts.content = createNoiseGeneratorDialog();
            opts.dialogTitle = "Noise Generator";
            opts.dialogBackgroundColour = Colours::lightgrey;
            opts.escapeKeyTriggersCloseButton = true;
            opts.useNativeTitleBar = true;
            opts.resizable = true;
            opts.runModal();
        }
        else if (b == &freqButton)
        {
            DialogWindow::LaunchOptions opts;
            opts.content = createFrequencyTesterDialog(deviceManager);
            opts.dialogTitle = "Frequency Tester";
            opts.dialogBackgroundColour = Colours::lightgrey;
            opts.escapeKeyTriggersCloseButton = true;
            opts.useNativeTitleBar = true;
            opts.resizable = true;
            opts.runModal();
        }
    }
};

class MainWindow : public DocumentWindow
{
public:
    MainWindow() : DocumentWindow ("DIY AV Editor",
                                   Colours::lightgrey,
                                   DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar (true);
        setContentOwned (new MainComponent(), true);
        centreWithSize (getWidth(), getHeight());
        setVisible (true);
    }

    void closeButtonPressed() override
    {
        JUCEApplication::getInstance()->systemRequestedQuit();
    }
};

class DiyAvApplication : public JUCEApplication
{
public:
    DiyAvApplication() = default;

    const String getApplicationName() override       { return "DIY AV UI CPP"; }
    const String getApplicationVersion() override    { return "0.1"; }
    bool moreThanOneInstanceAllowed() override       { return true; }

    void initialise (const String&) override
    {
        mainWindow.reset (new MainWindow());
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION (DiyAvApplication)


#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

// include dialog implementations that currently only exist as .cpp files
#include "SubliminalDialog.cpp"

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
        addAndMakeVisible(subliminalButton);
        subliminalButton.addListener(this);
        subliminalButton.setButtonText("Add Subliminal Voice");
    }

    void resized() override
    {
        subliminalButton.setBounds(10, 10, 160, 30);
    }

private:
    TextButton subliminalButton;

    void buttonClicked(Button* b) override
    {
        if (b == &subliminalButton)
        {
            SubliminalDialog dlg;
            DialogWindow::LaunchOptions opts;
            opts.content.setOwned(&dlg);
            opts.dialogTitle = "Add Subliminal Voice";
            opts.componentToCentreAround = this;
            opts.useNativeTitleBar = true;
            opts.resizable = false;
            int result = opts.runModal();

            if (result != 0 && dlg.wasAccepted())
            {
                auto voice = dlg.getVoice();
                DBG("Subliminal voice added: " << voice.description);
            }
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


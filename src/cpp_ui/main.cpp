#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

using namespace juce;

// Forward declarations for UI components that will be implemented later.
class StepListPanel;
class StepConfigPanel;

class MainComponent : public Component
{
public:
    MainComponent()
    {
        setSize (800, 600);
        // TODO: create and add child components once implemented
    }

    void resized() override
    {
        // TODO: layout child components
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


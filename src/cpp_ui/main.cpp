#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "NoiseGeneratorDialog.h"
#include "FrequencyTesterDialog.h"
#include "../cpp_audio/Track.h"
#include "StepPreviewComponent.h"

#include <memory>

#include "DefaultVoiceDialog.h"

// Forward declaration of theme helper implemented in Themes.cpp
extern void applyTheme (juce::LookAndFeel_V4&, const juce::String&);

// include dialog implementations that currently only exist as .cpp files
#include "SubliminalDialog.cpp"

using namespace juce;

// Forward declarations for UI components that will be implemented later.
class StepListPanel;
class StepConfigPanel;
#include "OverlayClipPanel.h"


namespace
{
    enum MenuIDs
    {
        menuNew = 1,
        menuOpen,
        menuSave,
        menuPreferences,
        menuDefaults,
        themeDark,
        themeGreen,
        themeBlue,
        themeMaterial
    };
}

class MainComponent : public Component,
                      private MenuBarModel

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

        menuBar.reset (new MenuBarComponent (this));
        addAndMakeVisible (menuBar.get());

        setLookAndFeel (&lookAndFeel);
        applyTheme (lookAndFeel, currentTheme);

        // TODO: create and add child components once implemented

        addAndMakeVisible(overlayPanel);
        addAndMakeVisible(stepPreview);
        addAndMakeVisible(subliminalButton);
        subliminalButton.addListener(this);
        subliminalButton.setButtonText("Add Subliminal Voice");

        newTrack();

    }

    ~MainComponent() override
    {
        menuBar->setModel (nullptr);
        setLookAndFeel (nullptr);

    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(10);
        noiseButton.setBounds(area.removeFromTop(30));
        area.removeFromTop(10);
        freqButton.setBounds(area.removeFromTop(30));

        auto previewArea = area.removeFromBottom(110);
        stepPreview.setBounds(previewArea.reduced(0, 4));

        overlayPanel.setBounds(area.reduced(0, 4));
        subliminalButton.setBounds(10, 10, 160, 30);
    }

private:
    TextButton noiseButton, freqButton;
    StepPreviewComponent stepPreview {deviceManager};
    AudioDeviceManager deviceManager;
    Track currentTrack;
    juce::File currentFile;

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

        auto area = getLocalBounds();
        menuBar->setBounds (area.removeFromTop (24));
        // TODO: layout child components with remaining area
    }

    Track createDefaultTrack()
    {
        Track t;
        t.settings.sampleRate = 44100.0;
        t.settings.crossfadeDuration = 1.0;
        t.settings.crossfadeCurve = "linear";
        return t;
    }

    void newTrack()
    {
        currentTrack = createDefaultTrack();
        currentFile = {};
        stepPreview.reset();
    }

    void openTrack()
    {
        FileChooser chooser("Open Track JSON", {}, "*.json");
        if (chooser.browseForFileToOpen())
        {
            currentFile = chooser.getResult();
            currentTrack = loadTrackFromJson(currentFile);
            AlertWindow::showMessageBoxAsync(AlertWindow::InfoIcon,
                                            "Open",
                                            "Loaded track from\n" + currentFile.getFullPathName());
            if (! currentTrack.steps.empty())
                stepPreview.loadStep(currentTrack.steps.front(), currentTrack.settings, 10.0);
            else
                stepPreview.reset();
        }
    }

    void saveTrack(bool saveAs)
    {
        if (saveAs || ! currentFile.existsAsFile())
        {
            FileChooser chooser("Save Track JSON", currentFile, "*.json");
            if (! chooser.browseForFileToSave(true))
                return;
            currentFile = chooser.getResult();
        }
        if (saveTrackToJson(currentTrack, currentFile))
            AlertWindow::showMessageBoxAsync(AlertWindow::InfoIcon,
                                            "Save",
                                            "Track saved to\n" + currentFile.getFullPathName());
        else
            AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon,
                                            "Save",
                                            "Failed to save track");
    }

    StringArray getMenuBarNames() override
    {
        return { "File" };
    }

    PopupMenu getMenuForIndex (int, const String& menuName) override
    {
        PopupMenu menu;
        if (menuName == "File")
        {
            menu.addItem (menuNew, "New");
            menu.addItem (menuOpen, "Open...");
            menu.addItem (menuSave, "Save");
            menu.addSeparator();
            menu.addItem (menuPreferences, "Preferences...");
            menu.addItem (menuDefaults, "Configure Defaults...");

            PopupMenu theme;
            theme.addItem (themeDark, "Dark", true, currentTheme == "Dark");
            theme.addItem (themeGreen, "Green", true, currentTheme == "Green");
            theme.addItem (themeBlue, "light-blue", true, currentTheme == "light-blue");
            theme.addItem (themeMaterial, "Material", true, currentTheme == "Material");
            menu.addSubMenu ("Theme", theme);
        }
        return menu;
    }

    void menuItemSelected (int menuItemID, int) override
    {
        switch (menuItemID)
        {
            case menuNew:
                newTrack();
                AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                                   "New", "Created new track");
                break;
            case menuOpen:
                openTrack();
                break;
            case menuSave:
                saveTrack(false);
                break;
            case menuPreferences:
                AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                                   "Preferences",
                                                   "Preferences dialog not available");
                break;
            case menuDefaults:
            {
                DefaultVoiceDialog dlg (prefs);
                dlg.runModalLoop();
                prefs.defaultVoice = dlg.getDefaultVoice();
                break;
            }
            case themeDark:     setTheme ("Dark"); break;
            case themeGreen:    setTheme ("Green"); break;
            case themeBlue:     setTheme ("light-blue"); break;
            case themeMaterial: setTheme ("Material"); break;
            default: break;
        }
    }

private:
    void setTheme (const String& name)
    {
        currentTheme = name;
        applyTheme (lookAndFeel, name);
        repaint();
    }

    std::unique_ptr<MenuBarComponent> menuBar;
    LookAndFeel_V4 lookAndFeel;
    Preferences prefs;
    String currentTheme { "Dark" };

    
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

private:
    OverlayClipPanel overlayPanel;

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


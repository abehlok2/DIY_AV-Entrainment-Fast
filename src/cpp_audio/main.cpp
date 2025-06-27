// Entry point for the JUCE based audio GUI.
// This file creates a minimal editor window that hosts
// the C++ audio widgets translated from the Python version.

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>

#include "ui/GlobalSettingsComponent.h"
#include "ui/StepListPanel.h"
#include "ui/OverlayClipPanel.h"
#include "ui/StepPreviewComponent.h"
#include "Track.h"
#include "ui/PreferencesDialog.h"
#include "ui/NoiseGeneratorDialog.h"
#include "ui/FrequencyTesterDialog.h"
#include "ui/Themes.h"
#include <vector>

class MainComponent : public juce::Component,
                      private juce::MenuBarModel
{
public:
    MainComponent(Preferences& p, juce::LookAndFeel_V4& lf)
        : prefs(p), lookAndFeel(lf), settings(deviceManager),
          preview(deviceManager), menuBar(this)
    {
        deviceManager.initialise(0, 2, nullptr, true);
        addAndMakeVisible(menuBar);
        addAndMakeVisible(settings);
        addAndMakeVisible(preview);
        addAndMakeVisible(stepList);
        addAndMakeVisible(editClipsButton);
        editClipsButton.onClick = [this] { openClipEditor(); };
        stepList.onStepSelected = [this](int index)
        {
            const auto& steps = stepList.getSteps();
            if (juce::isPositiveAndBelow(index, steps.size()))
            {
                Step step;
                step.durationSeconds = steps[index].duration;
                step.description = steps[index].description;
                for (const auto& vd : steps[index].voices)
                {
                    Voice v;
                    v.synthFunction = vd.synthFunction.toStdString();
                    if (auto* obj = vd.params.getDynamicObject())
                        v.params = obj->getProperties();
                    v.isTransition = vd.isTransition;
                    v.description = vd.description;
                    step.voices.push_back(std::move(v));
                }
                auto gsRaw = settings.getSettings();
                GlobalSettings gs;
                gs.sampleRate = gsRaw.sampleRate;
                gs.crossfadeDuration = gsRaw.crossfadeSeconds;
                gs.outputFilename = gsRaw.outputFile;
                gs.crossfadeCurve = "linear";
                double previewDur = step.durationSeconds < 180.0 ? step.durationSeconds : 60.0;
                preview.loadStep(step, gs, previewDur);
            }
            else
            {
                preview.reset();
            }
        };
        setSize (800, 600);
    }

    ~MainComponent() override
    {
        deviceManager.closeAudioDevice();
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (8);
        menuBar.setBounds(area.removeFromTop(24));
        settings.setBounds (area.removeFromTop (144));
        preview.setBounds(area.removeFromTop(80));
        editClipsButton.setBounds(area.removeFromTop(24));
        area.removeFromTop(4);
        stepList.setBounds (area);
    }

    juce::StringArray getMenuBarNames() override
    {
        return { "File", "Tools" };
    }

    juce::PopupMenu getMenuForIndex(int index, const juce::String&) override
    {
        juce::PopupMenu menu;
        if (index == 0)
        {
            menu.addItem(menuPreferences, "Preferences...");
        }
        else if (index == 1)
        {
            menu.addItem(menuNoiseGen, "Noise Generator...");
            menu.addItem(menuFreqTest, "Frequency Tester...");
            menu.addItem(menuOverlayClips, "Overlay Clips...");
        }
        return menu;
    }

    void menuItemSelected(int menuItemID, int) override
    {
        switch (menuItemID)
        {
            case menuPreferences:
                if (showPreferencesDialog(prefs))
                    applyTheme(lookAndFeel, prefs.theme);
                break;
            case menuNoiseGen:
                openNoiseGenerator();
                break;
            case menuFreqTest:
                openFrequencyTester();
                break;
            case menuOverlayClips:
                openClipEditor();
                break;
            default:
                break;
        }
    }

private:
    juce::AudioDeviceManager deviceManager;
    GlobalSettingsComponent settings;
    StepPreviewComponent preview;
    StepListPanel stepList;
    juce::TextButton editClipsButton {"Overlay Clips..."};
    juce::MenuBarComponent menuBar;
    Preferences& prefs;
    juce::LookAndFeel_V4& lookAndFeel;
    std::vector<OverlayClipPanel::ClipData> clips;

    enum MenuIds
    {
        menuPreferences = 1,
        menuNoiseGen,
        menuFreqTest,
        menuOverlayClips
    };

    void openClipEditor()
    {
        auto* panel = new OverlayClipPanel();
        panel->setClips(clips);
        panel->onClipsChanged = [this, panel]() { clips = panel->getClips(); };

        juce::DialogWindow::LaunchOptions opts;
        opts.content.setOwned(panel);
        opts.dialogTitle = "Overlay Clips";
        opts.dialogBackgroundColour = juce::Colours::lightgrey;
        opts.escapeKeyTriggersCloseButton = true;
        opts.useNativeTitleBar = true;
        opts.resizable = true;
        opts.runModal();
    }

    void openNoiseGenerator()
    {
        auto dialog = createNoiseGeneratorDialog();
        juce::DialogWindow::LaunchOptions opts;
        opts.content.setOwned(dialog.release());
        opts.dialogTitle = "Noise Generator";
        opts.dialogBackgroundColour = juce::Colours::lightgrey;
        opts.escapeKeyTriggersCloseButton = true;
        opts.useNativeTitleBar = true;
        opts.resizable = true;
        opts.runModal();
    }

    void openFrequencyTester()
    {
        auto dialog = createFrequencyTesterDialog(deviceManager);
        juce::DialogWindow::LaunchOptions opts;
        opts.content.setOwned(dialog.release());
        opts.dialogTitle = "Frequency Tester";
        opts.dialogBackgroundColour = juce::Colours::lightgrey;
        opts.escapeKeyTriggersCloseButton = true;
        opts.useNativeTitleBar = true;
        opts.resizable = true;
        opts.runModal();
    }
};

class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow(const juce::String& name, Preferences& p, juce::LookAndFeel_V4& lf)
        : DocumentWindow(name,
                          juce::Desktop::getInstance().getDefaultLookAndFeel()
                              .findColour(juce::ResizableWindow::backgroundColourId),
                          juce::DocumentWindow::allButtons),
          prefs(p), lookAndFeel(lf)
    {
        setUsingNativeTitleBar(true);
        setResizable(true, true);
        setContentOwned(new MainComponent(prefs, lookAndFeel), true);
        centreWithSize(getWidth(), getHeight());
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }

private:
    Preferences& prefs;
    juce::LookAndFeel_V4& lookAndFeel;
};

class AudioApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override       { return "DIY AV Audio"; }
    const juce::String getApplicationVersion() override    { return "1.0"; }

    void initialise (const juce::String&) override
    {
        juce::Desktop::getInstance().setDefaultLookAndFeel(&lookAndFeel);
        applyTheme(lookAndFeel, prefs.theme);
        mainWindow.reset(new MainWindow(getApplicationName(), prefs, lookAndFeel));
        showPreferencesDialog(prefs);
        applyTheme(lookAndFeel, prefs.theme);
    }

    void shutdown() override { mainWindow = nullptr; }

private:
    std::unique_ptr<MainWindow> mainWindow;
    juce::LookAndFeel_V4 lookAndFeel;
    Preferences prefs;
};

START_JUCE_APPLICATION(AudioApplication)


// Entry point for the JUCE based audio GUI.
// This file creates a minimal editor window that hosts
// the C++ audio widgets translated from the Python version.

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>

#include "ui/GlobalSettingsComponent.h"
#include "ui/StepListPanel.h"
#include "ui/OverlayClipPanel.h"
#include "ui/StepPreviewComponent.h"
#include "ui/CollapsibleBox.h"
#include "ui/ToolsComponent.h"
#include "Track.h"
#include "ui/PreferencesDialog.h"
#include "ui/Themes.h"
#include <vector>

class MainComponent : public juce::Component
{
public:
    MainComponent()
        : settingsBox("Global Settings"),
          toolsBox("Tools"),
          previewBox("Step Preview")
    {
        deviceManager.initialise(0, 2, nullptr, true);
        {
            auto ptr = std::make_unique<GlobalSettingsComponent>(deviceManager);
            settings = ptr.get();
            settingsBox.setContentComponent(std::move(ptr));
            addAndMakeVisible(settingsBox);
        }
        {
            auto ptr = std::make_unique<ToolsComponent>();
            tools = ptr.get();
            tools->onOverlayClips = [this] { openClipEditor(); };
            toolsBox.setContentComponent(std::move(ptr));
            addAndMakeVisible(toolsBox);
        }
        {
            auto ptr = std::make_unique<StepPreviewComponent>(deviceManager);
            preview = ptr.get();
            previewBox.setContentComponent(std::move(ptr));
            addAndMakeVisible(previewBox);
        }
        addAndMakeVisible(stepList);
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
                auto gsRaw = settings->getSettings();
                GlobalSettings gs;
                gs.sampleRate = gsRaw.sampleRate;
                gs.crossfadeDuration = gsRaw.crossfadeSeconds;
                gs.outputFilename = gsRaw.outputFile;
                gs.crossfadeCurve = "linear";
                double previewDur = step.durationSeconds < 180.0 ? step.durationSeconds : 60.0;
                preview->loadStep(step, gs, previewDur);
            }
            else
            {
                preview->reset();
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
        toolsBox.setBounds(area.removeFromTop(40));
        settingsBox.setBounds(area.removeFromTop(220));
        previewBox.setBounds(area.removeFromTop(100));
        area.removeFromTop(4);
        stepList.setBounds(area);
    }

private:
    juce::AudioDeviceManager deviceManager;
    CollapsibleBox settingsBox;
    CollapsibleBox toolsBox;
    CollapsibleBox previewBox;
    GlobalSettingsComponent* settings = nullptr;
    ToolsComponent* tools = nullptr;
    StepPreviewComponent* preview = nullptr;
    StepListPanel stepList;
    std::vector<OverlayClipPanel::ClipData> clips;

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
        juce::Desktop::getInstance().setDefaultLookAndFeel(&lookAndFeel);
        applyTheme(lookAndFeel, prefs.theme);
        mainWindow.reset(new MainWindow(getApplicationName()));
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


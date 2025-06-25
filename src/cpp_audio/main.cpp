// Entry point for the JUCE based audio GUI.
// This file creates a minimal editor window that hosts
// the C++ audio widgets translated from the Python version.

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>

#include "ui/GlobalSettingsComponent.h"
#include "ui/StepListPanel.h"
#include "ui/StepPreviewComponent.h"
#include "Track.h"

class MainComponent : public juce::Component
{
public:
    MainComponent()
        : preview(deviceManager)
    {
        deviceManager.initialise(0, 2, nullptr, true);
        addAndMakeVisible(settings);
        addAndMakeVisible(preview);
        addAndMakeVisible(stepList);
        stepList.onStepSelected = [this](int index)
        {
            const auto& steps = stepList.getSteps();
            if (juce::isPositiveAndBelow(index, steps.size()))
            {
                Step step;
                step.durationSeconds = steps[index].duration;
                step.description = steps[index].description;
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
        settings.setBounds (area.removeFromTop (120));
        preview.setBounds(area.removeFromTop(80));
        stepList.setBounds (area);
    }

private:
    juce::AudioDeviceManager deviceManager;
    GlobalSettingsComponent settings;
    StepPreviewComponent preview;
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


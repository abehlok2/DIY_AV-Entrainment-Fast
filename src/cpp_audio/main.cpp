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
#include "ui/Themes.h"
#include <vector>
#include <fstream>

class MainComponent : public juce::Component
{
public:
    MainComponent()
        : settings(deviceManager),
          preview(deviceManager)
    {
        deviceManager.initialise(0, 2, nullptr, true);
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
        newTrack();
        setSize (800, 600);
    }

    ~MainComponent() override
    {
        deviceManager.closeAudioDevice();
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (8);
        settings.setBounds (area.removeFromTop (144));
        preview.setBounds(area.removeFromTop(80));
        editClipsButton.setBounds(area.removeFromTop(24));
        area.removeFromTop(4);
        stepList.setBounds (area);
    }

private:
    juce::AudioDeviceManager deviceManager;
    GlobalSettingsComponent settings;
    StepPreviewComponent preview;
    StepListPanel stepList;
    juce::TextButton editClipsButton {"Overlay Clips..."};
    std::vector<OverlayClipPanel::ClipData> clips;
    juce::File currentFile;

    static juce::NamedValueSet varToNamedValueSet(const juce::var& v)
    {
        juce::NamedValueSet set;
        if (auto* obj = v.getDynamicObject())
        {
            for (const auto& p : obj->getProperties())
                set.set(p.name, p.value);
        }
        return set;
    }

    static juce::var namedValueSetToVar(const juce::NamedValueSet& set)
    {
        auto* obj = new juce::DynamicObject();
        for (const auto& p : set)
            obj->setProperty(p.name, p.value);
        return juce::var(obj);
    }

    Track collectTrack() const
    {
        Track t;
        auto gsRaw = settings.getSettings();
        t.settings.sampleRate = gsRaw.sampleRate;
        t.settings.crossfadeDuration = gsRaw.crossfadeSeconds;
        t.settings.outputFilename = gsRaw.outputFile;
        t.settings.crossfadeCurve = "linear";
        t.backgroundNoise.filePath = gsRaw.noiseFile;
        t.backgroundNoise.amp = gsRaw.noiseAmp;

        for (const auto& cd : clips)
        {
            Clip c;
            c.filePath = cd.filePath;
            c.start = cd.start;
            c.duration = cd.duration;
            c.amp = cd.amp;
            c.pan = cd.pan;
            c.fadeIn = cd.fadeIn;
            c.fadeOut = cd.fadeOut;
            c.description = cd.description;
            t.clips.push_back(std::move(c));
        }

        const auto& steps = stepList.getSteps();
        for (const auto& sd : steps)
        {
            Step s;
            s.durationSeconds = sd.duration;
            s.description = sd.description;
            for (const auto& vd : sd.voices)
            {
                Voice v;
                v.synthFunction = vd.synthFunction.toStdString();
                v.isTransition = vd.isTransition;
                v.params = varToNamedValueSet(vd.params);
                v.description = vd.description;
                s.voices.push_back(std::move(v));
            }
            t.steps.push_back(std::move(s));
        }
        return t;
    }

    void applyTrack(const Track& t)
    {
        GlobalSettingsComponent::Settings gs;
        gs.sampleRate = t.settings.sampleRate;
        gs.crossfadeSeconds = t.settings.crossfadeDuration;
        gs.outputFile = t.settings.outputFilename;
        gs.noiseFile = t.backgroundNoise.filePath;
        gs.noiseAmp = t.backgroundNoise.amp;
        settings.setSettings(gs);

        clips.clear();
        for (const auto& c : t.clips)
        {
            OverlayClipPanel::ClipData cd;
            cd.filePath = c.filePath;
            cd.start = c.start;
            cd.duration = c.duration;
            cd.amp = c.amp;
            cd.pan = c.pan;
            cd.fadeIn = c.fadeIn;
            cd.fadeOut = c.fadeOut;
            cd.description = c.description;
            clips.push_back(std::move(cd));
        }

        juce::Array<StepListPanel::StepData> newSteps;
        for (const auto& s : t.steps)
        {
            StepListPanel::StepData sd;
            sd.duration = s.durationSeconds;
            sd.description = s.description;
            for (const auto& v : s.voices)
            {
                VoiceEditorDialog::VoiceData vd;
                vd.synthFunction = v.synthFunction;
                vd.isTransition = v.isTransition;
                vd.params = namedValueSetToVar(v.params);
                vd.description = v.description;
                sd.voices.add(vd);
            }
            newSteps.add(sd);
        }
        stepList.setSteps(newSteps);
        preview.reset();
    }

    void newTrack()
    {
        currentFile = juce::File();
        clips.clear();
        GlobalSettingsComponent::Settings gs;
        gs.sampleRate = 44100.0;
        gs.crossfadeSeconds = 1.0;
        gs.outputFile = "my_track.wav";
        gs.noiseFile = "";
        gs.noiseAmp = 0.0;
        settings.setSettings(gs);
        stepList.setSteps({});
        preview.reset();
        if (auto* w = findParentComponentOfClass<juce::DocumentWindow>())
            w->setName("DIY AV Audio - New File");
    }

    void loadTrack()
    {
        juce::FileChooser fc("Load Track", {}, "*.json");
        if (fc.browseForFileToOpen())
        {
            auto file = fc.getResult();
            auto t = loadTrackFromJson(file);
            applyTrack(t);
            currentFile = file;
            if (auto* w = findParentComponentOfClass<juce::DocumentWindow>())
                w->setName("DIY AV Audio - " + file.getFileName());
        }
    }

    void saveTrack()
    {
        juce::File target = currentFile;
        if (!target.existsAsFile())
        {
            juce::FileChooser fc("Save Track", {}, "*.json");
            if (!fc.browseForFileToSave(true))
                return;
            target = fc.getResult();
        }
        auto t = collectTrack();
        if (saveTrackToJson(t, target))
        {
            currentFile = target;
            if (auto* w = findParentComponentOfClass<juce::DocumentWindow>())
                w->setName("DIY AV Audio - " + target.getFileName());
        }
    }

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

class MainWindow : public juce::DocumentWindow,
                   public juce::MenuBarModel
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
        setMenuBar(this);
        centreWithSize(getWidth(), getHeight());
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }

    juce::StringArray getMenuBarNames() override
    {
        return {"File"};
    }

    juce::PopupMenu getMenuForIndex(int, const juce::String& name) override
    {
        juce::PopupMenu m;
        if (name == "File")
        {
            m.addItem(1, "New");
            m.addItem(2, "Load...");
            m.addItem(3, "Save");
        }
        return m;
    }

    void menuItemSelected(int menuItemID, int) override
    {
        if (auto* mc = dynamic_cast<MainComponent*>(getContentComponent()))
        {
            if (menuItemID == 1) mc->newTrack();
            else if (menuItemID == 2) mc->loadTrack();
            else if (menuItemID == 3) mc->saveTrack();
        }
    }

    ~MainWindow() override { setMenuBar(nullptr); }
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


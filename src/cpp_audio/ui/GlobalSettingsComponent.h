
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory> // For std::unique_ptr

// Forward declaration to avoid including the full dialog header here.
class NoiseGeneratorDialog;

class GlobalSettingsComponent : public juce::Component,
                                private juce::Button::Listener {
public:
  struct Settings {
    double sampleRate = 44100.0;
    double crossfadeSeconds = 1.0;
    juce::String outputFile;
    juce::String noiseFile;
    double noiseAmp = 0.0;
  };

  GlobalSettingsComponent();
  ~GlobalSettingsComponent() override;

  Settings getSettings() const;
  void setSettings(const Settings &s);

private:
  void resized() override;
  void buttonClicked(juce::Button *b) override;

  // A helper function to create the dialog.
  std::unique_ptr<NoiseGeneratorDialog> createNoiseGeneratorDialog();

  juce::Label srLabel, cfLabel, outFileLabel, noiseFileLabel, noiseAmpLabel;
  juce::TextEditor srEdit, cfEdit, outFileEdit, noiseFileEdit, noiseAmpEdit;
  juce::TextButton browseOutButton, browseNoiseButton, noiseGenButton;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GlobalSettingsComponent)
};

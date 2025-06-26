#pragma once

#include "../models/NoiseParams.h" // Assuming NoiseParams is separated or defined here
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_extra/juce_gui_extra.h>

// Forward-declare the factory function
std::unique_ptr<juce::Component> createNoiseGeneratorDialog();

//==============================================================================
class NoiseGeneratorDialog : public juce::Component,
                             private juce::Button::Listener,
                             private juce::Slider::Listener {
public:
  NoiseGeneratorDialog();
  ~NoiseGeneratorDialog() override;

  void resized() override;

private:
  // This nested struct holds the controls for a single sweep.
  // It's private because only the NoiseGeneratorDialog needs to know about it.
  struct SweepControls {
    juce::Slider startMin, endMin, startMax, endMax;
    juce::Slider startQ, endQ, startCasc, endCasc;
  };

  // Listener Callbacks
  void buttonClicked(juce::Button *b) override;
  void sliderValueChanged(juce::Slider *s) override;

  // UI update and data handling methods
  NoiseParams getParams() const;
  void setParams(const NoiseParams &p);
  void updateSweepVisibility(int count);

  // UI Components
  juce::TextEditor fileEdit;
  juce::TextButton fileBrowse{"Browse"};
  juce::Slider durationSlider{juce::Slider::LinearHorizontal,
                              juce::Slider::TextBoxRight};
  juce::Slider sampleRateSlider{juce::Slider::LinearHorizontal,
                                juce::Slider::TextBoxRight};
  juce::ComboBox noiseType;
  juce::ToggleButton transitionToggle;
  juce::ComboBox lfoWaveform;
  juce::Slider lfoStart{juce::Slider::LinearHorizontal,
                        juce::Slider::TextBoxRight};
  juce::Slider lfoEnd{juce::Slider::LinearHorizontal,
                      juce::Slider::TextBoxRight};
  juce::Slider numSweeps{juce::Slider::LinearHorizontal,
                         juce::Slider::TextBoxRight};

  // OwnedArray holds pointers and manages their memory. This is necessary
  // because the SweepControls struct contains non-copyable juce::Sliders.
  juce::OwnedArray<SweepControls> sweepControls;

  juce::Slider lfoPhaseStart{juce::Slider::LinearHorizontal,
                             juce::Slider::TextBoxRight};
  juce::Slider lfoPhaseEnd{juce::Slider::LinearHorizontal,
                           juce::Slider::TextBoxRight};
  juce::Slider intraPhaseStart{juce::Slider::LinearHorizontal,
                               juce::Slider::TextBoxRight};
  juce::Slider intraPhaseEnd{juce::Slider::LinearHorizontal,
                             juce::Slider::TextBoxRight};
  juce::Slider initialOffset{juce::Slider::LinearHorizontal,
                             juce::Slider::TextBoxRight};
  juce::Slider postOffset{juce::Slider::LinearHorizontal,
                          juce::Slider::TextBoxRight};
  juce::TextEditor inputEdit;
  juce::TextButton inputBrowse{"Browse"};
  juce::TextButton loadButton{"Load"}, saveButton{"Save"}, testButton{"Test"},
      generateButton{"Generate"};

  // Audio members
  juce::AudioDeviceManager deviceManager;
  juce::AudioTransportSource transport;
  std::unique_ptr<juce::AudioFormatReaderSource> readerSource;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoiseGeneratorDialog)
};

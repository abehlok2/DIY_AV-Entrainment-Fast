
#include "GlobalSettingsComponent.h"
#include "NoiseGeneratorDialog.h" // Include the full dialog implementation here
#include "FrequencyTesterDialog.h"

using namespace juce;

// This function needs to be defined. For now, it returns a new instance.
// You might have this defined elsewhere, but it's needed for the example to be
// complete.
std::unique_ptr<NoiseGeneratorDialog>
GlobalSettingsComponent::createNoiseGeneratorDialog() {
  return std::make_unique<NoiseGeneratorDialog>();
}

GlobalSettingsComponent::GlobalSettingsComponent(juce::AudioDeviceManager& dm)
    : deviceManager(dm) {
  addAndMakeVisible(&srLabel);
  srLabel.setText("Sample Rate:", dontSendNotification);
  addAndMakeVisible(&srEdit);
  srEdit.setText("44100");

  addAndMakeVisible(&cfLabel);
  cfLabel.setText("Crossfade (s):", dontSendNotification);
  addAndMakeVisible(&cfEdit);
  cfEdit.setText("1.0");

  addAndMakeVisible(&outFileLabel);
  outFileLabel.setText("Output File:", dontSendNotification);
  addAndMakeVisible(&outFileEdit);
  addAndMakeVisible(&browseOutButton);
  browseOutButton.setButtonText("...");
  browseOutButton.addListener(this);

  addAndMakeVisible(&noiseFileLabel);
  noiseFileLabel.setText("Noise Preset:", dontSendNotification);
  addAndMakeVisible(&noiseFileEdit);
  addAndMakeVisible(&browseNoiseButton);
  browseNoiseButton.setButtonText("...");
  browseNoiseButton.addListener(this);

  addAndMakeVisible(&noiseAmpLabel);
  noiseAmpLabel.setText("Noise Amp:", dontSendNotification);
  addAndMakeVisible(&noiseAmpEdit);
  noiseAmpEdit.setText("0.0");

  addAndMakeVisible(&noiseGenButton);
  noiseGenButton.setButtonText("Generate Noise Preset...");
  noiseGenButton.addListener(this);

  addAndMakeVisible(&freqTestButton);
  freqTestButton.setButtonText("Frequency Test...");
  freqTestButton.addListener(this);
}

GlobalSettingsComponent::~GlobalSettingsComponent() {
  browseOutButton.removeListener(this);
  browseNoiseButton.removeListener(this);
  noiseGenButton.removeListener(this);
  freqTestButton.removeListener(this);
}

GlobalSettingsComponent::Settings GlobalSettingsComponent::getSettings() const {
  Settings s;
  s.sampleRate = srEdit.getText().getDoubleValue();
  s.crossfadeSeconds = cfEdit.getText().getDoubleValue();
  s.outputFile = outFileEdit.getText();
  s.noiseFile = noiseFileEdit.getText();
  s.noiseAmp = noiseAmpEdit.getText().getDoubleValue();
  return s;
}

void GlobalSettingsComponent::setSettings(const Settings &s) {
  srEdit.setText(String(s.sampleRate));
  cfEdit.setText(String(s.crossfadeSeconds));
  outFileEdit.setText(s.outputFile);
  noiseFileEdit.setText(s.noiseFile);
  noiseAmpEdit.setText(String(s.noiseAmp));
}

void GlobalSettingsComponent::resized() {
  auto area = getLocalBounds().reduced(4);
  const int labelW = 100;
  const int buttonW = 30;
  const int rowH = 24;
  const int gap = 4;

  auto row1 = area.removeFromTop(rowH);
  srLabel.setBounds(row1.removeFromLeft(labelW));
  srEdit.setBounds(row1);

  area.removeFromTop(gap);

  auto row2 = area.removeFromTop(rowH);
  cfLabel.setBounds(row2.removeFromLeft(labelW));
  cfEdit.setBounds(row2);

  area.removeFromTop(gap);

  auto row3 = area.removeFromTop(rowH);
  outFileLabel.setBounds(row3.removeFromLeft(labelW));
  browseOutButton.setBounds(row3.removeFromRight(buttonW));
  outFileEdit.setBounds(row3);

  area.removeFromTop(gap);

  auto row4 = area.removeFromTop(rowH);
  noiseFileLabel.setBounds(row4.removeFromLeft(labelW));
  browseNoiseButton.setBounds(row4.removeFromRight(buttonW));
  noiseFileEdit.setBounds(row4);

  area.removeFromTop(gap);

  auto row5 = area.removeFromTop(rowH);
  noiseAmpLabel.setBounds(row5.removeFromLeft(labelW));
  noiseAmpEdit.setBounds(row5);

  noiseGenButton.setBounds(area.removeFromTop(rowH));
  freqTestButton.setBounds(area.removeFromTop(rowH));
}

void GlobalSettingsComponent::buttonClicked(Button *b) {
  if (b == &browseOutButton) {
    FileChooser chooser("Select Output File", File(outFileEdit.getText()),
                        "*.wav;*.flac;*.mp3");
    if (chooser.browseForFileToSave(true))
      outFileEdit.setText(chooser.getResult().getFullPathName());
  } else if (b == &browseNoiseButton) {
    FileChooser chooser("Select Noise Preset", File(noiseFileEdit.getText()),
                        "*.noise;*.wav");
    if (chooser.browseForFileToOpen())
      noiseFileEdit.setText(chooser.getResult().getFullPathName());
  } else if (b == &noiseGenButton) {
    auto dialog = createNoiseGeneratorDialog();
    DialogWindow::LaunchOptions opts;

    opts.content.setOwned(dialog.release());
    opts.dialogTitle = "Noise Generator";
    opts.dialogBackgroundColour = Colours::lightgrey;
    opts.escapeKeyTriggersCloseButton = true;
    opts.useNativeTitleBar = true;
    opts.resizable = true;
    opts.runModal();
  } else if (b == &freqTestButton) {
    auto dialog = createFrequencyTesterDialog(deviceManager);
    DialogWindow::LaunchOptions opts;
    opts.content.setOwned(dialog.release());
    opts.dialogTitle = "Frequency Test";
    opts.dialogBackgroundColour = Colours::lightgrey;
    opts.escapeKeyTriggersCloseButton = true;
    opts.useNativeTitleBar = true;
    opts.resizable = true;
    opts.runModal();
  }
}

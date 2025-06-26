#include "GlobalSettingsComponent.h"
#include "NoiseGeneratorDialog.h"

using namespace juce;

GlobalSettingsComponent::GlobalSettingsComponent() {
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
  browseOutButton.addListener(this);

  addAndMakeVisible(&noiseFileLabel);
  noiseFileLabel.setText("Noise Preset:", dontSendNotification);
  addAndMakeVisible(&noiseFileEdit);
  addAndMakeVisible(&browseNoiseButton);
  browseNoiseButton.addListener(this);

  addAndMakeVisible(&noiseAmpLabel);
  noiseAmpLabel.setText("Noise Amp:", dontSendNotification);
  addAndMakeVisible(&noiseAmpEdit);
  noiseAmpEdit.setText("0.0");

  addAndMakeVisible(&noiseGenButton);
  noiseGenButton.addListener(this);
}

GlobalSettingsComponent::~GlobalSettingsComponent() {
  browseOutButton.removeListener(this);
  browseNoiseButton.removeListener(this);
  noiseGenButton.removeListener(this);
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
  srEdit.setText(juce::String(s.sampleRate));
  cfEdit.setText(juce::String(s.crossfadeSeconds));
  outFileEdit.setText(s.outputFile);
  noiseFileEdit.setText(s.noiseFile);
  noiseAmpEdit.setText(juce::String(s.noiseAmp));
}

void GlobalSettingsComponent::resized() {
  auto area = getLocalBounds().reduced(4);
  const int labelW = 100;
  const int buttonW = 80;
  const int rowH = 24;

  auto row = area.removeFromTop(rowH);
  srLabel.setBounds(row.removeFromLeft(labelW));
  srEdit.setBounds(row);

  row = area.removeFromTop(rowH);
  cfLabel.setBounds(row.removeFromLeft(labelW));
  cfEdit.setBounds(row);

  row = area.removeFromTop(rowH);
  outFileLabel.setBounds(row.removeFromLeft(labelW));
  browseOutButton.setBounds(row.removeFromRight(buttonW));
  outFileEdit.setBounds(row);

  row = area.removeFromTop(rowH);
  noiseFileLabel.setBounds(row.removeFromLeft(labelW));
  browseNoiseButton.setBounds(row.removeFromRight(buttonW));
  noiseFileEdit.setBounds(row);

  row = area.removeFromTop(rowH);
  noiseAmpLabel.setBounds(row.removeFromLeft(labelW));
  noiseAmpEdit.setBounds(row);

  row = area.removeFromTop(rowH);
  noiseGenButton.setBounds(row);
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
  }
}

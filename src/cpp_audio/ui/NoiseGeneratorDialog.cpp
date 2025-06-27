#include "NoiseGeneratorDialog.h"
#include "../synths/SynthFunctions.h"
#include <nlohmann/json.hpp>

using namespace juce;

//==============================================================================
// NoiseParams helper functions (ideally in NoiseParams.cpp)
//==============================================================================

namespace {

static bool saveNoiseParams(const NoiseParams &p, const File &file) {
  using json = nlohmann::json;
  json j;
  j["duration_seconds"] = p.durationSeconds;
  j["sample_rate"] = p.sampleRate;
  j["noise_type"] = p.noiseType.toStdString();
  j["lfo_waveform"] = p.lfoWaveform.toStdString();
  j["transition"] = p.transition;
  j["lfo_freq"] = p.lfoFreq;
  j["start_lfo_freq"] = p.startLfoFreq;
  j["end_lfo_freq"] = p.endLfoFreq;
  j["start_lfo_phase_offset_deg"] = p.startLfoPhaseOffsetDeg;
  j["end_lfo_phase_offset_deg"] = p.endLfoPhaseOffsetDeg;
  j["start_intra_phase_offset_deg"] = p.startIntraPhaseOffsetDeg;
  j["end_intra_phase_offset_deg"] = p.endIntraPhaseOffsetDeg;
  j["initial_offset"] = p.initialOffset;
  j["post_offset"] = p.postOffset;
  j["input_audio_path"] = p.inputAudioPath.toStdString();
  json sw = json::array();
  for (const auto &s : p.sweeps) {
    json o;
    o["start_min"] = s.startMin;
    o["end_min"] = s.endMin;
    o["start_max"] = s.startMax;
    o["end_max"] = s.endMax;
    o["start_q"] = s.startQ;
    o["end_q"] = s.endQ;
    o["start_casc"] = s.startCasc;
    o["end_casc"] = s.endCasc;
    sw.push_back(o);
  }
  j["sweeps"] = sw;
  return file.replaceWithText(j.dump(2));
}

static bool loadNoiseParams(const File &file, NoiseParams &p) {
  using json = nlohmann::json;
  json j = json::parse(file.loadFileAsString().toStdString(), nullptr, false);
  if (j.is_discarded())
    return false;

  p.durationSeconds = j.value("duration_seconds", p.durationSeconds);
  p.sampleRate = j.value("sample_rate", p.sampleRate);
  p.noiseType = j.value("noise_type", p.noiseType.toStdString());
  p.lfoWaveform = j.value("lfo_waveform", p.lfoWaveform.toStdString());
  p.transition = j.value("transition", p.transition);
  p.lfoFreq = j.value("lfo_freq", p.lfoFreq);
  p.startLfoFreq = j.value("start_lfo_freq", p.startLfoFreq);
  p.endLfoFreq = j.value("end_lfo_freq", p.endLfoFreq);
  p.startLfoPhaseOffsetDeg =
      j.value("start_lfo_phase_offset_deg", p.startLfoPhaseOffsetDeg);
  p.endLfoPhaseOffsetDeg =
      j.value("end_lfo_phase_offset_deg", p.endLfoPhaseOffsetDeg);
  p.startIntraPhaseOffsetDeg =
      j.value("start_intra_phase_offset_deg", p.startIntraPhaseOffsetDeg);
  p.endIntraPhaseOffsetDeg =
      j.value("end_intra_phase_offset_deg", p.endIntraPhaseOffsetDeg);
  p.initialOffset = j.value("initial_offset", p.initialOffset);
  p.postOffset = j.value("post_offset", p.postOffset);
  p.inputAudioPath =
      j.value("input_audio_path", p.inputAudioPath.toStdString());
  p.sweeps.clear();

  if (j.contains("sweeps")) {
    for (const auto &e : j["sweeps"]) {
      NoiseParams::Sweep s;
      s.startMin = e.value("start_min", s.startMin);
      s.endMin = e.value("end_min", s.endMin);
      s.startMax = e.value("start_max", s.startMax);
      s.endMax = e.value("end_max", s.endMax);
      s.startQ = e.value("start_q", s.startQ);
      s.endQ = e.value("end_q", s.endQ);
      s.startCasc = e.value("start_casc", s.startCasc);
      s.endCasc = e.value("end_casc", s.endCasc);
      p.sweeps.add(s);
    }
  }
  return true;
}

static AudioBuffer<float> generateNoiseBuffer(const NoiseParams &p) {
  NamedValueSet params;
  params.set("noise_type", p.noiseType);
  params.set("lfo_waveform", p.lfoWaveform);
  params.set("input_audio_path", p.inputAudioPath);
  params.set("initial_offset", p.initialOffset);
  params.set("post_offset", p.postOffset);
  params.set("start_lfo_phase_offset_deg", p.startLfoPhaseOffsetDeg);
  params.set("end_lfo_phase_offset_deg", p.endLfoPhaseOffsetDeg);
  params.set("start_intra_phase_offset_deg", p.startIntraPhaseOffsetDeg);
  params.set("end_intra_phase_offset_deg", p.endIntraPhaseOffsetDeg);

  Array<var> startSweeps, endSweeps, sweeps, startQ, endQ, startCasc, endCasc;

  for (const auto &s : p.sweeps) {
    // --- FIX START ---
    // Create DynamicObjects for the start, end, and continuous sweeps
    auto *startSweepObj = new DynamicObject();
    startSweepObj->setProperty("start_min", s.startMin);
    startSweepObj->setProperty("start_max", s.startMax);
    startSweeps.add(var(startSweepObj));

    auto *endSweepObj = new DynamicObject();
    endSweepObj->setProperty("end_min", s.endMin);
    endSweepObj->setProperty("end_max", s.endMax);
    endSweeps.add(var(endSweepObj));

    auto *sweepObj = new DynamicObject();
    sweepObj->setProperty("start_min", s.startMin);
    sweepObj->setProperty("start_max", s.startMax);
    sweeps.add(var(sweepObj));
    // --- FIX END ---

    startQ.add(s.startQ);
    endQ.add(s.endQ);
    startCasc.add(s.startCasc);
    endCasc.add(s.endCasc);
  }

  if (p.transition) {
    params.set("start_lfo_freq", p.startLfoFreq);
    params.set("end_lfo_freq", p.endLfoFreq);
    params.set("start_filter_sweeps", startSweeps);
    params.set("end_filter_sweeps", endSweeps);
    params.set("start_notch_q", startQ.size() > 1 ? var(startQ) : startQ[0]);
    params.set("end_notch_q", endQ.size() > 1 ? var(endQ) : endQ[0]);
    params.set("start_cascade_count",
               startCasc.size() > 1 ? var(startCasc) : startCasc[0]);
    params.set("end_cascade_count",
               endCasc.size() > 1 ? var(endCasc) : endCasc[0]);
    return generateSweptNotchPinkSoundTransition(p.durationSeconds,
                                                 p.sampleRate, params);
  } else {
    params.set("lfo_freq", p.lfoFreq);
    params.set("filter_sweeps", sweeps);
    params.set("notch_q", startQ.size() > 1 ? var(startQ) : startQ[0]);
    params.set("cascade_count",
               startCasc.size() > 1 ? var(startCasc) : startCasc[0]);
    params.set("lfo_phase_offset_deg", p.startLfoPhaseOffsetDeg);
    params.set("intra_phase_offset_deg", p.startIntraPhaseOffsetDeg);
    return generateSweptNotchPinkSound(p.durationSeconds, p.sampleRate, params);
  }
}

} // namespace

//==============================================================================
// NoiseGeneratorDialog Implementation
//==============================================================================
NoiseGeneratorDialog::NoiseGeneratorDialog() {
  fileEdit.setText("swept_notch_noise.wav");
  addAndMakeVisible(&fileEdit);
  addAndMakeVisible(&fileBrowse);
  fileBrowse.addListener(this);

  durationSlider.setRange(1.0, 100000.0, 0.1);
  durationSlider.setValue(60.0);
  addAndMakeVisible(&durationSlider);

  sampleRateSlider.setRange(8000, 192000, 1);
  sampleRateSlider.setValue(44100);
  addAndMakeVisible(&sampleRateSlider);

  noiseType.addItem("Pink", 1);
  noiseType.addItem("Brown", 2);
  noiseType.setSelectedId(1);
  addAndMakeVisible(&noiseType);

  transitionToggle.setButtonText("Enable Transition");
  addAndMakeVisible(&transitionToggle);

  lfoWaveform.addItem("Sine", 1);
  lfoWaveform.addItem("Triangle", 2);
  lfoWaveform.setSelectedId(1);
  addAndMakeVisible(&lfoWaveform);

  lfoStart.setRange(0.001, 10.0, 0.0001);
  lfoStart.setValue(1.0 / 12.0);
  lfoStart.addListener(this);
  addAndMakeVisible(&lfoStart);

  lfoEnd.setRange(0.001, 10.0, 0.0001);
  lfoEnd.setValue(1.0 / 12.0);
  addAndMakeVisible(&lfoEnd);

  numSweeps.setRange(1, 3, 1);
  numSweeps.setValue(1);
  numSweeps.addListener(this);
  addAndMakeVisible(&numSweeps);

  for (int i = 0; i < 3; ++i) {
    auto *sc = new SweepControls();
    sc->startMin.setRange(20, 20000, 1);
    sc->startMax.setRange(20, 22050, 1);
    sc->endMin.setRange(20, 20000, 1);
    sc->endMax.setRange(20, 22050, 1);
    sc->startMin.setValue(i == 0 ? 1000 : (i == 1 ? 500 : 1850));
    sc->startMax.setValue(i == 0 ? 10000 : (i == 1 ? 1000 : 3350));
    sc->endMin.setValue(sc->startMin.getValue());
    sc->endMax.setValue(sc->startMax.getValue());
    sc->startQ.setRange(1, 1000, 1);
    sc->startQ.setValue(25);
    sc->endQ.setRange(1, 1000, 1);
    sc->endQ.setValue(25);
    sc->startCasc.setRange(1, 20, 1);
    sc->startCasc.setValue(10);
    sc->endCasc.setRange(1, 20, 1);
    sc->endCasc.setValue(10);

    addAndMakeVisible(sc->startMin);
    addAndMakeVisible(sc->endMin);
    addAndMakeVisible(sc->startMax);
    addAndMakeVisible(sc->endMax);
    addAndMakeVisible(sc->startQ);
    addAndMakeVisible(sc->endQ);
    addAndMakeVisible(sc->startCasc);
    addAndMakeVisible(sc->endCasc);

    sweepControls.add(sc);
  }
  updateSweepVisibility(1);

  lfoPhaseStart.setRange(0, 360, 1);
  lfoPhaseStart.setValue(0);
  lfoPhaseEnd.setRange(0, 360, 1);
  lfoPhaseEnd.setValue(0);
  addAndMakeVisible(&lfoPhaseStart);
  addAndMakeVisible(&lfoPhaseEnd);

  intraPhaseStart.setRange(0, 360, 1);
  intraPhaseStart.setValue(0);
  intraPhaseEnd.setRange(0, 360, 1);
  intraPhaseEnd.setValue(0);
  addAndMakeVisible(&intraPhaseStart);
  addAndMakeVisible(&intraPhaseEnd);

  initialOffset.setRange(0.0, 10000.0, 0.001);
  initialOffset.setValue(0.0);
  postOffset.setRange(0.0, 10000.0, 0.001);
  postOffset.setValue(0.0);
  addAndMakeVisible(&initialOffset);
  addAndMakeVisible(&postOffset);

  addAndMakeVisible(&inputEdit);
  addAndMakeVisible(&inputBrowse);
  inputBrowse.addListener(this);

  addAndMakeVisible(&loadButton);
  addAndMakeVisible(&saveButton);
  addAndMakeVisible(&testButton);
  addAndMakeVisible(&generateButton);
  loadButton.addListener(this);
  saveButton.addListener(this);
  testButton.addListener(this);
  generateButton.addListener(this);

  setSize(600, 700);
  deviceManager.initialise(0, 2, nullptr, true);
}

NoiseGeneratorDialog::~NoiseGeneratorDialog() {
  transport.stop();
  deviceManager.closeAudioDevice();
}

void NoiseGeneratorDialog::resized() {
  int x = 10, y = 10, w = getWidth() - 20, h = 24;
  fileEdit.setBounds(x, y, w - 80, h);
  fileBrowse.setBounds(x + w - 70, y, 60, h);
  y += h + 8;
  durationSlider.setBounds(x, y, w, h);
  y += h + 8;
  sampleRateSlider.setBounds(x, y, w, h);
  y += h + 8;
  noiseType.setBounds(x, y, w, h);
  y += h + 8;
  transitionToggle.setBounds(x, y, w, h);
  y += h + 8;
  lfoWaveform.setBounds(x, y, w, h);
  y += h + 8;
  lfoStart.setBounds(x, y, w / 2 - 5, h);
  lfoEnd.setBounds(x + w / 2 + 5, y, w / 2 - 5, h);
  y += h + 8;
  numSweeps.setBounds(x, y, w, h);
  y += h + 8;
  for (int i = 0; i < sweepControls.size(); ++i) {
    auto *sc = sweepControls.getUnchecked(i);
    if (!sc->startMin.isVisible())
      continue;
    sc->startMin.setBounds(x, y, w / 4 - 2, h);
    sc->endMin.setBounds(x + w / 4 + 2, y, w / 4 - 2, h);
    sc->startMax.setBounds(x + w / 2 + 4, y, w / 4 - 2, h);
    sc->endMax.setBounds(x + w * 3 / 4 + 6, y, w / 4 - 2, h);
    y += h + 2;
    sc->startQ.setBounds(x, y, w / 4 - 2, h);
    sc->endQ.setBounds(x + w / 4 + 2, y, w / 4 - 2, h);
    sc->startCasc.setBounds(x + w / 2 + 4, y, w / 4 - 2, h);
    sc->endCasc.setBounds(x + w * 3 / 4 + 6, y, w / 4 - 2, h);
    y += h + 8;
  }
  lfoPhaseStart.setBounds(x, y, w / 2 - 5, h);
  lfoPhaseEnd.setBounds(x + w / 2 + 5, y, w / 2 - 5, h);
  y += h + 8;
  intraPhaseStart.setBounds(x, y, w / 2 - 5, h);
  intraPhaseEnd.setBounds(x + w / 2 + 5, y, w / 2 - 5, h);
  y += h + 8;
  initialOffset.setBounds(x, y, w / 2 - 5, h);
  postOffset.setBounds(x + w / 2 + 5, y, w / 2 - 5, h);
  y += h + 8;
  inputEdit.setBounds(x, y, w - 80, h);
  inputBrowse.setBounds(x + w - 70, y, 60, h);
  y += h + 8;
  loadButton.setBounds(x, y, 60, h);
  saveButton.setBounds(x + 70, y, 60, h);
  testButton.setBounds(x + w - 130, y, 60, h);
  generateButton.setBounds(x + w - 60, y, 60, h);
}

NoiseParams NoiseGeneratorDialog::getParams() const {
  NoiseParams p;
  p.durationSeconds = durationSlider.getValue();
  p.sampleRate = (int)sampleRateSlider.getValue();
  p.noiseType = noiseType.getText().toLowerCase();
  p.lfoWaveform = lfoWaveform.getText().toLowerCase();
  p.transition = transitionToggle.getToggleState();
  p.lfoFreq = lfoStart.getValue();
  p.startLfoFreq = lfoStart.getValue();
  p.endLfoFreq = lfoEnd.getValue();
  p.startLfoPhaseOffsetDeg = (int)lfoPhaseStart.getValue();
  p.endLfoPhaseOffsetDeg = (int)lfoPhaseEnd.getValue();
  p.startIntraPhaseOffsetDeg = (int)intraPhaseStart.getValue();
  p.endIntraPhaseOffsetDeg = (int)intraPhaseEnd.getValue();
  p.initialOffset = initialOffset.getValue();
  p.postOffset = postOffset.getValue();
  p.inputAudioPath = inputEdit.getText();
  p.sweeps.clear();
  int n = (int)numSweeps.getValue();
  for (int i = 0; i < n && i < sweepControls.size(); ++i) {
    const auto *sc = sweepControls.getUnchecked(i);
    NoiseParams::Sweep s;
    s.startMin = (int)sc->startMin.getValue();
    s.endMin = (int)sc->endMin.getValue();
    s.startMax = (int)sc->startMax.getValue();
    s.endMax = (int)sc->endMax.getValue();
    s.startQ = (int)sc->startQ.getValue();
    s.endQ = (int)sc->endQ.getValue();
    s.startCasc = (int)sc->startCasc.getValue();
    s.endCasc = (int)sc->endCasc.getValue();
    p.sweeps.add(s);
  }
  return p;
}

void NoiseGeneratorDialog::setParams(const NoiseParams &p) {
  durationSlider.setValue(p.durationSeconds);
  sampleRateSlider.setValue(p.sampleRate);
  noiseType.setSelectedId(p.noiseType.startsWithIgnoreCase("brown") ? 2 : 1);
  lfoWaveform.setSelectedId(p.lfoWaveform.startsWithIgnoreCase("triangle") ? 2
                                                                           : 1);
  transitionToggle.setToggleState(p.transition, dontSendNotification);
  lfoStart.setValue(p.transition ? p.startLfoFreq : p.lfoFreq);
  lfoEnd.setValue(p.endLfoFreq);
  numSweeps.setValue(jmax(1, p.sweeps.size()));
  for (int i = 0; i < sweepControls.size(); ++i) {
    if (i < p.sweeps.size()) {
      const auto &s = p.sweeps.getReference(i);
      auto *sc = sweepControls.getUnchecked(i);
      sc->startMin.setValue(s.startMin);
      sc->endMin.setValue(s.endMin);
      sc->startMax.setValue(s.startMax);
      sc->endMax.setValue(s.endMax);
      sc->startQ.setValue(s.startQ);
      sc->endQ.setValue(s.endQ);
      sc->startCasc.setValue(s.startCasc);
      sc->endCasc.setValue(s.endCasc);
    }
  }
  lfoPhaseStart.setValue(p.startLfoPhaseOffsetDeg);
  lfoPhaseEnd.setValue(p.endLfoPhaseOffsetDeg);
  intraPhaseStart.setValue(p.startIntraPhaseOffsetDeg);
  intraPhaseEnd.setValue(p.endIntraPhaseOffsetDeg);
  initialOffset.setValue(p.initialOffset);
  postOffset.setValue(p.postOffset);
  inputEdit.setText(p.inputAudioPath);
  updateSweepVisibility((int)numSweeps.getValue());
}

void NoiseGeneratorDialog::updateSweepVisibility(int count) {
  for (int i = 0; i < sweepControls.size(); ++i) {
    auto *sc = sweepControls.getUnchecked(i);
    bool vis = i < count;
    sc->startMin.setVisible(vis);
    sc->endMin.setVisible(vis);
    sc->startMax.setVisible(vis);
    sc->endMax.setVisible(vis);
    sc->startQ.setVisible(vis);
    sc->endQ.setVisible(vis);
    sc->startCasc.setVisible(vis);
    sc->endCasc.setVisible(vis);
  }
  resized(); //
}

void NoiseGeneratorDialog::buttonClicked(Button *b) {
  if (b == &fileBrowse) {
    FileChooser fc("Save Audio", {}, "*.wav");
    if (fc.browseForFileToSave(true))
      fileEdit.setText(fc.getResult().getFullPathName());
  } else if (b == &inputBrowse) {
    FileChooser fc("Load Audio", {}, "*.wav;*.flac;*.mp3");
    if (fc.browseForFileToOpen())
      inputEdit.setText(fc.getResult().getFullPathName());
  } else if (b == &loadButton) {
    FileChooser fc("Load Noise Settings", {}, String("*") + ".noise");
    if (fc.browseForFileToOpen()) {
      NoiseParams p;
      if (loadNoiseParams(fc.getResult(), p))
        setParams(p);
    }
  } else if (b == &saveButton) {
    FileChooser fc("Save Noise Settings", {}, String("*") + ".noise");
    if (fc.browseForFileToSave(true))
      saveNoiseParams(getParams(), fc.getResult());
  } else if (b == &generateButton) {
    auto p = getParams();
    auto buffer = generateNoiseBuffer(p);
    File out(fileEdit.getText());
    WavAudioFormat format;
    std::unique_ptr<AudioFormatWriter> writer(format.createWriterFor(
        out.createOutputStream().release(), p.sampleRate, 2, 16, {}, 0));
    if (writer != nullptr)
      writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
  } else if (b == &testButton) {
    NoiseParams p = getParams();
    p.durationSeconds = 30.0;
    auto buffer = generateNoiseBuffer(p);
    MemoryOutputStream mem;
    WavAudioFormat wav;
    std::unique_ptr<AudioFormatWriter> writer(
        wav.createWriterFor(&mem, p.sampleRate, 2, 16, {}, 0));
    if (writer) {
      writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
    }
    auto stream = std::make_unique<MemoryInputStream>(mem.getData(),
                                                      mem.getDataSize(), false);
    AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    std::unique_ptr<AudioFormatReader> reader(
        formatManager.createReaderFor(std::move(stream)));
    if (reader != nullptr) {
      readerSource.reset(new AudioFormatReaderSource(reader.release(), true));
      transport.setSource(readerSource.get(), 0, nullptr, p.sampleRate);
      transport.start();
    }
  }
}

void NoiseGeneratorDialog::sliderValueChanged(Slider *s) {
  if (s == &numSweeps)
    updateSweepVisibility((int)numSweeps.getValue());
}

//==============================================================================
// Factory function
//==============================================================================
std::unique_ptr<Component> createNoiseGeneratorDialog() {
  return std::make_unique<NoiseGeneratorDialog>();
}
